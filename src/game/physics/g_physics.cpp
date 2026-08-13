// ============================================================================
// g_physics.cpp - physics.o game integration (phys_xboxr physics.o family)
// Ported smallest-first from IDA (release map offset + 0x40C000 = VA).
// ============================================================================

#include "physics/physics_system.h"
#include "physics/rb_ragdoll_model.h"
#include "core/tlFixedString.h"
#include <float.h>
#include <intrin.h>
#include <math.h>
#include <new>
#include <stdio.h>
#include <string.h>

// effect_events / core_systems views
class Entity;
class Handle;
namespace rb_prop_system {
void remove_entity(Entity* e);  // ?remove_entity@rb_prop_system@@YAXPAVEntity@@@Z
}
typedef int EEffectContext;
enum ECollisionMaterial : int {
    kCollisionMaterialMin = 0,
    kCollisionMaterialASPHALT = 1,
    kCollisionMaterialNONE = 2,
    kCollisionMaterialFLESH = 4,
};
struct SimpleCollisionDesc {
    math::Position3 coord;   // +0x00
    math::Position3 normal;  // +0x10
};
struct CollisionDesc {
    SimpleCollisionDesc simple;  // +0x00
    ECollisionMaterial material; // +0x20
};
Handle PostEffectEventPhysicsImpact(const Entity* ent, int myColMat,
                                    const CollisionDesc* col_desc,
                                    float intensity);  // effect_events.cpp
Entity* SpawnHelmet(Entity* self, const float* hitP, const float* hitDir,
                    float iDamage);  // g.o

// effect-events / vehicle externs (game.o / core.o)
// proximity_data_t (game.o; minimal view - lo +0x00, hi +0x10)
struct proximity_data_t {
    math::Position3 lo;  // +0x00
    math::Position3 hi;  // +0x10
};
extern void filter_proximity_data(const math::Position3& lo,
                                  const math::Position3& hi, int contents,
                                  const proximity_data_t& in,
                                  proximity_data_t& out);  // game.o
extern void j_nullsub_50(void* self);  // g.o
extern void EffectEventAdjustEffect_Scale(Handle effect, const char* param,
                                          float scale);  // core.o 0x4CB8E0
extern Handle PostEffectEventVehicle(const Entity* ent,
                                     const char* vehicleType,
                                     int action);  // core.o
extern Handle PostEffectEventVehicleWheel(const Entity* ent,
                                          const char* vehicleType,
                                          int action, int mat_type,
                                          unsigned int wheel_tag_hash);  // core.o
extern void UpdateWheelMarks(Entity* owner, int wheel_id, bool wheel_state,
                             const math::Position3& hitp,
                             const math::Dir3& hitn);  // physics.o
extern unsigned int s_wheelTagHashes[6];  // g.o @ 0xEE62CC

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
bool Warning(const char* fmt, ...);
bool Error(const char* fmt, ...);
}

namespace nuge {
void calc_velocities(const math::Mat43& mat0, const math::Mat43& mat1,
                     const math::Dir3& center_offset_loc, float delta_t,
                     math::Dir3* t_vel, math::Dir3* a_vel);
void calc_sphere_inertia(float radius, math::Dir3* unit_inertia,
                         float* volume);  // ?calc_sphere_inertia@nuge@@SAXMPAVDir3@math@@PAM@Z
void calc_box_inertia(const math::Dir3* dim, math::Dir3* unit_inertia,
                      float* volume);  // ?calc_box_inertia@nuge@@SAXPBVDir3@math@@PAV23@PAM@Z
}
void AnglesToAxis(const math::Position3* angles, float (*axis)[3]);
void AnglesToAxis(const math::Position3& angles, const math::Position3& origin,
                  math::Mat43& mat);  // ?AnglesToAxis@@YAXABVPosition3@math@@0AAVMat43@2@@Z
void AnglesToAxis(const float* angles, float (*axis)[3]);  // ?AnglesToAxis@@YAXPBMPAY02M@Z
void SetIdentity(math::Mat43& m);  // inline COMDAT (pulse_sum.h; ea: 0x6E4D00)

class PakManager {
public:
    static PakManager* sInst;
    void* CrazyTempMemBorrow(unsigned int align, unsigned int size);
    void  CrazyTempMemGiveBack(void* ptr);
};

void merge_spheres(const math::Position3& c1, float r1, math::Position3* c2,
                   float* r2);
// Color (core/color.h view; RGBA float, 16 bytes)
class Color {
public:
    float r;  // +0x00
    float g;  // +0x04
    float b;  // +0x08
    float a;  // +0x0C

    Color() : r(0.0f), g(0.0f), b(0.0f), a(1.0f) {}
    Color(float _r, float _g, float _b, float _a)
        : r(_r), g(_g), b(_b), a(_a) {}
};
class DebugRender {
public:
    static DebugRender sInst;
    static void RenderAxis(const math::Mat43& mat, float length, float width);
    static void RenderSphere(const math::Position3& pos, float radius,
                             const Color& col);  // render.o 0xAC63A0
    static void RenderLineBox(const math::Mat43& LToW,
                              const math::DiagMat33& size,
                              const Color& col);  // render.o 0xAC6700
    static void RenderText(const char* str, int x, int y, const Color& col,
                           float depth, float size);  // render.o 0xAAC7D0
    static void RenderLine(const math::Position3& pt1,
                           const math::Position3& pt2, const Color& col,
                           float thickness);  // render.o 0xAC7AB0
    static void RenderCone(const math::Position3& pos, const math::Dir3& dir,
                           float angle, float length,
                           const Color& col);  // render.o 0xAC51C0
    void AddRenderer(void (*fp)());  // ?AddRenderer@DebugRender@@QAEXP6AXXZ@Z
};
// ?sInst@DebugRender@@2V1@A (physics.o data @ 0xF74D20)
DebugRender DebugRender::sInst;
void DebugRender::AddRenderer(void (*fp)())
{
    (void)fp;
}
static void DebugRender_RenderLine(const math::Position3& pt1,
                                   const math::Position3& pt2,
                                   const float* color, float thickness)
{
    (void)pt1; (void)pt2; (void)color; (void)thickness;
}

class Entity;
struct rb_extra_info;
class rb_vehicle;
class DObj;
class PakFile;
class rigid_body;
rigid_body_constraint_custom_path* path_constraint_create(
    Entity* veh);  // ?path_constraint_create@@YAPAVrigid_body_constraint_custom_path@@PAVEntity@@@Z
extern bool g_in_physics_collision_callback;
void VEH_Backup(Entity* ent);  // ?VEH_Backup@@YAXPAVEntity@@@Z (g.o)
void VEH_UpdatePath(Entity* ent, int msec);  // ?VEH_UpdatePath@@YAXPAVEntity@@H@Z (g.o)
class biped_phys_info;
struct phys_gjk_geom_list;
class phys_gjk_geom_cod_base;
struct trajectory_t;
class PhysData {
public:
    uint8_t mName[4];   // +0x00 (InplaceString)
    float   mMass;      // +0x04
    float   mBounce;    // +0x08
    float   mFric;      // +0x0C
    void*   mConstraints;  // +0x10 (InplaceVector<PhysConstraint>)
};

// InplaceVector<T> (ae/inplace/InplaceVector.h view; 8 bytes). Layout
// verified from AddPiece disasm (mSize read from +0x00, elements from +0x04).
template <typename T>
struct InplaceVector {
    unsigned int mSize;  // +0x00
    T*           mList;  // +0x04
};

// PhysConstraint (physics.o RBPhysData.h view; 40 bytes, IDA ordinal)
struct PhysConstraint {
    float mMinAngle;   // +0x00
    float mMaxAngle;   // +0x04
    float mDist;       // +0x08
    float mDamp;       // +0x0C
    float mOrigin[3];  // +0x10
    float mAngles[3];  // +0x1C
};

// IVPointer<T> (game_types.h view; class tag V matches the binary manglings)
template <typename T>
class IVPointer {
public:
    T*           mValue;  // +0x00
    unsigned int mPakId;  // +0x04
};
class DCGSet;
void phys_collision_allocater_ballistic_reinit();  // 0x6F7060
enum hitLocation_t;
enum EHitLocation;                       // Broc's EHitLocation (g_local.h)
int GetPhysBoneID(hitLocation_t id);
void ApplyPhysics(Entity* hitEnt, const math::Position3& hitp,
                  const math::Dir3& hitd, float force, bool local_hitp,
                  hitLocation_t hitLoc);
// G_Damage (game.o g_combat.cpp; declared for the wheel-collide damage call)
void G_Damage(Entity* targ, Entity* inflictor, Entity* attacker,
              const float* dir, const float* point, int damage, int dflags,
              int mod, EHitLocation hitLoc, int weapon);
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
void G_SetOrigin(Entity* ent, const math::Position3* origin);  // g.o
void G_SetAngle(Entity* ent, const math::Position3* angle);    // g.o
namespace BrocSys {
void Mover_RotateSpeed(Entity* pEnt, const math::Position3& vRotSpeed,
                       float fTotalTime, float fAccelTime,
                       float fDecelTime);  // ?Mover_RotateSpeed@BrocSys@@YAXPAVEntity@@ABVPosition3@math@@MMM@Z
}
// stub until scr.o Mover_RotateSpeed is ported
void BrocSys::Mover_RotateSpeed(Entity* pEnt, const math::Position3& vRotSpeed,
                                float fTotalTime, float fAccelTime,
                                float fDecelTime)
{
    (void)pEnt; (void)vRotSpeed; (void)fTotalTime; (void)fAccelTime;
    (void)fDecelTime;
}
void phys_full_inv_multiply_mat(math::Mat43& dest_m,
                                const math::Mat43& left_m,
                                const math::Mat43& right_m);  // physics.o inline
static const __m128 Float4_SignMask_12 = { -0.0f, -0.0f, -0.0f, -0.0f };
static const __m128 Float4_Zero_12 = { 0.0f, 0.0f, 0.0f, 0.0f };  // 0xD142E0
// ea: 0x7187C0 - dest = inverse(left) * right (left is orthonormal: transpose)
void phys_full_inv_multiply_mat(math::Mat43& dest_m,
                                const math::Mat43& left_m,
                                const math::Mat43& right_m)
{
    __m128 v3 = left_m.z.v;
    __m128 v4 = left_m.y.v;
    __m128 v5 = _mm_shuffle_ps(left_m.x.v, v4, 68);
    __m128 v6 =
        _mm_shuffle_ps(_mm_shuffle_ps(left_m.x.v, v4, 238), v3, 168);
    __m128 v7 = right_m.y.v;
    dest_m.x.v = _mm_add_ps(
        _mm_add_ps(
            _mm_mul_ps(_mm_shuffle_ps(right_m.x.v, right_m.x.v, 0),
                       _mm_shuffle_ps(v5, v3, 136)),
            _mm_mul_ps(_mm_shuffle_ps(right_m.x.v, right_m.x.v, 85),
                       _mm_shuffle_ps(v5, v3, 221))),
        _mm_mul_ps(_mm_shuffle_ps(right_m.x.v, right_m.x.v, 170), v6));
    dest_m.y.v = _mm_add_ps(
        _mm_add_ps(
            _mm_mul_ps(_mm_shuffle_ps(v7, v7, 0), _mm_shuffle_ps(v5, v3, 136)),
            _mm_mul_ps(_mm_shuffle_ps(v7, v7, 85),
                       _mm_shuffle_ps(v5, v3, 221))),
        _mm_mul_ps(_mm_shuffle_ps(v7, v7, 170), v6));
    __m128 v11 = right_m.z.v;
    dest_m.z.v = _mm_add_ps(
        _mm_add_ps(
            _mm_mul_ps(_mm_shuffle_ps(v11, v11, 0),
                       _mm_shuffle_ps(v5, v3, 136)),
            _mm_mul_ps(_mm_shuffle_ps(v11, v11, 85),
                       _mm_shuffle_ps(v5, v3, 221))),
        _mm_mul_ps(_mm_shuffle_ps(v11, v11, 170), v6));
    __m128 v15 = _mm_shuffle_ps(v5, v3, 136);
    __m128 v16 = _mm_shuffle_ps(v5, v3, 221);
    __m128 v17 = _mm_xor_ps(
        Float4_SignMask_12,
        _mm_add_ps(
            _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(left_m.w.v, left_m.w.v, 0),
                                  v15),
                       _mm_mul_ps(_mm_shuffle_ps(left_m.w.v, left_m.w.v, 85),
                                  v16)),
            _mm_mul_ps(_mm_shuffle_ps(left_m.w.v, left_m.w.v, 170), v6)));
    __m128 v18 = right_m.w.v;
    dest_m.w.v = _mm_add_ps(
        _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(v18, v18, 0), v15),
                   _mm_mul_ps(_mm_shuffle_ps(v18, v18, 85), v16)),
        _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(v18, v18, 170), v6), v17));
}
TPakId CurPakId();  // ?CurPakId@@YA?AW4TPakId@@XZ (streamer.o)
void ValidatePakId(TPakId pakId);  // streamer.o ?ValidatePakId@@YAXW4TPakId@@@Z
// cdl_object_t (g_local.h view; local copy - cflags + aabb)
struct cdl_object_t {
    int   cflags;        // +0x00
    int   sflags;        // +0x04
    float center[3];     // +0x08
    float box_radius[3]; // +0x14
    float sphere_radius; // +0x20
};
struct vehicle_pathpos_t;
void G_VehInitPathPos(vehicle_pathpos_t* vpp);  // g.o 0x4526D0
bool FindInitialVel(Entity* e, math::Dir3& initial_t_vel,
                    math::Dir3& initial_a_vel);  // physics.o
void SetupConstraints(rb_extra_info* rb_inf);  // physics.o
extern float PHYSICS_GRAVITY_SCALE_1;  // physics.o @ 0xE01F08
extern const math::Dir3& PHYSICS_GRAVITY_DIRECTION_3;  // physics.o @ 0xE01F00
void BG_EvaluateTrajectory(const trajectory_t* tr, int atTime,
                           math::Position3& result);  // g.o
struct level_locals_t {
    uint8_t _pad0[0x9C];
    int     time;  // +0x9C
};
extern level_locals_t level;  // ?level@@3Ulevel_locals_t@@A
extern int gPhysicsFinder;  // ?gPhysicsFinder@@3HA (g.o)
// ?low_end_speed@@3MA / ?percent_to_give@@3MA (physics.o data @ 0xE36B2C/0xE36B28)
float low_end_speed = 300.0f;    // 0x43960000
float percent_to_give = 1.5f;    // 0x3FC00000
// ?min_speed2@@3MA (physics.o data @ 0xE36B20; 5000.0)
float min_speed2 = 5000.0f;      // 0x45992000
struct vmCvar_t {
    int integer;  // +0x00 (minimal)
};
extern vmCvar_t g_speed;  // ?g_speed@@3UvmCvar_t@@A (g.o)
// MultiplayerMgr (core.o view; ApplyLocalPhysicsToVehicle only)
class MultiplayerMgr {
public:
    static MultiplayerMgr* sInst;  // ?sInst@MultiplayerMgr@@2PAV1@A
    void ApplyLocalPhysicsToVehicle(Entity* vehicle, math::Mat43* mat,
                                    math::Dir3* velocity);
    void VehicleRequestOwnership(Entity* vehicle,
                                 Entity* newOwner);  // core.o (stub)
};
// stub until MultiplayerMgr internals are ported (core.o)
void MultiplayerMgr::ApplyLocalPhysicsToVehicle(Entity* vehicle,
                                                math::Mat43* mat,
                                                math::Dir3* velocity)
{
    (void)vehicle; (void)mat; (void)velocity;
}
void MultiplayerMgr::VehicleRequestOwnership(Entity* vehicle,
                                             Entity* newOwner)
{
    (void)vehicle; (void)newOwner;
}
MultiplayerMgr* MultiplayerMgr::sInst = nullptr;
class EntityManager {
public:
    static EntityManager* sInst;  // ?sInst@EntityManager@@2PAV1@A (game.o)
    Entity* GetPlayer(int idx);   // ?GetPlayer@EntityManager@@QAEPAVEntity@@H@Z (g.o inline)
    bool IsLocalPlayer(Entity* entity);   // ?IsLocalPlayer@EntityManager@@QAE_NPAVEntity@@@Z (game.o)
    int  GetPlayerIndex(Entity* entity);  // ?GetPlayerIndex@EntityManager@@QAEHPAVEntity@@@Z (game.o)
    Entity* mWorld;               // +0x44
};
struct sentient_s {
    uint8_t _pad[0x38];
    int32_t bIgnoreMe;  // +0x38
};
struct actor_s {
    uint8_t _pad[0x310];
    int     bIsAlive;  // +0x310 (Physics.bIsAlive)
};
void G_EntUnlink(Entity* ent);  // g.o
struct tagInfoLocal {  // tagInfo_t subset (parent +0x00)
    void* parent;
};
enum EPropPriority {
    PROP_PRIORITY_LOW = 0,
    PROP_PRIORITY_MEDIUM = 1,
    PROP_PRIORITY_HIGH = 2,
};
void DObjGetBasePose(DObj* obj);  // ?DObjGetBasePose@@YAXPAVDObj@@@Z (render.o)
void path_constraint_destroy(class rigid_body_constraint_custom_path* vpc);
void path_constraint_update(rigid_body_constraint_custom_path* vpc,
                            Entity* veh);  // 0x6F5B60

// Handle (game_types.h view; local copy)
class Handle {
public:
    unsigned int mVal;  // +0x00
};

// Broc::vector (broc_types.h view; local copy - 12 bytes)
namespace Broc {
struct vector {
    float x;  // +0x00
    float y;  // +0x04
    float z;  // +0x08
};
class string {
public:
    void* mBlock;  // +0x00 (Block*; chars follow the header)
    string& operator=(const char* txt);  // ??4string@Broc@@QAEAAV01@PBD@Z (Broc.o)
};
}

// ragdoll_collision_callback (physics.o RBRagdollCollision.cpp). Raw-offset
// pools: m_rb_colgeom (m_alloc_list +0xFB0, count +0x1004), m_rb_cp
// (m_alloc_list +0x1048, count +0x1084).
class rigid_body_sphere_list;
class rb_capsule_pair;
struct ragdoll_collision_callback {
    Entity* m_owner;  // +0x00
    uint8_t _pad4[0xFB0 - 0x04];
    rigid_body_sphere_list** m_rb_colgeom_alloc_list;  // +0xFB0
    uint8_t _padFB4[0x1004 - 0xFB4];
    int     m_rb_colgeom_count;  // +0x1004
    uint8_t _pad1008[0x1048 - 0x1008];
    rb_capsule_pair** m_rb_cp_alloc_list;  // +0x1048
    uint8_t _pad104C[0x1084 - 0x104C];
    int     m_rb_cp_count;  // +0x1084

    void set(Entity* const owner);  // ?set@ragdoll_collision_callback@@QAEXQAVEntity@@@Z
    void get_all_collisions();  // ?get_all_collisions@ragdoll_collision_callback@@QAEXXZ
    void collide_terrain();  // ?collide_terrain@ragdoll_collision_callback@@QAEXXZ
    void process_environment_collision_events();  // ?process_environment_collision_events@ragdoll_collision_callback@@QAEXXZ
    void remove_colgeom(int rb_id);  // ?remove_colgeom@ragdoll_collision_callback@@QAEXH@Z
    rigid_body_sphere_list* get_colgeom(int rb_id);  // ?get_colgeom@ragdoll_collision_callback@@QAEPAVrigid_body_sphere_list@@H@Z
    rigid_body_sphere_list* add_colgeom(int rb_id);  // ?add_colgeom@ragdoll_collision_callback@@QAEPAVrigid_body_sphere_list@@H@Z
};

// bone_mass_info (physics.o RBRagdoll.cpp; 464 bytes). Layout verified from
// IDA ordinal 6939 (matches set 0x6F42C0 and calc_stuff 0x6F9070).
class bone_mass_info {
public:
    int   m_b1;           // +0x00
    int   m_b2;           // +0x04
    float m_percent;      // +0x08
    uint8_t _pad0C[0x10 - 0x0C];
    math::Position3 m_b1_adjust_p2_loc;  // +0x10
    math::Position3 m_b1_adjust_p1_loc;  // +0x20
    float m_p1_radius;    // +0x30
    float m_p2_radius;    // +0x34
    int   m_collision_sphere_count;  // +0x38
    uint8_t _pad3C[0x40 - 0x3C];
    math::Position3 m_p1; // +0x40
    math::Position3 m_p2; // +0x50
    math::Position3 m_com;  // +0x60
    math::Position3 m_capsule_p1;  // +0x70
    math::Position3 m_capsule_p2;  // +0x80
    float m_capsule_radius;  // +0x90
    uint8_t _pad94[0xA0 - 0x94];
    math::Position3 m_capsule_b1_adjust_p2_loc;  // +0xA0
    float m_entity_collision_sphere_radius;  // +0xB0
    float m_mass;         // +0xB4
    math::Position3 m_inertia;  // +0xC0
    float m_inertia_sphere_radius;  // +0xD0
    float m_friction_k;   // +0xD4
    float m_damp_k;       // +0xD8
    int   m_rb_id;        // +0xDC
    int   m_rb_parent_id; // +0xE0
    int   m_rb_bone;      // +0xE4
    int   m_rb_parent_bone;  // +0xE8
    int   m_joint_type;   // +0xEC
    float m_theta_min;    // +0xF0
    float m_theta_max;    // +0xF4
    uint8_t _padF8[0x100 - 0xF8];
    math::Position3 m_rb_parent_pivot_loc;  // +0x100
    math::Position3 m_rb_pivot_loc;         // +0x110
    math::Dir3 m_rb_parent_axis_loc;  // +0x120
    math::Dir3 m_rb_axis_loc;         // +0x130
    math::Dir3 m_rb_parent_ref_loc;   // +0x140
    math::Dir3 m_rb_ref_loc;          // +0x150
    int   m_joint_limit_count;  // +0x160
    math::Dir3 m_joint_limit_axis[4];  // +0x170
    float m_joint_limit_angle[4];      // +0x1B0
    float m_joint_power;               // +0x1C0

    enum joint_type_e {
        JOINT_TYPE_NONE = 0,
        JOINT_TYPE_HINGE = 1,
        JOINT_TYPE_SWIVEL = 2,
    };

    void set(int b1, int b2, float p1_radius, float p2_radius, float percent,
             const math::Position3& b1_adjust_p2_loc,
             int collision_sphere_count, float mass,
             float inertia_sphere_radius, float friction_k, int rb_id,
             int rb_parent_id, joint_type_e joint_type, float theta_min,
             float theta_max, const math::Dir3& rb_parent_axis_loc,
             const math::Dir3& rb_axis_loc,
             const math::Dir3& rb_parent_ref_loc,
             const math::Dir3& rb_ref_loc, float power);  // ?set@bone_mass_info@@QAEXHHMMMABVPosition3@math@@HMMMHHW4joint_type_e@1@MMABVDir3@3@222M@Z
    void calc_stuff(Entity* const owner);  // ?calc_stuff@bone_mass_info@@QAEXQAVEntity@@@Z
};

// HashString (broc_types.h view; local copy - 4 bytes)
class HashString {
public:
    unsigned int mHash;  // +0x00
    HashString();  // ??0HashString@@QAE@XZ
    HashString(Broc::string& str);  // ??0HashString@@QAE@AAVstring@Broc@@@Z
};

// hash_const_t (g_local.h view; local copy - only physics fields used)
struct hash_const_t {
    uint8_t    _pad[0xA8];
    HashString goal;          // +0xA8
    uint8_t    _padAC[0x114 - 0xAC];
    HashString physicsdone;   // +0x114
    HashString physicsstart;  // +0x118
    HashString flipped;       // +0x11C
};
// ?hash_const@@3Uhash_const_t@@A (g.o data @ 0xED2AB0)
extern hash_const_t hash_const;

// PadAliasMgr (game.o PadAliasMgr.cpp view; class tag V matches
// ?sInst@PadAliasMgr@@2PAV1@A). sInst + Context methods defined in g_cmd.cpp.
enum EPadAliasButton {
    kPadAliasButtonInvalid = -1,
    kPadAliasButtonGas = 0,
    kPadAliasButtonReverse = 1,
    kPadAliasButtonHandBrake = 2,
    kPadAliasButtonAlignTurret = 3,
    kPadAliasButtonFireCoax = 4,
    kPadAliasButtonSwitchSeats = 5,
};
enum EPadAliasStick {
    kPadAliasStickInvalid = -1,
    kPadAliasStickVehicleSteering = 0,
    kPadAliasStickTankSteering = 1,
};
class PadAliasMgr {
public:
    struct Context {
        int  GetButtonValue(int ctrlNum, EPadAliasButton buttonAlias);  // ?GetButtonValue@Context@PadAliasMgr@@QAEHHW4EPadAliasButton@@@Z
        void GetStickValue(int ctrlNum, EPadAliasStick stickAlias,
                           int& stickX, int& stickY);  // ?GetStickValue@Context@PadAliasMgr@@QAEXHW4EPadAliasStick@@AAH1@Z
    };
    Context mCtx[3];            // +0x00 (0x148 stride)
    static PadAliasMgr* sInst;  // ?sInst@PadAliasMgr@@2PAV1@A @ 0xF4F458
};

// ServerTime (core.o view; class tag V + public static match repo mangling
// ?sInst@ServerTime@@2V1@A). Definition in g_globals.cpp.
class ServerTime {
public:
    unsigned int mNumTicksElapsed;  // +0x00
    int          mTickMSec;         // +0x04
    float        mTickDelta;        // +0x08
    float        mTickDeltaInv;     // +0x0C
    float        mElapsedTime;      // +0x10
    static ServerTime sInst;
};

// GamePause (cg.o / game.o; ?IsGamePaused@GamePause@@SA_NH@Z, defined in
// g_entity_misc.cpp)
struct GamePause {
    static bool IsGamePaused(int client);
};

namespace LocalClient {
int PortToValidClient(int port);  // ?PortToValidClient@LocalClient@@YAHH@Z (cl.o)
}

// physics.o data globals (RBVehicleController.cpp; addresses from IDA)
float hand_brake_min_speed = 300.0f;   // 0xE36ACC
float steer_velocity = 400.0f;         // 0xE36AD0
float thresh = 2.0f;                   // 0xE36AD4
float steer_reduce_frac = 0.5f;        // 0xE36AD8
float vehicleDeadZone_0 = 40.0f;       // 0xE36B04
float tweaker = 11.0f;                 // 0xE36B24

// per-client controller port array (cl.o ?dword_F6A28C@@3HA @ 0xF6A28C; the
// tank/strafe flag byte lives 8 bytes into each 3208-byte record)
extern int dword_F6A28C[4 * 802];

bool IsLocalPlayer(Entity* entity);  // ?IsLocalPlayer@@YA_NPAVEntity@@@Z (g.o)
    bool IsPlayerFullySeatedInVehicle(Entity* player);  // ?IsPlayerFullySeatedInVehicle@@YA_NPAVEntity@@@Z (g.o)
bool Entity_has_zone_collision(const void* self);  // game.o C bridge
int RecalibrateInput(int val);  // ?RecalibrateInput@@YAHH@Z (cl.o)
extern int g_vehicle_button_threshold;  // ?g_vehicle_button_threshold@@3HA (physics.o @ 0xE01F04)
float vectoyaw(float* vec);  // ?vectoyaw@@YAMPAM@Z (core.o)
float AngleNormalize180Accurate(float angle);  // ?AngleNormalize180Accurate@@YAMM@Z (core.o)
void Axis4ToAngles(const float (*axis)[4], float* angles);  // ?Axis4ToAngles@@YAXPAY03$$CBMPAM@Z (core.o)
// make_rotate(Mat43&, Dir3 const&, float, float) - pulse_sum.h inline
void make_rotate(math::Mat43& m, const math::Dir3& u, float ca, float sa);
// game.o raw segment collide helpers (g_cm_load.cpp)
struct cdlPlane { int packed[4]; };  // 16 bytes
struct cdl_object_t;
bool collide_segment(const cdl_object_t& obj, const math::Dir3* vert_list,
                     const unsigned char* index_list,
                     unsigned short first_vert, int num_indices,
                     const math::Position3& p0, const math::Position3& p1,
                     float& t, math::Position3& normal,
                     int* tid);  // game.o 0x61EB80
bool collide_box_segment(const math::Position3& p0,
                         const math::Position3& p1,
                         const math::Position3& bmin,
                         const math::Position3& bmax, float& t,
                         math::Position3* normal);  // game.o 0x60C720
bool collide_brush_segment(const math::Position3& p0,
                           const math::Position3& p1,
                           const math::Position3& bmin,
                           const math::Position3& bmax,
                           const cdlPlane* sides, unsigned int nsides,
                           float& t, math::Position3* normal);  // game.o 0x61ACC0
void* G_GetModel(const char* modelName, TPakId pakId);  // ?G_GetModel@@YAPAXPBDW4TPakId@@@Z (g.o)
Entity* G_Spawn(TPakId pakId);  // ?G_Spawn@@YAPAVEntity@@W4TPakId@@@Z (g.o)
int G_CallSpawnEntity(Entity* ent);  // ?G_CallSpawnEntity@@YAHPAVEntity@@@Z (g.o)
void SV_SetBrushModel(Entity* ent);  // ?SV_SetBrushModel@@YAXPAVEntity@@@Z (sv.o)
void CalculatePhysData(Entity* ent, IVPointer<PhysData> physData);  // ?CalculatePhysData@@YAXPAVEntity@@V?$IVPointer@VPhysData@@@@@Z
void CalculatePhysData(Entity* ent, float& mass, float& friction,
                       float& bounce);  // ?CalculatePhysData@@YAXPAVEntity@@AAM11@Z
// infoSurfaceProperties (game.o data @ 0xDF8130)
struct infoSurfProperty_t {
    float density;   // +0x00
    float friction;  // +0x04
};
// ?infoSurfaceProperties@@3PAUinfoSurfProperty_t@@A (game.o data @ 0xDF8130)
infoSurfProperty_t infoSurfaceProperties[23] = {
    {0.2f, 0.8f}, {0.8f, 1.0f}, {1.0f, 0.9f}, {0.1f, 0.9f},
    {0.8f, 1.0f}, {0.3f, 0.8f}, {1.0f, 0.5f}, {0.1f, 1.0f},
    {0.1f, 1.0f}, {0.2f, 0.7f}, {0.2f, 0.6f}, {0.2f, 0.1f},
    {0.3f, 1.0f}, {0.4f, 0.6f}, {0.1f, 0.9f}, {0.2f, 1.0f},
    {0.5f, 1.0f}, {0.1f, 0.7f}, {0.2f, 0.4f}, {1.0f, 0.4f},
    {0.3f, 0.9f}, {0.5f, 1.0f}, {0.2f, 0.5f},
};
// BrocAPI view (broc_types.h; gpBrocAPI tag must stay U for the mangling)
struct BrocAPI;
extern BrocAPI* gpBrocAPI;  // ?gpBrocAPI@@3PAUBrocAPI@@A (Broc.o @ 0xF3ABDC)
struct TPakInfoLocal {
    uint8_t _pad[0xB4];
    TPakId  mPakId;  // +0xB4
};
struct BrocAPILocal {
    uint8_t _pad[0x61C];
    TPakInfoLocal* (*mGetPakVector)(float x, float y, float z);  // +0x61C
};
// XModel view (lod[5] +0x24, name InplaceString +0x48)
struct XModelLodLocal {
    uint8_t _pad[8];
    void*   xmodelParts;  // +0x08
};
struct XModelLocal {
    uint8_t         _pad[0x24];
    XModelLodLocal* lod[5];  // +0x24
    uint8_t         _pad38[0x48 - 0x38];
    const char*     mStr;    // +0x48 (name)
};
// DObjSkelMat - DObj skeleton matrix (64 bytes; core_types.h view)
struct DObjSkelMat {
    float axis[3][4];  // +0x00
    float origin[4];   // +0x30
};
DObjSkelMat* SV_DObjGetMatrixArray(Entity* entity);  // ?SV_DObjGetMatrixArray@@YAPAUDObjSkelMat@@PAVEntity@@@Z (sv.o)
DObjSkelMat* DObjGetMatrixArray(const DObj* obj, int modelIndex);  // ?DObjGetMatrixArray@@YAPAUDObjSkelMat@@PBVDObj@@H@Z (render.o)
void DObjMatriceModelToLocal(Entity* owner);  // ?DObjMatriceModelToLocal@@YAXPAVEntity@@@Z
void CopyMatrix(math::Mat43& out, DObjSkelMat& in);  // ?CopyMatrix@@YAXAAVMat43@math@@AAUDObjSkelMat@@@Z (render.o inline)
// dword_E01E5C - physics.o bone-index mapping table (10 x { boneIdx, name })
extern int dword_E01E5C[20];

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
    float   m_speed_max;          // +0x00
    float   m_accel_max;          // +0x04
    float   m_reverse_scale;      // +0x08
    float   m_steer_angle_max;    // +0x0C
    float   m_steer_speed;        // +0x10
    float   m_wheel_radius;       // +0x14
    float   m_susp_spring_k;      // +0x18
    float   m_susp_damp_k;        // +0x1C
    float   m_susp_adj;           // +0x20
    float   m_susp_hard_limit;    // +0x24
    float   m_tire_fric_fwd;      // +0x28
    float   m_tire_fric_side;     // +0x2C
    float   m_tire_fric_brake;    // +0x30
    float   m_tire_fric_hand_brake;  // +0x34
    float   m_body_mass;          // +0x38
    float   m_mass_center_delta_x;  // +0x3C
    float   m_mass_center_delta_y;  // +0x40
    float   m_mass_center_delta_z;  // +0x44
    float   m_roll_stability;     // +0x48
    float   m_roll_resistance;    // +0x4C
    float   m_upright_strength;   // +0x50
    float   m_tilt_fakey;         // +0x54
    float   m_peel_out_max_speed;  // +0x58
    float   m_inertia_scale_x;    // +0x5C
    float   m_tire_damp_coast;    // +0x60
    float   m_tire_damp_brake;    // +0x64
    float   m_tire_damp_hand;     // +0x68
    int     m_traction_type;      // +0x6C
    uint8_t _pad70[0xB0 - 0x70];
    math::Position3 m_bbox_min;   // +0xB0
    math::Position3 m_bbox_max;   // +0xC0

    static vehicle_rb_parameter* GetRBVehParameter(
        const char* name);  // ?GetRBVehParameter@vehicle_rb_parameter@@SAPAV1@PBD@Z
};

// RBVehicleController (physics.o RBVehicleController.cpp). 64-byte layout
// verified from IDA (mVehicleController at rb_vehicle +0x2D0).
class RBVehicleController {
public:
    math::Position3 m_script_goal_position;  // +0x00
    float m_script_goal_radius;              // +0x10
    float m_script_goal_speed;               // +0x14
    float m_stuck_time;                      // +0x18
    math::Position3 m_stuck_position;        // +0x20
    float m_hold_controls_time;              // +0x30

    void SetScriptTarget(rb_vehicle& rbveh, const math::Position3& goal_position,
                         float goal_radius, float goal_speed);  // ?SetScriptTarget@RBVehicleController@@QAEXAAVrb_vehicle@@ABVPosition3@math@@MM@Z
    void UpdateControls(rb_vehicle& rbveh);  // ?UpdateControls@RBVehicleController@@QAEXAAVrb_vehicle@@@Z

private:
    void UpdateJump(rb_vehicle& rbveh);  // ?UpdateJump@RBVehicleController@@AAEXAAVrb_vehicle@@@Z
    void UpdateControlsDefault(rb_vehicle& rbveh, int controller_port);  // ?UpdateControlsDefault@RBVehicleController@@AAEXAAVrb_vehicle@@H@Z
    void UpdateControlsStrafeMode(rb_vehicle& rbveh, int client_controller);  // ?UpdateControlsStrafeMode@RBVehicleController@@AAEXAAVrb_vehicle@@H@Z
    void UpdateControlsTankStrafeMode(rb_vehicle& rbveh, int controller_port);  // ?UpdateControlsTankStrafeMode@RBVehicleController@@AAEXAAVrb_vehicle@@H@Z
    void UpdateControlsTank(rb_vehicle& rbveh, int controller_port);  // ?UpdateControlsTank@RBVehicleController@@AAEXAAVrb_vehicle@@H@Z
    void UpdateVehicleInputs(rb_vehicle& rbveh, float target_yaw,
                             float target_accel,
                             float forward_preference);  // ?UpdateVehicleInputs@RBVehicleController@@AAEXAAVrb_vehicle@@MMM@Z
    void UpdateScriptVehicleControl(rb_vehicle& rbveh);  // ?UpdateScriptVehicleControl@RBVehicleController@@AAEXAAVrb_vehicle@@@Z
};

// rb_vehicle (physics.o RBVehicle.cpp). Layout verified against the ctor
// (0x6FC080) + is_peeling_out (0x6F4AD0) + path_constraint_update (0x6F5B60)
// disassembly. class tag (V) required for pool/param manglings.
class rb_vehicle {
public:
    int    m_wheel_effect_state[4];  // +0x00 (wheel_effect_state_e)
    Handle m_wheel_effects[4];       // +0x10
    Handle m_exhaust_effect;         // +0x20
    int    m_wheel_bone_indices[8];  // +0x24
    math::Mat43 m_wheel_orig_relpo[8];  // +0x50
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
    uint8_t _pad284[0x290 - 0x284];
    math::Mat43 m_prev_rb_mat;                   // +0x290 (teleport writes w at +0x2C0)
    RBVehicleController mVehicleController;       // +0x2D0 (64 bytes)
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

    static int sRenderAllVehicles;  // ?sRenderAllVehicles@rb_vehicle@@2HA

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
    void start_physics();         // ?start_physics@rb_vehicle@@QAEXXZ
    void start_path(int attach_mode);  // ?start_path@rb_vehicle@@QAEXH@Z
    void set_brake(float braking);    // ?set_brake@rb_vehicle@@QAEXM@Z (inline)
    void set_throttle(float throttle);  // ?set_throttle@rb_vehicle@@QAEXM@Z (inline)
    void set_hand_brake(float braking);  // ?set_hand_brake@rb_vehicle@@QAEXM@Z (inline)
    void set_steer_factor(float steer_factor);  // ?set_steer_factor@rb_vehicle@@QAEXM@Z (inline)
    void update_from_network(const math::Position3& position,
                             const math::Position3& angles,
                             const math::Dir3& vel,
                             const math::Dir3& aVel);  // ?update_from_network@rb_vehicle@@QAEXABVPosition3@math@@0ABVDir3@3@1@Z
    void update_from_scene_anim(const math::Position3& position,
                                const math::Position3& angles,
                                const math::Dir3& vel);  // ?update_from_scene_anim@rb_vehicle@@QAEXABVPosition3@math@@0ABVDir3@3@@Z
    math::Dir3 get_velocity() const;  // ?get_velocity@rb_vehicle@@QBE?AVDir3@math@@XZ
    math::Dir3 get_angular_velocity() const;  // ?get_angular_velocity@rb_vehicle@@QBE?AVDir3@math@@XZ
    math::Position3 get_rb_position() const;  // ?get_rb_position@rb_vehicle@@QBE?AVPosition3@math@@XZ
    math::Dir3 get_rb_angles() const;         // ?get_rb_angles@rb_vehicle@@QBE?AVDir3@math@@XZ
    void teleport(const Broc::vector& vSpawnPos, const Broc::vector* vAngles);  // ?teleport@rb_vehicle@@QAEXABUvector@Broc@@PBU23@@Z
    static void frame_prolog_all_systems(float delta_t);  // ?frame_prolog_all_systems@rb_vehicle@@SAXM@Z
    static void frame_epilog_all_systems(float delta_t);  // ?frame_epilog_all_systems@rb_vehicle@@SAXM@Z
    static void debug_render_all();  // ?debug_render_all@rb_vehicle@@SAXXZ
    void debug_render();             // ?debug_render@rb_vehicle@@QAEXXZ
private:
    void _update_prolog(float delta_t);  // ?_update_prolog@rb_vehicle@@AAEXM@Z
    void _update_epilog(float delta_t);  // ?_update_epilog@rb_vehicle@@AAEXM@Z
    void _update_unpause();       // ?_update_unpause@rb_vehicle@@AAEXXZ
    void _align_wheels();         // ?_align_wheels@rb_vehicle@@AAEXXZ
    void _set_default_pose_wheels_only();  // ?_set_default_pose_wheels_only@rb_vehicle@@AAEXXZ
    float _calc_initial_susp_spring_k(
        rigid_body_constraint_wheel* wheel_constraint);  // ?_calc_initial_susp_spring_k@rb_vehicle@@AAEMPAVrigid_body_constraint_wheel@@@Z
    void _update_friction(float delta_t);  // ?_update_friction@rb_vehicle@@AAEXM@Z
    void _update_fakey_stuff(float delta_t);  // ?_update_fakey_stuff@rb_vehicle@@AAEXM@Z
    void _update_orientation_constraint();  // ?_update_orientation_constraint@rb_vehicle@@AAEXXZ
    void update_steering(float delta_t);  // ?update_steering@rb_vehicle@@QAEXM@Z
    void _update_wheel_effects(float delta_t);  // ?_update_wheel_effects@rb_vehicle@@AAEXM@Z
    void calc_tread_matrices();  // ?calc_tread_matrices@rb_vehicle@@QAEXXZ
    void get_wheel_matrix(int i, math::Mat43* mat) const;  // ?get_wheel_matrix@rb_vehicle@@QBEXHPAVMat43@math@@@Z
};

// XBoneHierarchy / XModelParts partial views (render.o types; XModelParts
// mTransforms is InplaceVector<Mat43::Packed> at +0x00, mHierarchy at +0x10).
struct XBoneHierarchyLocal {
    uint8_t      _pad0[4];
    unsigned int mNameHash;  // +0x04
    int          mParentIndex;  // +0x08
};
struct XModelPartsLocal {
    InplaceVector<math::Mat43::Packed> mTransforms;  // +0x00
    uint8_t               _pad08[0x10 - 0x08];
    unsigned int          mHierarchySize;  // +0x10
    XBoneHierarchyLocal*  mHierarchy;      // +0x14
};

// physics.o hash globals (-1 until the hash system fills them at startup)
unsigned int g_tag_left_tread_hash = 0xFFFFFFFF;      // @ 0xF7B084
unsigned int g_tag_right_tread_hash = 0xFFFFFFFF;     // @ 0xF8C0B0
unsigned int g_tag_steeringwheel_hash = 0xFFFFFFFF;   // @ 0xF8C0B4
unsigned int g_tag_left_gear_hash = 0xFFFFFFFF;       // @ 0xF8D620
unsigned int g_tag_right_gear_hash = 0xFFFFFFFF;      // @ 0xF8C140

// ?sRenderAllVehicles@rb_vehicle@@2HA (physics.o data @ 0xF794B4; the owning
// definition lives in g_scr_vehicle.cpp)
// ?sphere_size@@3MA (physics.o data @ 0xE36AC8; 0x40A00000 = 5.0)
float sphere_size = 5.0f;
// ?colorWhite@@3QBMB (render.o data @ 0xD0161C; {1,1,1,1})
static const float colorWhite[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

// G_DebugCircleEx (game.o 0x457170; ported in g_main.cpp)
void G_DebugCircleEx(const float* center, float radius, const float* dir,
                     const float* color, int depthTest, int duration);

// InplaceVector<Mat43::Packed>::operator[] (g.o inline COMDAT 0x4AC980)
static inline math::Mat43::Packed& iv_packed_at(
    InplaceVector<math::Mat43::Packed>* v, unsigned int index)
{
    unsigned int v2 = index;
    if (index >= v->mSize)
    {
        AeAssert::gCurrentAuthor = AeAssert::ARO;
        AeAssert::gCurrentFile = "../ae\\inplace/InplaceVector.h";
        AeAssert::gCurrentLine = 81;
        AeAssert::gCurrentExpr = "index < mSize";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Bounds check"))
            __debugbreak();
        if (index >= v->mSize)
            v2 = 0;
    }
    return v->mList[v2];
}

// XModelParts::GetBoneIndex(uint nameHash) (render.o inline COMDAT 0x6EB6C0)
static inline unsigned int xmodel_parts_get_bone_index(
    XModelPartsLocal* this_, unsigned int nameHash)
{
    if (this_->mHierarchySize == 0)
        return -1;
    unsigned int v3 = 0;
    while (1)
    {
        unsigned int v4 = v3;
        if (v3 >= this_->mHierarchySize)
        {
            AeAssert::gCurrentAuthor = AeAssert::ARO;
            AeAssert::gCurrentFile = "../ae\\inplace/InplaceVector.h";
            AeAssert::gCurrentLine = 81;
            AeAssert::gCurrentExpr = "index < mSize";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("Bounds check"))
                __debugbreak();
            if (v3 >= this_->mHierarchySize)
                v4 = 0;
        }
        if (this_->mHierarchy[v4].mNameHash == 0)
        {
            AeAssert::gCurrentAuthor = AeAssert::JRS;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\XModelParts.h";
            AeAssert::gCurrentLine = 203;
            AeAssert::gCurrentExpr = "mHierarchy[i].mNameHash != 0";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("Uninitialized part being used?"))
                __debugbreak();
        }
        unsigned int v5 = v3;
        if (v3 >= this_->mHierarchySize)
        {
            AeAssert::gCurrentAuthor = AeAssert::ARO;
            AeAssert::gCurrentFile = "../ae\\inplace/InplaceVector.h";
            AeAssert::gCurrentLine = 81;
            AeAssert::gCurrentExpr = "index < mSize";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("Bounds check"))
                __debugbreak();
            if (v3 >= this_->mHierarchySize)
                v5 = 0;
        }
        if (this_->mHierarchy[v5].mNameHash == nameHash)
            break;
        if (++v3 >= this_->mHierarchySize)
            return -1;
    }
    return v3;
}

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
    bool can_have_event();  // ?can_have_event@rb_extra_info@@QAE_NXZ (0x719A90)
    void set(Entity* const ent, rigid_body* const rb,
             const math::Mat43& transform, float bs_radius,
             const math::Position3& bs_center_loc);  // ?set@rb_extra_info@@QAEXQAVEntity@@QAVrigid_body@@ABVMat43@math@@MABVPosition3@5@@Z
};

template <typename T, int N> class phys_static_memory_pool;
// ?g_list_rb_extra_info@@3V?$phys_static_memory_pool@Vrb_extra_info@@$0CD@@@A
// (physics.o data @ 0xE2A000; definition in the pool section below)
extern phys_static_memory_pool<rb_extra_info, 35> g_list_rb_extra_info;
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
// ?dampValue@@3MA (physics.o data @ 0xE36AC0)
float dampValue = 0.5f;

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
    uint8_t         _pad24[0x30 - 0x24];
    math::Position3 m_p1;      // +0x30
    math::Position3 m_p2;      // +0x40

    void set(const math::Position3& p1_loc, const math::Position3& p2_loc,
             float r);  // ?set@rb_collision_capsule@@QAEXABVPosition3@math@@0M@Z
    void xform(const math::Mat43& mat);  // ?xform@rb_collision_capsule@@QAEXABVMat43@math@@@Z
};

// ea: 0x6F4650
void rb_collision_capsule::set(const math::Position3& p1_loc,
                               const math::Position3& p2_loc, float r)
{
    m_p1_loc.v = p1_loc.v;
    m_p2_loc.v = p2_loc.v;
    m_r = r;
}

// ea: 0x6FB690
void rb_collision_capsule::xform(const math::Mat43& mat)
{
    m_p1.v = _mm_add_ps(
        _mm_add_ps(
            _mm_mul_ps(_mm_shuffle_ps(m_p1_loc.v, m_p1_loc.v, 0), mat.x.v),
            _mm_mul_ps(_mm_shuffle_ps(m_p1_loc.v, m_p1_loc.v, 85), mat.y.v)),
        _mm_add_ps(
            _mm_mul_ps(_mm_shuffle_ps(m_p1_loc.v, m_p1_loc.v, 170), mat.z.v),
            mat.w.v));
    m_p2.v = _mm_add_ps(
        _mm_add_ps(
            _mm_mul_ps(_mm_shuffle_ps(m_p2_loc.v, m_p2_loc.v, 0), mat.x.v),
            _mm_mul_ps(_mm_shuffle_ps(m_p2_loc.v, m_p2_loc.v, 85), mat.y.v)),
        _mm_add_ps(
            _mm_mul_ps(_mm_shuffle_ps(m_p2_loc.v, m_p2_loc.v, 170), mat.z.v),
            mat.w.v));
}


// DObj (render.o; local stub view). copy_skeleton disasm reads numBones at
// +0xCF; GetMat/GetBoneIndex are render.o symbols (?GetMat@DObj@@QAEABVMat43@
// math@@H@Z / ?GetBoneIndex@DObj@@QBEHPBD@Z) - stub until render.o is ported.
class DObj {
public:
    uint8_t _pad0[0x60];
    unsigned char modelParents[8];  // +0x60
    void*   skel;                   // +0x70 (DSkel*)
    uint8_t _pad74[0x80 - 0x74];
    void*   models[8];              // +0x80 (IVPointer<XModel>[8]: value/pakId pairs)
    uint8_t _padA0[0xC0 - 0xA0];
    int     mPakId;            // +0xC0
    void*   mPhysDataValue;    // +0xC4 (IVPointerRaw mPhysData)
    int     mPhysDataPakId;    // +0xC8
    uint8_t _padCC[0xCE - 0xCC];
    unsigned char numModels;  // +0xCE
    unsigned char numBones;   // +0xCF

    const math::Mat43& GetMat(int boneIndex);
    int GetBoneIndex(const char* name) const;
    int GetBoneIndexInternal(unsigned int nameHash) const;  // ?GetBoneIndexInternal@DObj@@QBEHI@Z (render.o)
    int GetHighLOD();  // ?GetHighLOD@DObj@@QAEHXZ (physics.o inline 0x71C360)
    int GetBoneParent(int boneIndex);  // ?GetBoneParent@DObj@@QAEHH@Z (render.o; stub)
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
int DObj::GetBoneIndexInternal(unsigned int nameHash) const
{
    (void)nameHash;
    return -1;
}

// nalGeneric (nal_generic.o / nal_init.o; minimal views for setup_physics).
// Cross-object stubs - the real nal library is ported separately.
namespace nalGeneric {
class nalGenericPose;
class nalGenericSkeleton;
class nalGenericAnim;
class nalGenericInstance;
struct nalMatrix4x4 { float m[4][4]; };

class nalGenericPose {
public:
    uint8_t _pad[0x8];
    void*   PoseData;      // +0x08
    bool    AllocedData;   // +0x0C

    nalGenericPose(const nalGenericSkeleton* skel, int flags);  // 0x86E660 (stub)
    ~nalGenericPose();                                          // 0x868D80 (stub)
};

class nalGenericSkeleton {
public:
    void* __vftable;  // +0x00
    uint8_t _pad4[0xC8 - 0x04];
    nalGenericPose DefaultPose;  // +0xC8
    static void* vtbl_ptr;       // ?vtbl_ptr@nalGenericSkeleton@nalGeneric@@2PAXA @ 0x10E6D04

    void GetBoneMatrices(const nalGenericPose& pose, nalMatrix4x4* matrices,
                         int lod);  // 0x869500 (stub)
};
void* nalGenericSkeleton::vtbl_ptr = nullptr;

class nalGenericAnim {
public:
    void* __vftable;  // +0x00
    uint8_t _pad4[0x30 - 0x04];
    nalGenericSkeleton* Skeleton;  // +0x30 (nalAnimClass<nalAnyPose>::Skeleton)

    nalGenericInstance* CreateInstance(nalGenericSkeleton* skeleton);  // 0x518430 (stub)
};

class nalGenericInstance {
public:
    void* __vftable;  // +0x00
    uint8_t _pad4[0x0C - 0x04];
    nalGenericSkeleton* Skeleton;  // +0x0C

    // GetPose - ea: 0x71A210 (physics.o inline COMDAT)
    void GetPose(int index, nalGenericPose& pose, int lod);
    // 4-arg nal_generic.o callee (0x86DFC0; cross-object stub)
    void GetPose(int index, nalGenericPose& pose,
                 const nalGenericPose& defaultPose, int lod);
};

class nalAnyPose {};
template <typename T> class nalAnimClass {};
}

// cdGetAnim (streamer.o 0x677BE0; cross-object stub)
nalGeneric::nalAnimClass<nalGeneric::nalAnyPose>*
cdGetAnim(TPakId pakId, const tlFixedString& name);

// physics.o data @ 0xF91704 / 0xF91708 / 0xF9170C (RBRagdoll.cpp statics)
nalGeneric::nalGenericInstance* inst = nullptr;
nalGeneric::nalGenericAnim* anim = nullptr;
int S45_1_guard = 0;

// Cross-object nal stubs (correct manglings; bodies ported with nal library)
inline nalGeneric::nalGenericPose::nalGenericPose(
    const nalGenericSkeleton* skel, int flags)
{
    (void)skel; (void)flags;
}
inline nalGeneric::nalGenericPose::~nalGenericPose() {}
inline void nalGeneric::nalGenericSkeleton::GetBoneMatrices(
    const nalGenericPose& pose, nalMatrix4x4* matrices, int lod)
{
    (void)pose; (void)matrices; (void)lod;
}
inline nalGeneric::nalGenericInstance* nalGeneric::nalGenericAnim::CreateInstance(
    nalGenericSkeleton* skeleton)
{
    (void)skeleton;
    return nullptr;
}
inline void nalGeneric::nalGenericInstance::GetPose(
    int index, nalGenericPose& pose, const nalGenericPose& defaultPose,
    int lod)
{
    (void)index; (void)pose; (void)defaultPose; (void)lod;
}
nalGeneric::nalAnimClass<nalGeneric::nalAnyPose>*
cdGetAnim(TPakId pakId, const tlFixedString& name)
{
    (void)pakId; (void)name;
    return nullptr;
}

// ea: 0x71A210 (physics.o inline COMDAT)
void nalGeneric::nalGenericInstance::GetPose(int index,
                                             nalGenericPose& pose, int lod)
{
    GetPose(index, pose, Skeleton->DefaultPose, lod);
}

// ea: 0x71C360 (physics.o inline COMDAT)
int DObj::GetHighLOD()
{
    int v2 = 0;
    void** model_slot = (void**)((char*)models + 0);
    ValidatePakId((TPakId)(uintptr_t)model_slot[1]);
    XModelLocal* mValue = (XModelLocal*)model_slot[0];
    for (int i = 9; i < 14; ++i)
    {
        if (((unsigned int*)mValue)[i] != 0)
            return v2;
        ++v2;
    }
    AeAssert::gCurrentAuthor = AeAssert::ARO;
    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\DObj.h";
    AeAssert::gCurrentLine = 242;
    AeAssert::gCurrentExpr = nullptr;
    if (AeAssert::Error("unable to find low lod"))
        __debugbreak();
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
        uint8_t        _pad08[0x0C - 0x08];
        void*          bmodel;       // +0x0C (DCGSet*)
        uint8_t        _pad10[0x30 - 0x10];
        math::Position3 absmin;      // +0x30
        math::Position3 absmax;      // +0x40
        uint8_t        _pad50[0x64 - 0x50];
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
    unsigned int mHandle;        // +0x234 (DbLinkedHandle mVal)
    uint8_t _pad238[0x23C - 0x238];
    DObj* mDObj;                 // +0x23C
    uint8_t _pad240[0x248 - 0x240];
    biped_phys_info* mBPInfo;    // +0x248
    void*   mDestructibleValue;  // +0x24C (IVPointer<Destructible>)
    int     mDestructiblePakId;  // +0x250
    void* client;                // +0x254 (Client*; ps.viewangles +0xD0)
    void* actor;                 // +0x258 (actor_s*)
    void* sentient;              // +0x25C (sentient_s*)
    void* scr_vehicle;           // +0x260 (scr_vehicle_t*)
    uint8_t _pad264[0x270 - 0x264];
    void*   mModelValue;         // +0x270 (IVPointer<XModel>)
    int     mModelPakId;         // +0x274
    uint8_t _pad278[0x27C - 0x278];
    Broc::string mClassName;     // +0x27C
    HashString   mClassNameHash; // +0x280
    uint8_t _pad284[0x28C - 0x284];
    Broc::string mTarget;        // +0x28C (path node target name)
    uint8_t _pad290[0x2B0 - 0x290];
    uint8_t physicsObject;       // +0x2B0
    uint8_t _pad2B1[0x2B8 - 0x2B1];
    int32_t takedamage;          // +0x2B8
    uint8_t _pad2BC[0x2C0 - 0x2BC];
    int32_t spawnflags;          // +0x2C0
    int32_t  flags;              // +0x2C4
    unsigned int mFlags;         // +0x2C8 (Bitmask<unsigned int>)
    uint8_t _pad2CC[0x31C - 0x2CC];
    float    speed;              // +0x31C (verified from _update_epilog)
    uint8_t _pad320[0x348 - 0x320];
    int32_t nextthink;           // +0x348
    int32_t think;               // +0x34C (fn_think_e)
    uint8_t _pad350[0x355 - 0x350];
    uint8_t  die;                // +0x355
    uint8_t  _pad356[3];
    int32_t  health;             // +0x358
    uint8_t  _pad35C[0x3D4 - 0x35C];
    void*    tagInfo;            // +0x3D4 (tagInfo_t*)
    uint8_t  _pad3D8[0x3DC - 0x3D8];
    void*    scripted;           // +0x3DC (animscripted_t*)
    uint8_t  _pad3E0[0x45C - 0x3E0];
    proximity_data_t* proximity_data;  // +0x45C

    math::Mat43 CalcAbsMat(int boneIndex);  // ?CalcAbsMat@Entity@@QAE?AVMat43@math@@H@Z
    math::Mat43 GetRelMat(int boneIndex);   // ?GetRelMat@Entity@@QAE?AVMat43@math@@H@Z
    const math::Mat43 CalcRotTranMat43();  // ?CalcRotTranMat43@Entity@@QAE?BVMat43@math@@XZ
    int GetParentBoneIndex(int boneIndex);  // ?GetParentBoneIndex@Entity@@QAEHH@Z (game.o)
    const math::Mat43::Packed& GetBaseRelMat(
        int boneIndex);  // ?GetBaseRelMat@Entity@@QAEABUPacked@Mat43@math@@H@Z (game.o)
    // ?has_zone_collision@Entity@@QAE_NXZ (game.o) - call the C bridge
    void Notify(HashString h);  // ?Notify@Entity@@QAEXVHashString@@@Z (game.o)
    void set_bp_info(biped_phys_info* bpInfo);  // ?set_bp_info@Entity@@QAEXPAVbiped_phys_info@@@Z (game.o)
    void CalcOriginAnglesFromMat();  // ?CalcOriginAnglesFromMat@Entity@@QAEXXZ (game.o)
    bool IsLocalPlayer() const;  // ?IsLocalPlayer@Entity@@QBE_NXZ (game.o)
    int  GetPlayerIndex() const; // ?GetPlayerIndex@Entity@@QBEHXZ (game.o)
};

// EntityHandleDb (g.o view; mElements +0xA8, 0x540 entries). Defined early
// because RBVehicleController::UpdateControls resolves seat occupant handles.
struct EntityHandleDbDbElement {
    Entity* mObject;  // +0x00
    int     mKey;     // +0x04
};
class EntityHandleDb {
public:
    uint8_t _pad[0xA8];  // +0x00 (incl. mFreeIndices BitSet<1344>)
    EntityHandleDbDbElement mElements[0x540];  // +0xA8
    static EntityHandleDb sInst;  // ?sInst@EntityHandleDb@@0V1@A (g.o)

    Entity* GetObject(int idx);  // HandleDb<Entity,1344,SizedHandle<12,20>> (g.o)
};

// ?GetObject@?$HandleDb@VEntity@@$0FEA@V?$SizedHandle@$0M@$0BE@@@@@QBEPAVEntity@@H@Z
// (g.o; stub until g.o is ported)
Entity* EntityHandleDb::GetObject(int idx)
{
    (void)idx;
    return nullptr;
}

// ?DObjGetBasePose@@YAXPAVDObj@@@Z (render.o; stub until render.o is ported)
void DObjGetBasePose(DObj* obj)
{
    (void)obj;
}

// ?set_bp_info@Entity@@QAEXPAVbiped_phys_info@@@Z (game.o; stub until ported)
void Entity::set_bp_info(biped_phys_info* bpInfo)
{
    mBPInfo = bpInfo;
}

// ea: 0x6F9070
void bone_mass_info::calc_stuff(Entity* const owner)
{
    math::Mat43 b1_abs_mat = owner->CalcAbsMat(m_b1);
    math::Mat43 b2_abs_mat = owner->CalcAbsMat(m_b2);
    __m128 v4 = b1_abs_mat.y.v;
    __m128 v5 = b1_abs_mat.x.v;
    __m128 v6 = b1_abs_mat.z.v;
    m_p1.v = b1_abs_mat.w.v;
    __m128 m_percent_low = _mm_set1_ps(m_percent);
    math::Dir3 v39;
    v39.v = _mm_add_ps(
        b2_abs_mat.w.v,
        _mm_add_ps(
            _mm_add_ps(
                _mm_mul_ps(
                    _mm_shuffle_ps(m_b1_adjust_p2_loc.v,
                                   m_b1_adjust_p2_loc.v, 0),
                    v5),
                _mm_mul_ps(
                    _mm_shuffle_ps(m_b1_adjust_p2_loc.v,
                                   m_b1_adjust_p2_loc.v, 85),
                    v4)),
            _mm_mul_ps(
                _mm_shuffle_ps(m_b1_adjust_p2_loc.v,
                               m_b1_adjust_p2_loc.v, 170),
                v6)));
    __m128 v8 = _mm_set1_ps(1.0f - m_percent);
    m_p2.v = v39.v;
    __m128 v10 = _mm_add_ps(
        _mm_mul_ps(m_p1.v, v8),
        _mm_mul_ps(m_p2.v, m_percent_low));
    __m128 v11 = _mm_shuffle_ps(v5, v4, 68);
    __m128 v12 = _mm_shuffle_ps(_mm_shuffle_ps(v5, v4, 238), v6, 168);
    __m128 v13 = v11;
    __m128 v14 = _mm_shuffle_ps(v11, v6, 221);
    __m128 v15 = _mm_shuffle_ps(v13, v6, 136);
    float v15w = b1_abs_mat.w.v.m128_f32[0];
    m_com.v = _mm_add_ps(
        _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(v10, v10, 0), v14),
                   _mm_mul_ps(_mm_shuffle_ps(v10, v10, 85), v12)),
        _mm_add_ps(
            _mm_mul_ps(_mm_shuffle_ps(v10, v10, 170), v13),
            _mm_xor_ps(
                Float4_SignMask_12,
                _mm_add_ps(
                    _mm_add_ps(
                        _mm_mul_ps(
                            _mm_shuffle_ps(b1_abs_mat.w.v, b1_abs_mat.w.v, 0),
                            v14),
                        _mm_mul_ps(
                            _mm_shuffle_ps(b1_abs_mat.w.v, b1_abs_mat.w.v, 85),
                            v12)),
                    _mm_mul_ps(
                        _mm_shuffle_ps(b1_abs_mat.w.v, b1_abs_mat.w.v, 170),
                        v13)))));
    m_capsule_p1.v.m128_f32[0] = v15w;
    m_capsule_p1.v.m128_f32[1] = b1_abs_mat.w.v.m128_f32[1];
    m_capsule_p1.v.m128_f32[2] = b1_abs_mat.w.v.m128_f32[2];
    m_capsule_p1.v.m128_f32[3] = b1_abs_mat.w.v.m128_f32[3];
    math::Position3 v16;
    v16.v = m_p1.v;
    v39.v = _mm_add_ps(
        b2_abs_mat.w.v,
        _mm_add_ps(
            _mm_add_ps(
                _mm_mul_ps(
                    _mm_shuffle_ps(m_capsule_b1_adjust_p2_loc.v,
                                   m_capsule_b1_adjust_p2_loc.v, 0),
                    b1_abs_mat.x.v),
                _mm_mul_ps(
                    _mm_shuffle_ps(m_capsule_b1_adjust_p2_loc.v,
                                   m_capsule_b1_adjust_p2_loc.v, 85),
                    b1_abs_mat.y.v)),
            _mm_mul_ps(
                _mm_shuffle_ps(m_capsule_b1_adjust_p2_loc.v,
                               m_capsule_b1_adjust_p2_loc.v, 170),
                v6)));
    math::Position3 v17;
    v17.v = m_p2.v;
    m_capsule_p2.v = v39.v;
    __m128 v = _mm_sub_ps(v17.v, v16.v);
    __m128 v19 = _mm_mul_ps(v, v);
    float v38 = v19.m128_f32[0]
        + (v19.m128_f32[1] + v19.m128_f32[2]);
    math::Dir3 v39b;
    v39b.v = v;
    v38 = sqrtf(v38);
    if (v38 <= 0.001f)
    {
        bool v20 = _tlAssert("c:\\cod\\code\\game\\RBRagdoll.cpp", 772,
                             "nbone_dir > .001f", defaultFileName);
        v = v39b.v;
        if (v20)
            __debugbreak();
    }
    __m128 v21 = _mm_set1_ps(1.0f / v38);
    __m128 v22 = _mm_mul_ps(v, v21);
    if (m_rb_id == 0)
    {
        m_p2.v = _mm_sub_ps(
            m_p2.v, _mm_mul_ps(v22, _mm_set1_ps(m_p2_radius * 0.75f)));
    }
    if (m_rb_id == 7 || m_rb_id == 9)
    {
        m_p2.v = _mm_sub_ps(
            m_p2.v, _mm_mul_ps(v22, _mm_set1_ps(m_p2_radius * 0.94999999f)));
    }
    float v26 = (m_p1_radius - 1.7f) - 0.34f;
    float v27 = (m_p2_radius - 1.7f) - 0.34f;
    m_p1_radius = v26;
    m_p2_radius = v27;
    if (v26 <= 0.0f
        && _tlAssert("c:\\cod\\code\\game\\RBRagdoll.cpp", 788,
                     "m_p1_radius > 0.0f", defaultFileName))
        __debugbreak();
    if (m_p2_radius <= 0.0f
        && _tlAssert("c:\\cod\\code\\game\\RBRagdoll.cpp", 789,
                     "m_p2_radius > 0.0f", defaultFileName))
        __debugbreak();
    math::Dir3 unit_inertia;
    float volume;
    nuge::calc_sphere_inertia(m_inertia_sphere_radius, &unit_inertia,
                              &volume);
    math::Dir3 v39c;
    v39c.v = _mm_mul_ps(unit_inertia.v, _mm_set1_ps(m_mass / volume));
    m_inertia.v = v39c.v;
    int m_b1_ = m_b1;
    m_rb_bone = m_b1_;
    int ParentBoneIndex = owner->GetParentBoneIndex(m_b1_);
    m_rb_parent_bone = ParentBoneIndex;
    if (ParentBoneIndex >= 0)
    {
        const math::Mat43::Packed& BaseRelMat = owner->GetBaseRelMat(m_rb_bone);
        math::Dir3 v39d;
        v39d.v.m128_f32[0] = BaseRelMat.w.x;
        v39d.v.m128_f32[1] = BaseRelMat.w.y;
        v39d.v.m128_f32[2] = BaseRelMat.w.z;
        v39d.v.m128_f32[3] = 0.0f;
        m_rb_parent_pivot_loc.v = v39d.v;
        m_rb_pivot_loc.v = _mm_setzero_ps();
    }
}

// Camera (cg.o view; minimal local copy; mVehicleCamMode +0x194 verified from
// UpdateControls disasm: cmp dword ptr [eax+194h], 2/3)
enum EVehicleCameraMode {
    VEH_MODE_FIRSTPERSON = 0,
    VEH_MODE_CHASECAM = 1,
    VEH_MODE_HLO = 2,
    VEH_MODE_STRAFE = 3,
};
struct Camera {
    uint8_t         _pad0[0x30];
    math::Position3 mPrevViewPos;  // +0x30
    math::Position3 mPrevAngles;   // +0x40
    math::Position3 mPrevViewDir;  // +0x50
    uint8_t         _pad60[0x194 - 0x60];
    EVehicleCameraMode mVehicleCamMode;  // +0x194
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

// ea: 0x6F4930
void rb_vehicle::_set_default_pose_wheels_only()
{
    int* m_wheel_bone_indices = this->m_wheel_bone_indices;
    math::Dir3* p_z = &this->m_wheel_orig_relpo[0].z;
    for (int i = 8; i != 0; --i)
    {
        DObj* mDObj = m_owner->mDObj;
        if (mDObj->numBones >= *m_wheel_bone_indices
            && *m_wheel_bone_indices >= 0)
        {
            // DObj::GetMat(boneIndex) = wheel_orig_relpo[i]
            memcpy((char*)&mDObj->GetMat(*m_wheel_bone_indices),
                   p_z[-2].v.m128_f32, sizeof(math::Mat43));
        }
        ++m_wheel_bone_indices;
        p_z += 4;
    }
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
    WHEEL_STATE_ROLLING = 0,
    WHEEL_STATE_AIRBORN = 1,
    WHEEL_STATE_PEELINGOUT = 2,
    WHEEL_STATE_SKIDDING = 3,
};
// vehicle_info_t (g_vehiclefuncs.h view; type +0x20)
struct vehicle_info_t {
    char    name[32];  // +0x00
    short   type;  // +0x20
    short   subtype;  // +0x22
    uint8_t _pad24[0x21C - 0x24];
    char    vehiclePhysicsParms[32];  // +0x21C
    char    vehiclePhysicsParmsThird[32];  // +0x23C
};
vehicle_info_t* VEH_GetInfo(int idx);  // ?VEH_GetInfo@@YAPAUvehicle_info_t@@H@Z (g.o)
// ?g_vehicle_gravity_multiplier@@3MA (physics.o data @ 0xE01EE0)
float g_vehicle_gravity_multiplier = 1.5f;  // 0x3FC00000
// ?hand_brake_friction_time@@3MA (physics.o data @ 0xE36AC4)
float hand_brake_friction_time = 0.3f;      // 0x3E99999A

// scr_vehicle_t (g_local.h view). Layout verified against IDA (ordinal 4887):
// pathPos +0x00 (vehicle_pathpos_t, 0xB8), phys +0xC0 (vehicle_physic_t,
// 0xB0), infoIdx +0x178, seats +0x1E0 (11*28), current +0x3E0 / next +0x420
// (LerpedVariables, 0x40 each), boneIndex +0x460. mRBVeh is a legacy
// local-view field (not present in the binary scr_vehicle_t).
struct vehicle_node_view {      // vehicle_node_t (64 bytes)
    void* mName;                // +0x00 (Broc::string)
    void* mTarget;              // +0x04
    float speed;                // +0x08
    float lookAhead;            // +0x0C
    void* script_noteworthy;    // +0x10 (Broc::string)
    float origin[3];            // +0x14
    float dir[3];               // +0x20
    float angles[3];            // +0x2C
    float length;               // +0x38
    uint8_t packed[4];          // +0x3C (nextIdx/prevIdx/rotated bitfields)
};
struct vehicle_pathpos_view {   // vehicle_pathpos_t (184 bytes)
    int16_t nodeIdx;            // +0x00
    int16_t endOfPath;          // +0x02
    float   frac;               // +0x04
    float   speed;              // +0x08
    float   lookAhead;          // +0x0C
    float   slide;              // +0x10
    float   origin[3];          // +0x14
    float   angles[3];          // +0x20
    float   lookPos[3];         // +0x2C
    vehicle_node_view switchNode[2];  // +0x38
};
struct vehicle_physic_view {    // vehicle_physic_t (176 bytes)
    math::Position3 origin;     // +0x00
    math::Position3 prevOrigin; // +0x10
    math::Position3 angles;     // +0x20
    math::Position3 prevAngles; // +0x30
    math::Dir3      vel;        // +0x40
    math::Dir3      rotVel;     // +0x50
    float wheelZVel[6];         // +0x60
    float wheelZPos[6];         // +0x78
    int   wheelSurfType[6];     // +0x90
};
struct scr_vehicle_t {
    vehicle_pathpos_view pathPos;  // +0x00
    vehicle_physic_view  phys;     // +0xC0
    void*   mEntity;               // +0x170
    void*   mPhysicsOwner;         // +0x174
    int16_t infoIdx;               // +0x178
    int16_t waitNode;              // +0x17A
    float   waitSpeed;             // +0x17C
    uint8_t _pad180[0x1E0 - 0x180];
    struct {                        // 28-byte seat (IDA ordinal 4882)
        int          flags;         // +0x00
        unsigned int occupant;      // +0x04 (DbLinkedHandle mVal)
        int          boneIndex;     // +0x08
        int          weapon;        // +0x0C
        float        heat;          // +0x10
        unsigned int overheatEffect;  // +0x14 (Handle)
        uint8_t      gunMounted;    // +0x18
        uint8_t      overheating;   // +0x19
        uint8_t      firing;        // +0x1A
    } seats[11];                    // +0x1E0
    uint8_t _pad314[0x3C8 - 0x314];
    int     playersAttached;        // +0x3C8
    uint8_t _pad3CC[0x3E0 - 0x3CC];
    struct {  // scr_vehicle_t::LerpedVariables (64 bytes)
        math::Position3 mBodyPosition;  // +0x00
        math::Position3 mTurretAngles;  // +0x10
        math::Position3 mGunnerAngles;  // +0x20
        float mSteeringAngle;           // +0x30
        float mHatchAngleRight;         // +0x34
        float mHatchAngleLeft;          // +0x38
    } current;                          // +0x3E0
    struct {
        math::Position3 mBodyPosition;
        math::Position3 mTurretAngles;
        math::Position3 mGunnerAngles;
        float mSteeringAngle;
        float mHatchAngleRight;
        float mHatchAngleLeft;
    } next;                             // +0x420
    uint8_t boneIndex[120];             // +0x460
    uint8_t _pad4D8[0x518 - 0x4D8];
    void*   mRBVeh;                     // +0x518 (legacy local-view field)

    // ?CollisionDamage@scr_vehicle_t@@QAEXPAVEntity@@ABVPosition3@math@@1M@Z
    // (g.o; g_scr_vehicle.cpp defines the pointer-arg variant)
    void CollisionDamage(Entity* ent, const math::Position3* pos,
                         const math::Position3* dir, float intensity);
    void AssignPhysics(Entity* player);  // ?AssignPhysics@scr_vehicle_t@@QAEXPAVEntity@@@Z (g.o)
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

// ea: 0x6F5B50
void RBVehicleController::UpdateJump(rb_vehicle& rbveh)
{
    (void)rbveh;
}

// ea: 0x6FDD20
void RBVehicleController::UpdateControlsDefault(rb_vehicle& rbveh,
                                                int controller_port)
{
    rb_vehicle* v3 = &rbveh;
    int v4 = controller_port;
    rbveh.m_coasting_factor = rbveh.m_parameter->m_tire_damp_coast;
    int stickY, stickX;
    PadAliasMgr::sInst->mCtx[0].GetStickValue(
        v4, kPadAliasStickVehicleSteering, stickX, stickY);
    stickX = RecalibrateInput(stickX);
    float in = -(stickX * 0.0078125f);
    v3->m_steer_factor = ClampRange(in, -1.0f, 1.0f);
    int ButtonValue = PadAliasMgr::sInst->mCtx[0].GetButtonValue(
        v4, kPadAliasButtonGas);
    if (ButtonValue <= 150)
    {
        if (ButtonValue == 0)
        {
            if (PadAliasMgr::sInst->mCtx[0].GetButtonValue(
                    v4, kPadAliasButtonReverse) != 0)
                v3->set_throttle(-1.0f);
            else
                v3->set_throttle(0.0f);
            goto LABEL_8;
        }
    }
    else
    {
        ButtonValue = 255;
    }
    v3->set_throttle(ButtonValue * 0.0039215689f);
LABEL_8:
    if (PadAliasMgr::sInst->mCtx[0].GetButtonValue(v4,
                                                   kPadAliasButtonHandBrake)
        <= g_vehicle_button_threshold)
        v3->set_hand_brake(0.0f);
    else
        v3->set_hand_brake(1.0f);
    v3->set_brake(0.0f);
}

// ea: 0x702580
void RBVehicleController::UpdateControlsStrafeMode(rb_vehicle& rbveh,
                                                   int client_controller)
{
    float input_yaw = 0.0f;
    int valid = LocalClient::PortToValidClient(client_controller);
    Entity* Player = EntityManager::sInst->GetPlayer(valid);
    Entity* v6 = Player;
    if (Player != nullptr)
        input_yaw = ((float*)((char*)Player->client + 0xD0))[1];
    int move, stickY;
    PadAliasMgr::sInst->mCtx[0].GetStickValue(
        client_controller, kPadAliasStickVehicleSteering, move, stickY);
    int v7;
    if (v6 != nullptr && v6->IsLocalPlayer()
        && gCamera[v6->GetPlayerIndex()].mVehicleCamMode == VEH_MODE_HLO)
    {
        v7 = 0;
        move = 0;
    }
    else
    {
        v7 = move;
    }
    float vec;
    float v8 = fabs((float)stickY);
    if (vehicleDeadZone_0 <= v8)
    {
        int v10 = stickY <= 0 ? ((stickY >= 0) - 1) : 1;
        vec = ((v8 - vehicleDeadZone_0) / (128.0f - vehicleDeadZone_0)) * v10;
    }
    else
    {
        vec = 0.0f;
    }
    float v11 = fabs((float)move);
    float v17;
    if (vehicleDeadZone_0 <= v11)
    {
        int v12 = v7 <= 0 ? ((v7 >= 0) - 1) : 1;
        v17 = ((v11 - vehicleDeadZone_0) / (128.0f - vehicleDeadZone_0)) * v12;
    }
    else
    {
        v17 = 0.0f;
    }
    float input = sqrtf(v17 * v17 + vec * vec);
    if (input > 1.0f)
        input = 1.0f;
    input_yaw = vectoyaw(&vec) + input_yaw + 180.0f;
    UpdateVehicleInputs(rbveh, input_yaw, input, -vec);
}

// ea: 0x702730
void RBVehicleController::UpdateControlsTankStrafeMode(rb_vehicle& rbveh,
                                                       int controller_port)
{
    UpdateControlsStrafeMode(rbveh, controller_port);
}

// ea: 0x6FDE40
void RBVehicleController::UpdateControlsTank(rb_vehicle& rbveh,
                                             int controller_port)
{
    rbveh.m_coasting_factor = rbveh.m_parameter->m_tire_damp_coast;
    int y, x;
    PadAliasMgr::sInst->mCtx[1].GetStickValue(
        controller_port, kPadAliasStickVehicleSteering, x, y);
    x = RecalibrateInput(x);
    int v5 = RecalibrateInput(y);
    y = v5;
    float target_yaw = 0.0f;
    float input_dist = sqrtf((float)(y * y + x * x));
    float v6 = 0.0f;
    int input_dista;
    if (input_dist == 0.0f)
    {
        input_dista = 0;
    }
    else
    {
        v6 = x / input_dist;
        target_yaw = v6;
        if (v6 > 0.0f)
            input_dista = 1;
        else
        {
            input_dista = -1;
            if (v6 >= 0.0f)
                input_dista = 0;
        }
    }
    float v7 = -(target_yaw * target_yaw * input_dista);
    float v8;
    if (fabs(v7) >= 0.98000002f)
        v8 = v7;
    else
        v8 = steer_reduce_frac * v7;
    float v9 = ((v5 * 0.007751938f) * (v5 * 0.007751938f))
        + ((((x * 0.25f) * v6) * 0.007751938f)
           * (((x * 0.25f) * v6) * 0.007751938f));
    float v10;
    if (v9 == 0.0f)
    {
        v10 = 0.0f;
    }
    else
    {
        v10 = v9 / sqrtf(v9);
    }
    if (v5 > 0)
    {
        v10 = v10 * -1.0f;
    }
    if (rbveh.m_forward_vel < 0.0f && v10 >= 0.0f)
        v8 = 0.0f;
    rbveh.m_steer_factor = ClampRange(v8, -1.0f, 1.0f);
    rbveh.set_throttle(v10);
    rbveh.set_hand_brake(0.0f);
    rbveh.set_brake(0.0f);
    rb_extra_info* m_chassis_rbinf = rbveh.m_chassis_rbinf;
    if (m_chassis_rbinf != nullptr
        && PadAliasMgr::sInst->mCtx[1].GetButtonValue(
               controller_port, kPadAliasButtonAlignTurret)
               > g_vehicle_button_threshold)
    {
        const math::Mat43* mat = &m_chassis_rbinf->m_rb->get_mat();
        float rb_angles[3];
        Axis4ToAngles((const float(*)[4])mat, rb_angles);
        AngleNormalize180Accurate(
            gCamera[controller_port].mPrevAngles.v.m128_f32[1]
            - m_chassis_rbinf->m_ent->r.currentAngles.v.m128_f32[1]);
        float v15 = rbveh.m_steer_factor - rb_angles[1];
        AngleNormalize180Accurate(v15);
        float v14 = 1.0f;
        float input_distb = 1.0f;
        if (rbveh.m_throttle > 0.94999999f)
        {
            v14 = 0.15000001f;
            input_distb = 0.15000001f;
        }
        float target_yawb = rbveh.m_steer_factor;
        if (target_yawb > thresh)
        {
            rbveh.set_steer_factor(input_distb);
            rbveh.set_throttle(1.0f);
            return;
        }
        if ((0.0f - thresh) > target_yawb)
        {
            rbveh.set_steer_factor(0.0f - v14);
            rbveh.set_throttle(1.0f);
            return;
        }
        rbveh.set_steer_factor(0.0f);
    }
}

// ea: 0x6FDA20
void RBVehicleController::UpdateVehicleInputs(rb_vehicle& rbveh,
                                              float target_yaw,
                                              float target_accel,
                                              float forward_preference)
{
    float v6 = 0.0f;
    float delta_yaw = 0.0f;
    if (target_accel != 0.0f && (rbveh.m_flags.mMask & 1) == 0)
    {
        const math::Mat43& mat = rbveh.m_chassis_rbinf->m_rb->get_mat();
        float ang[3];
        Axis4ToAngles((const float(*)[4])&mat, ang);
        float v8 = target_yaw - ang[1];
        delta_yaw = AngleNormalize180Accurate(v8);
        v6 = delta_yaw;
    }
    if ((rbveh.m_flags.mMask & 0x20) == 0)
        rbveh.m_coasting_factor = rbveh.m_parameter->m_tire_damp_coast * 3.0f;
    int v9 = 0;
    char v28 = 0;
    float v10;
    if (fabs(delta_yaw) <= forward_preference * 45.0f + 135.0f)
    {
        v10 = 1.0f;
    }
    else
    {
        if (v6 <= 0.0f)
        {
            if (v6 < 0.0f)
                v9 = -1;
        }
        else
        {
            v9 = 1;
        }
        delta_yaw = v6 - (v9 * 180.0f);
        v10 = -1.0f;
        v28 = 1;
    }
    rbveh.set_throttle(v10 * target_accel);
    float m_forward_vel = rbveh.m_forward_vel;
    math::Dir3 velocity;
    float vel_len_sq = 0.0f;
    if (target_accel == 0.0f
        || (m_forward_vel <= 5.0f || v28 == 0)
               && (m_forward_vel >= -5.0f || v28 != 0))
    {
        rbveh.set_brake(0.0f);
    }
    else
    {
        velocity = rbveh.get_velocity();
        __m128 v13 = _mm_mul_ps(velocity.v, velocity.v);
        vel_len_sq =
            v13.m128_f32[0] + (v13.m128_f32[1] + v13.m128_f32[2]);
        float v14 = 0.0f;
        if (vel_len_sq <= (steer_velocity * steer_velocity))
            v14 = delta_yaw;
        delta_yaw = v14 * -1.0f;
        rbveh.set_brake(1.0f);
    }
    float v15 = 0.0f;
    if (delta_yaw <= 0.0f)
    {
        if (delta_yaw < 0.0f)
            v15 = -1.0f;
    }
    else
    {
        v15 = 1.0f;
    }
    float v16 = v28 == 0 ? 1.0f : -1.0f;
    bool v17 = (rbveh.m_flags.mMask & 0x20) == 0;
    float v18 = v16 * v15;
    float v19 = v17 ? 30.0f : 100.0f;
    delta_yaw = fabs(delta_yaw);
    if (v19 > delta_yaw)
        v18 = (delta_yaw / v19) * v18;
    rbveh.m_steer_factor = ClampRange(v18, -1.0f, 1.0f);
    if (delta_yaw <= 60.0f)
    {
        rbveh.set_hand_brake(0.0f);
    }
    else
    {
        velocity = rbveh.get_velocity();
        __m128 v22 = _mm_mul_ps(velocity.v, velocity.v);
        float v24 =
            v22.m128_f32[0] + (v22.m128_f32[1] + v22.m128_f32[2]);
        if (v24 <= (hand_brake_min_speed * hand_brake_min_speed))
            rbveh.set_hand_brake(0.0f);
        else
            rbveh.set_hand_brake(1.0f);
    }
}

// ea: 0x702060
void RBVehicleController::UpdateScriptVehicleControl(rb_vehicle& rbveh)
{
    Entity* m_owner = rbveh.m_owner;
    math::Position3 currentOrigin = m_owner->r.currentOrigin;
    __m128 v6 = _mm_sub_ps(m_script_goal_position.v, currentOrigin.v);
    math::Dir3 to_goal;
    to_goal.v = _mm_shuffle_ps(
        v6, _mm_shuffle_ps(_mm_setzero_ps(), v6, 240), 196);
    __m128 v8 = _mm_mul_ps(to_goal.v, to_goal.v);
    float goal_speed =
        sqrtf(v8.m128_f32[0] + (v8.m128_f32[1] + v8.m128_f32[2]));
    float yaw_deg;
    if (goal_speed <= 0.0f)
    {
        yaw_deg = 0.0f;
    }
    else
    {
        to_goal.v = _mm_div_ps(to_goal.v, _mm_set1_ps(goal_speed));
        float v26 = fabs(to_goal.v.m128_f32[0]);
        float owner = fabs(to_goal.v.m128_f32[1]);
        float v10;
        if (0.0f == owner + v26)
        {
            v10 = 0.0f;
        }
        else
        {
            float v25 = 1.0f
                / sqrtf(to_goal.v.m128_f32[1] * to_goal.v.m128_f32[1]
                        + to_goal.v.m128_f32[0] * to_goal.v.m128_f32[0]);
            if (owner <= v26)
            {
                float v13 = v25 * owner;
                if ((v25 * owner) >= 0.5f)
                {
                    float ownerd = sqrtf(fabs((1.0f - v13) * 0.5f));
                    v10 = ((((((((ownerd * ownerd) * ownerd)
                                * (ownerd * ownerd))
                               * (ownerd * ownerd))
                              * -0.1079625f)
                             - ((((ownerd * ownerd) * ownerd)
                                 * (ownerd * ownerd))
                                * 0.15000001f))
                            - (((ownerd * ownerd) * ownerd)
                               * 0.33333331f))
                           - (ownerd * 2.0f))
                        + 1.570796f;
                }
                else
                {
                    v10 = (((((((v13 * v13) * v13) * (v13 * v13))
                              * (v13 * v13))
                             * 0.053981241f)
                            + ((((v13 * v13) * v13) * (v13 * v13))
                               * 0.075000003f))
                           + (((v13 * v13) * v13) * 0.1666667f))
                        + v13;
                }
            }
            else
            {
                float v11 = v25 * v26;
                float v12;
                if ((v25 * v26) >= 0.5f)
                {
                    float ownerc = sqrtf(fabs((1.0f - v11) * 0.5f));
                    v12 = ((((((((ownerc * ownerc) * ownerc)
                                * (ownerc * ownerc))
                               * (ownerc * ownerc))
                              * -0.1079625f)
                             - ((((ownerc * ownerc) * ownerc)
                                 * (ownerc * ownerc))
                                * 0.15000001f))
                            - (((ownerc * ownerc) * ownerc)
                               * 0.33333331f))
                           - (ownerc * 2.0f))
                        + 1.570796f;
                }
                else
                {
                    v12 = (((((((v11 * v11) * v11) * (v11 * v11))
                              * (v11 * v11))
                             * 0.053981241f)
                            + ((((v11 * v11) * v11) * (v11 * v11))
                               * 0.075000003f))
                           + (((v11 * v11) * v11) * 0.1666667f))
                        + v11;
                }
                v10 = 1.5707964f - v12;
            }
            if (to_goal.v.m128_f32[0] < 0.0f)
                v10 = 3.1415927f - v10;
            if (to_goal.v.m128_f32[1] < 0.0f)
                v10 = 0.0f - v10;
        }
        float v14 = v10 * 57.295776f;
        yaw_deg = v14;
        if (v14 < 0.0f)
            yaw_deg = v14 + 360.0f;
    }
    __m128 v16 =
        _mm_mul_ps(m_owner->r.currentMat.y.v, to_goal.v);
    float v17 = fabs(v16.m128_f32[0]
                     + (v16.m128_f32[1] + v16.m128_f32[2]));
    __m128 v18 = _mm_sub_ps(currentOrigin.v, m_stuck_position.v);
    __m128 v19 = _mm_mul_ps(v18, v18);
    if ((v19.m128_f32[0] + (v19.m128_f32[1] + v19.m128_f32[2])) < 25.0f
        || m_stuck_time < 0.0f)
    {
        m_stuck_time = m_stuck_time + ServerTime::sInst.mTickDelta;
    }
    else
    {
        m_stuck_position.v = m_owner->r.currentOrigin.v;
        m_stuck_time = 0.0f;
    }
    if (m_stuck_time > 3.0f)
        m_stuck_time = -0.75f;
    float v27;
    if (((rbveh.m_flags.mMask & 0x20) != 0 || (v27 = v17) <= 0.80000001f
         || goal_speed >= 400.0f)
        && m_stuck_time >= 0.0f)
    {
        UpdateVehicleInputs(rbveh, yaw_deg, 1.0f, 0.0f);
    }
    else
    {
        UpdateVehicleInputs(rbveh, yaw_deg + 180.0f, 1.0f, -1.6f);
    }
    vehicle_rb_parameter* m_parameter = rbveh.m_parameter;
    float v21 = m_parameter->m_speed_max * m_script_goal_speed;
    float v22 = v21 + 10.0f;
    float ownerb = fabs(rbveh.m_forward_vel);
    if ((rbveh.m_flags.mMask & 0x200) != 0)
    {
        if (ownerb <= v21)
            return;
        goto LABEL_34;
    }
    if (m_script_goal_radius <= goal_speed)
    {
        bool v23;
        if (goal_speed >= 1000.0f)
        {
            if (ownerb <= v22)
                return;
            v23 = ownerb <= 300.0f;
        }
        else
        {
            if (ownerb <= v22)
                return;
            if (ownerb > ((m_parameter->m_speed_max - v22) * (goal_speed * 0.001f)) + v22)
            {
            LABEL_34:
                rbveh.set_throttle(0.0f);
            LABEL_35:
                rbveh.set_hand_brake(1.0f);
                return;
            }
            v23 = ownerb <= 300.0f;
        }
        if (!v23)
            rbveh.set_throttle(0.0f);
    }
    else
    {
        if (m_script_goal_speed == 0.0f)
        {
            rbveh.set_throttle(0.0f);
            v22 = v21 + 10.0f;
        }
        if (v22 < ownerb)
            goto LABEL_35;
        m_owner->Notify(hash_const.goal);
        rbveh.m_flags.mMask &= ~8u;
    }
}

// ea: 0x70C530
void RBVehicleController::UpdateControls(rb_vehicle& rbveh)
{
    scr_vehicle_t* scr_vehicle = (scr_vehicle_t*)rbveh.m_owner->scr_vehicle;
    unsigned int mVal = scr_vehicle->seats[0].occupant;
    unsigned int v5 = mVal & 0xFFF;
    Entity* occupant = nullptr;
    if (v5 < 0x540 && mVal >> 12 == EntityHandleDb::sInst.mElements[v5].mKey)
        occupant = EntityHandleDb::sInst.mElements[v5].mObject;
    int PlayerIndex = EntityManager::sInst->GetPlayerIndex(occupant);
    if (PlayerIndex >= 0)
    {
        if (!EntityManager::sInst->IsLocalPlayer(occupant))
            return;
        m_hold_controls_time = 1.0f;
        if (GamePause::IsGamePaused(PlayerIndex))
            return;
        unsigned int mMask = rbveh.m_flags.mMask;
        if ((mMask & 0x100) != 0)
        {
            if ((mMask & 1) != 0)
            {
                rbveh.start_path(0);
                return;
            }
            scr_vehicle = (scr_vehicle_t*)rbveh.m_owner->scr_vehicle;
            rbveh.set_steer_factor(
                ((scr_vehicle->next.mSteeringAngle * 3.1415927f)
                 * 0.0055555557f)
                * tweaker);
            goto LABEL_38;
        }
        if ((mMask & 0x200) == 0 || (rbveh.m_flags.mMask & 1) != 0)
        {
            if ((mMask & 8) == 0)
            {
                if (!IsLocalPlayer(occupant)
                    || !IsPlayerFullySeatedInVehicle(occupant))
                {
                    if (occupant != nullptr)
                        return;
                    rbveh.set_steer_factor(0.0f);
                    goto LABEL_38;
                }
                if ((rbveh.m_flags.mMask & 0x20) != 0)
                {
                    if (((char*)&dword_F6A28C[802 * PlayerIndex])[8] != 0)
                        UpdateControlsTank(rbveh,
                                           dword_F6A28C[802 * PlayerIndex]);
                    else
                        UpdateControlsStrafeMode(
                            rbveh, dword_F6A28C[802 * PlayerIndex]);
                    vehicle_rb_parameter* m_parameter = rbveh.m_parameter;
                    if ((rbveh.m_forward_vel > m_parameter->m_speed_max
                         && rbveh.m_throttle > 0.0f)
                        || ((0.0f - m_parameter->m_speed_max)
                                > rbveh.m_forward_vel
                            && rbveh.m_throttle < 0.0f))
                    {
                        rbveh.set_throttle(0.0f);
                    }
                }
                else if (occupant->IsLocalPlayer()
                         && (gCamera[occupant->GetPlayerIndex()]
                                     .mVehicleCamMode
                                 == VEH_MODE_HLO
                             || gCamera[occupant->GetPlayerIndex()]
                                        .mVehicleCamMode
                                    == VEH_MODE_STRAFE))
                {
                    UpdateControlsStrafeMode(
                        rbveh, dword_F6A28C[802 * PlayerIndex]);
                }
                else
                {
                    UpdateControlsDefault(rbveh,
                                          dword_F6A28C[802 * PlayerIndex]);
                }
                return;
            }
        }
        else
        {
            scr_vehicle = (scr_vehicle_t*)rbveh.m_owner->scr_vehicle;
            math::Position3 v11;
            v11.v = _mm_setr_ps(scr_vehicle->pathPos.lookPos[0],
                                scr_vehicle->pathPos.lookPos[1],
                                scr_vehicle->pathPos.lookPos[2], 0.0f);
            m_script_goal_position.v = v11.v;
            m_script_goal_radius = 5.0f;
            m_script_goal_speed =
                scr_vehicle->pathPos.speed / rbveh.m_parameter->m_speed_max;
        }
        UpdateScriptVehicleControl(rbveh);
        return;
    }
    if (m_hold_controls_time <= 0.0f)
    {
        float zero = 0.0f;
        rbveh.m_steer_factor = ClampRange(zero, -1.0f, 1.0f);
    LABEL_38:
        rbveh.set_throttle(0.0f);
        rbveh.set_hand_brake(0.0f);
        return;
    }
    float v6 = m_hold_controls_time - ServerTime::sInst.mTickDelta;
    m_hold_controls_time = v6;
    if (v6 < 0.0f)
        m_hold_controls_time = 0.0f;
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

// ?wheelNames@@3PAPADA (physics.o data @ 0xE01EE4)
char* wheelNames[8] = {
    "tag_wheel_front_left",   "tag_wheel_front_right",
    "tag_wheel_back_left",    "tag_wheel_back_right",
    "tag_wheel_middle_left",  "tag_wheel_middle_right",
    "tag_wheel_middle_back_left", "tag_wheel_middle_back_right",
};

// ea: 0x6F4830
void rb_vehicle::_align_wheels()
{
    for (int i = 0; i < 8; ++i)
        m_wheel_bone_indices[i] = m_owner->mDObj->GetBoneIndex(wheelNames[i]);
    for (int i = 0; i < 8; ++i)
    {
        if (m_wheel_bone_indices[i] >= 0)
            m_wheel_orig_relpo[i] =
                m_owner->mDObj->GetMat(m_wheel_bone_indices[i]);
    }
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

// ea: 0x6F4B00
void rb_vehicle::_update_fakey_stuff(float delta_t)
{
    float v2 = 0.0f;
    rigid_body* m_rb = m_chassis_rbinf->m_rb;
    m_num_colliding_wheels = 0;
    if (m_throttle > 0.5f)
        v2 = m_parameter->m_tilt_fakey * m_parameter->m_susp_spring_k;
    else if (m_throttle < -0.5f)
        v2 = 0.0f - (m_parameter->m_tilt_fakey
                     * m_parameter->m_susp_spring_k);

    rigid_body_constraint_wheel* v6 =
        (rigid_body_constraint_wheel*)m_wheels[1];
    if (v6 != nullptr)
    {
        float spring_k = _calc_initial_susp_spring_k(
            (rigid_body_constraint_wheel*)m_wheels[1]);
        v6->m_suspension_stiffness_k = spring_k + v2;
        if ((v6->m_wheel_flags & 1) != 0)
            ++m_num_colliding_wheels;
        rigid_body_constraint_wheel* v8 =
            (rigid_body_constraint_wheel*)m_wheels[0];
        v8->m_suspension_stiffness_k = spring_k + v2;
        if ((v8->m_wheel_flags & 1) != 0)
            ++m_num_colliding_wheels;
    }
    rigid_body_constraint_wheel* v9 =
        (rigid_body_constraint_wheel*)m_wheels[3];
    if (v9 != nullptr)
    {
        float spring_ka = _calc_initial_susp_spring_k(v9);
        if ((v9->m_wheel_flags & 1) != 0)
            ++m_num_colliding_wheels;
        v9->m_suspension_stiffness_k = spring_ka - v2;
        rigid_body_constraint_wheel* v10 =
            (rigid_body_constraint_wheel*)m_wheels[2];
        v10->m_suspension_stiffness_k = spring_ka - v2;
        if ((v10->m_wheel_flags & 1) != 0)
            ++m_num_colliding_wheels;
    }

    if (m_orientation_constraint == nullptr)
    {
        environment_rigid_body* environment_rigid_body =
            phys_sys::get_environment_rigid_body();
        rigid_body_constraint_custom_orientation* rbc_custom_orientation =
            phys_sys::create_rbc_custom_orientation(m_rb,
                                                    environment_rigid_body,
                                                    false);
        m_parameter = this->m_parameter;
        m_orientation_constraint = rbc_custom_orientation;
        rbc_custom_orientation->m_torque_resistance =
            m_parameter->m_roll_resistance * m_parameter->m_body_mass;
        m_orientation_constraint->m_upright_strength =
            m_parameter->m_upright_strength * m_parameter->m_body_mass;
    }

    bool v14 = m_throttle > 0.89999998f
               && m_parameter->m_peel_out_max_speed > m_forward_vel;
    bool v17 = false;
    for (int i = 0; i < 8; ++i)
    {
        rigid_body_constraint_wheel* w =
            (rigid_body_constraint_wheel*)m_wheels[i];
        if (w != nullptr && w->m_wheel_state == 0
            && (v14
                || (m_throttle > 0.69999999f
                    && (w->m_wheel_flags & 1) == 0)))
        {
            v17 = true;
            w->m_wheel_pos = (delta_t * 40.0f) + w->m_wheel_pos;
        }
    }

    float v25 = 0.80000001f;
    float rpm_target =
        fabs(m_forward_vel / m_parameter->m_speed_max) * fabs(m_throttle);
    if (!v17)
        v25 = rpm_target;
    m_fake_rpm = (((v25 - m_fake_rpm) * delta_t) * 5.0f) + m_fake_rpm;
}

// ea: 0x6FC490
void rb_vehicle::_update_orientation_constraint()
{
    if (m_orientation_constraint == nullptr
        && _tlAssert("c:\\cod\\code\\game\\RBVehicle.cpp", 760,
                     "m_orientation_constraint", defaultFileName))
        __debugbreak();
    if (m_chassis_rbinf == nullptr
        && _tlAssert("c:\\cod\\code\\game\\RBVehicle.cpp", 761,
                     "m_chassis_rbinf", defaultFileName))
        __debugbreak();
    if (m_chassis_rbinf->m_rb == nullptr
        && _tlAssert("c:\\cod\\code\\game\\RBVehicle.cpp", 762,
                     "m_chassis_rbinf->m_rb", defaultFileName))
        __debugbreak();
    unsigned int mMask = m_flags.mMask;
    bool v3 =
        ((mMask & 8) != 0 || (mMask & 0x80) != 0 || (mMask & 0x100) != 0
         || (mMask & 0x200) != 0)
        && m_owner->health > 0 && (mMask & 0x400) == 0;
    if (m_num_colliding_wheels != 0
        || m_chassis_rbinf->m_rb->m_contact_count != 0)
    {
        m_orientation_constraint->m_active = v3;
        m_orientation_constraint->m_no_orientation_correction = false;
    }
    else
    {
        m_orientation_constraint->m_active = v3;
        m_orientation_constraint->m_no_orientation_correction = true;
    }
}

// ?gWheelAxisLoc@@3VDir3@math@@A (physics.o data @ 0xF8D380; runtime-filled,
// binary bytes are 0xFFFFFFFF = NaN)
math::Dir3 gWheelAxisLoc = { _mm_castsi128_ps(_mm_set1_epi32(-1)) };

// ea: 0x6F50A0
void rb_vehicle::update_steering(float delta_t)
{
    float m_steer_factor = this->m_steer_factor;
    if (m_steer_factor > 1.0f)
        m_steer_factor = 1.0f;
    else if (m_steer_factor < -1.0f)
        m_steer_factor = -1.0f;
    float v4 = m_steer_speed * delta_t;
    float v5 = (m_steer_max_angle * m_steer_factor) - m_steer_current_angle;
    if (v5 <= v4)
    {
        if ((0.0f - v4) > v5)
            v5 = 0.0f - v4;
    }
    else
    {
        v5 = m_steer_speed * delta_t;
    }
    m_steer_current_angle += v5;
    if (fabs(m_steer_current_angle) <= 0.0049999999f)
    {
        for (int i = 0; i < 8; ++i)
        {
            rigid_body_constraint_wheel* w =
                (rigid_body_constraint_wheel*)m_wheels[i];
            if (w != nullptr)
            {
                w->m_b1_wheel_axis_loc.v = gWheelAxisLoc.v;
                w->m_turning_radius_ratio_max_speed = 1.0f;
                w->m_turning_radius_ratio_accel = 1.0f;
            }
        }
    }
    else
    {
        bool v17 = true;
        if (m_wheels[0] != nullptr)
            v17 = (((rigid_body_constraint_wheel*)m_wheels[0])->m_wheel_flags
                   & 8)
                  != 0;
        for (int i = 1; i < 7; ++i)
        {
            if (m_wheels[i] != nullptr
                && (((rigid_body_constraint_wheel*)m_wheels[i])->m_wheel_flags
                    & 8)
                       == 0)
                v17 = false;
        }
        float v25, v26;
        if ((m_wheels[7] == nullptr
             || (((rigid_body_constraint_wheel*)m_wheels[7])->m_wheel_flags
                 & 8)
                    != 0)
            && v17)
        {
            v25 = m_steer_front_back_length * -0.5f;
            v26 = m_steer_front_back_length / tanf(m_steer_current_angle)
                  * 0.5f;
        }
        else
        {
            v25 = 0.0f - m_steer_front_back_length;
            v26 = m_steer_front_back_length / tanf(m_steer_current_angle);
        }
        __m128 offset = _mm_setr_ps(v25, v26, 0.0f, 0.0f);
        float nwheel_axis = sqrtf(v25 * v25 + v26 * v26);
        __m128 v31 = _mm_add_ps(m_steer_front_pt_loc.v, offset);
        float wheel_ratios[8] = {};
        int num_accel_wheels = 0;
        float accel_sum = 0.0f;
        for (int v72 = 0; v72 < 8; ++v72)
        {
            rigid_body_constraint_wheel* w =
                (rigid_body_constraint_wheel*)m_wheels[v72];
            if (w != nullptr)
            {
                __m128 diff = _mm_sub_ps(v31, w->m_b1_wheel_center_loc.v);
                __m128 d2 = _mm_mul_ps(diff, diff);
                float dist = sqrtf(d2.m128_f32[0]
                                   + (d2.m128_f32[1] + d2.m128_f32[2]));
                if ((w->m_wheel_flags & 8) != 0)
                {
                    float sign =
                        m_steer_current_angle < 0.0f ? -1.0f : 1.0f;
                    if (dist <= 0.001f)
                    {
                        w->m_b1_wheel_axis_loc.v = gWheelAxisLoc.v;
                    }
                    else
                    {
                        w->m_b1_wheel_axis_loc.v =
                            _mm_mul_ps(diff, _mm_set1_ps(sign / dist));
                    }
                }
                float ratio = dist / nwheel_axis;
                if ((w->m_wheel_flags & 0x10) != 0)
                {
                    ++num_accel_wheels;
                    wheel_ratios[v72] = ratio;
                    accel_sum += ratio;
                }
            }
        }
        float scale = (float)num_accel_wheels / accel_sum;
        for (int i = 0; i < 8; ++i)
        {
            rigid_body_constraint_wheel* w =
                (rigid_body_constraint_wheel*)m_wheels[i];
            if (w != nullptr)
            {
                w->m_turning_radius_ratio_max_speed = wheel_ratios[i];
                w->m_turning_radius_ratio_accel = wheel_ratios[i] * scale;
            }
        }
    }
}

// ea: 0x705070
void rb_vehicle::_update_wheel_effects(float delta_t)
{
    (void)delta_t;
    if ((m_flags.mMask & 1) != 0)
        return;

    // Peel-out flag: throttle above 0.9 and forward vel above peel-out max.
    bool peeling = m_throttle > 0.89999998f
                   && m_forward_vel > m_parameter->m_peel_out_max_speed;
    vehicle_info_t* v76 =
        VEH_GetInfo(((scr_vehicle_t*)m_owner->scr_vehicle)->infoIdx);
    rb_extra_info* m_chassis_rbinf = this->m_chassis_rbinf;
    rigid_body* m_rb = m_chassis_rbinf->m_rb;

    // Water-filter pass (contents 32): ML (4) / MR (5) wheels.
    proximity_data_t water_objects;
    if (m_owner->proximity_data != nullptr
        && m_owner->proximity_data->lo.v.m128_f32[0]
               <= m_owner->proximity_data->hi.v.m128_f32[0])
    {
        filter_proximity_data(
            m_owner->proximity_data->lo, m_owner->proximity_data->hi, 32,
            *m_owner->proximity_data, water_objects);
        if ((m_flags.mMask & 0x20) != 0)
        {
            rigid_body_constraint_wheel* v9 =
                (rigid_body_constraint_wheel*)m_wheels[4];
            math::Position3 hitp;
            hitp.v = v9->m_b2_hitp_loc.v;
            math::Dir3 hitn;
            hitn.v = v9->m_b2_hitn_loc.v;
            bool airborne = (v9->m_wheel_flags & 1) == 0;
            UpdateWheelMarks(m_owner, 4, airborne, hitp, hitn);
            rigid_body_constraint_wheel* v13 =
                (rigid_body_constraint_wheel*)m_wheels[5];
            math::Position3 hitp2;
            hitp2.v = v13->m_b2_hitp_loc.v;
            math::Dir3 hitn2;
            hitn2.v = v13->m_b2_hitn_loc.v;
            bool airborne2 = (v13->m_wheel_flags & 1) == 0;
            UpdateWheelMarks(m_owner, 5, airborne2, hitp2, hitn2);
            j_nullsub_50(&water_objects);
            return;
        }
    }

    int wheel = 0;
    for (; wheel < 4; ++wheel)
    {
        rigid_body_constraint_wheel* v17 =
            (rigid_body_constraint_wheel*)m_wheels[wheel];
        if (v17 == nullptr
            || (v76->subtype == 2 && wheel == 1))
            break;

        unsigned int m_wheel_flags = v17->m_wheel_flags;
        wheel_effect_state_e oldState =
            (wheel_effect_state_e)m_wheel_effect_state[wheel];
        float v70 = 0.0f;
        if ((m_wheel_flags & 1) != 0)
        {
            if (peeling)
            {
                m_wheel_effect_state[wheel] = WHEEL_STATE_PEELINGOUT;
            }
            else if ((m_flags.mMask & 2) != 0
                     && fabs(m_owner->speed) > 100.0f)
            {
                m_wheel_effect_state[wheel] = WHEEL_STATE_SKIDDING;
                v70 = 600.0f;
            }
            else if ((m_wheel_flags & 4) != 0)
            {
                // Compute slip: body velocity + angular contribution, then
                // remove the suspension-normal component.
                const math::Mat43& mat = m_rb->dangerous_get_mat();
                __m128 v22 = _mm_shuffle_ps(mat.x.v, mat.y.v, 68);
                __m128 tv = _mm_add_ps(
                    _mm_add_ps(
                        _mm_mul_ps(_mm_shuffle_ps(m_rb->m_t_vel.v,
                                                  m_rb->m_t_vel.v, 0),
                                   _mm_shuffle_ps(v22, mat.z.v, 136)),
                        _mm_mul_ps(_mm_shuffle_ps(m_rb->m_t_vel.v,
                                                  m_rb->m_t_vel.v, 0x55),
                                   _mm_shuffle_ps(v22, mat.z.v, 221))),
                    _mm_mul_ps(
                        _mm_shuffle_ps(m_rb->m_t_vel.v, m_rb->m_t_vel.v,
                                       0xAA),
                        _mm_shuffle_ps(
                            _mm_shuffle_ps(mat.x.v, mat.y.v, 238), mat.z.v,
                            168)));
                const math::Mat43& mat2 = m_rb->dangerous_get_mat();
                __m128 v26 = _mm_shuffle_ps(mat2.x.v, mat2.y.v, 68);
                __m128 av = _mm_add_ps(
                    _mm_add_ps(
                        _mm_mul_ps(_mm_shuffle_ps(m_rb->m_a_vel.v,
                                                  m_rb->m_a_vel.v, 0),
                                   _mm_shuffle_ps(v26, mat2.z.v, 136)),
                        _mm_mul_ps(_mm_shuffle_ps(m_rb->m_a_vel.v,
                                                  m_rb->m_a_vel.v, 0x55),
                                   _mm_shuffle_ps(v26, mat2.z.v, 221))),
                    _mm_mul_ps(
                        _mm_shuffle_ps(m_rb->m_a_vel.v, m_rb->m_a_vel.v,
                                       0xAA),
                        _mm_shuffle_ps(
                            _mm_shuffle_ps(mat2.x.v, mat2.y.v, 238), mat2.z.v,
                            168)));
                math::Dir3 susp = v17->m_b1_suspension_dir_loc;
                __m128 wheel_pt = _mm_add_ps(
                    v17->m_b1_wheel_center_loc.v,
                    _mm_mul_ps(
                        susp.v,
                        _mm_set1_ps(v17->m_wheel_radius
                                    - v17->m_wheel_displaced_center_dist)));
                __m128 ang = _mm_sub_ps(
                    _mm_mul_ps(_mm_shuffle_ps(av, av, 9),
                               _mm_shuffle_ps(wheel_pt, wheel_pt, 18)),
                    _mm_mul_ps(_mm_shuffle_ps(av, av, 18),
                               _mm_shuffle_ps(wheel_pt, wheel_pt, 9)));
                math::Dir3 axis = v17->m_b1_wheel_axis_loc;
                __m128 surf = _mm_add_ps(
                    _mm_add_ps(tv, ang),
                    _mm_mul_ps(
                        _mm_sub_ps(
                            _mm_mul_ps(_mm_shuffle_ps(axis.v, axis.v, 9),
                                       _mm_shuffle_ps(susp.v, susp.v, 18)),
                            _mm_mul_ps(_mm_shuffle_ps(axis.v, axis.v, 18),
                                       _mm_shuffle_ps(susp.v, susp.v, 9))),
                        _mm_set1_ps(v17->m_wheel_vel * v17->m_wheel_radius)));
                __m128 dot = _mm_mul_ps(surf, susp.v);
                float proj =
                    dot.m128_f32[0]
                    + (_mm_shuffle_ps(dot, dot, 85).m128_f32[0]
                       + _mm_shuffle_ps(dot, dot, 170).m128_f32[0]);
                __m128 lat = _mm_sub_ps(
                    surf, _mm_mul_ps(susp.v, _mm_set1_ps(proj)));
                __m128 lat2 = _mm_mul_ps(lat, lat);
                v70 = lat2.m128_f32[0]
                      + (_mm_shuffle_ps(lat2, lat2, 85).m128_f32[0]
                         + _mm_shuffle_ps(lat2, lat2, 170).m128_f32[0]);
                if (v70 > 62500.0f)
                    m_wheel_effect_state[wheel] = WHEEL_STATE_SKIDDING;
                else
                    m_wheel_effect_state[wheel] = WHEEL_STATE_ROLLING;
            }
            else
            {
                m_wheel_effect_state[wheel] = WHEEL_STATE_ROLLING;
            }
        }
        else
        {
            m_wheel_effect_state[wheel] = WHEEL_STATE_AIRBORN;
        }

        if (m_wheel_effect_state[wheel] == oldState)
            continue;

        if (oldState == WHEEL_STATE_PEELINGOUT
            || oldState == WHEEL_STATE_SKIDDING)
        {
            // StopEffect (m_wheel_effects[wheel] -> 0)
            m_wheel_effects[wheel].mVal = 0;
        }
        if (m_wheel_effect_state[wheel] == WHEEL_STATE_PEELINGOUT)
            break;  // stop processing wheels while peeling
        if (m_wheel_effect_state[wheel] == WHEEL_STATE_SKIDDING)
        {
            Handle v66;
            v66 = PostEffectEventVehicleWheel(
                m_owner, v76->name, 0x2B,
                ((scr_vehicle_t*)m_owner->scr_vehicle)
                    ->phys.wheelSurfType[wheel],
                s_wheelTagHashes[wheel]);
            m_wheel_effects[wheel] = v66;
            float vol = (v70 - 150.0f) * 0.0000049382716f;
            if (vol < 0.0f)
                vol = 0.0f;
            else if (vol > 1.0f)
                vol = 1.0f;
            EffectEventAdjustEffect_Scale(m_wheel_effects[wheel],
                                          "SOUND_VOLUME", vol);
            EffectEventAdjustEffect_Scale(m_wheel_effects[wheel],
                                          "EmissionRate", vol * 60.0f);
        }
        else
        {
            math::Position3 hitp;
            hitp.v = v17->m_b2_hitp_loc.v;
            math::Dir3 hitn;
            hitn.v = v17->m_b2_hitn_loc.v;
            UpdateWheelMarks(m_owner, wheel, m_wheel_effect_state[wheel],
                             hitp, hitn);
        }
    }

    // Exhaust/dust effect tail: start or adjust the exhaust effect.
    if (m_exhaust_effect.mVal != 0)
    {
        float scale = m_current_fwd_fric_scale * 0.0033333334f;
        if (scale > 1.0f)
            scale = 1.0f;
        float v53 = fabs(m_throttle) * 30.0f + scale * 10.0f;
        EffectEventAdjustEffect_Scale(m_exhaust_effect, "EmissionRate",
                                      v53);
    }
    else
    {
        m_exhaust_effect = PostEffectEventVehicle(m_owner, v76->name, 0x2C);
    }
    j_nullsub_50(&water_objects);
}
// ea: 0x6FC5B0
void rb_vehicle::calc_tread_matrices()
{
    // Left side wheels: 2 (front), 4 (rear), 0 (mid). The tread matrix is the
    // dir/cross basis built from the mid->front wheel axis plus the average
    // suspension height, transformed into model space and written back into
    // the tread and gear bones.
    rigid_body_constraint_wheel* w2 =
        (rigid_body_constraint_wheel*)m_wheels[2];
    rigid_body_constraint_wheel* w4 =
        (rigid_body_constraint_wheel*)m_wheels[4];
    rigid_body_constraint_wheel* w0 =
        (rigid_body_constraint_wheel*)m_wheels[0];
    __m128 posFront = _mm_sub_ps(
        w2->m_b1_wheel_center_loc.v,
        _mm_mul_ps(w2->m_b1_suspension_dir_loc.v,
                   _mm_set1_ps(w2->m_wheel_displaced_center_dist)));
    __m128 posRear = _mm_sub_ps(
        w4->m_b1_wheel_center_loc.v,
        _mm_mul_ps(w4->m_b1_suspension_dir_loc.v,
                   _mm_set1_ps(w4->m_wheel_displaced_center_dist)));
    __m128 posMid = _mm_sub_ps(
        w0->m_b1_wheel_center_loc.v,
        _mm_mul_ps(w0->m_b1_suspension_dir_loc.v,
                   _mm_set1_ps(w0->m_wheel_displaced_center_dist)));
    int leftTread =
        m_owner->mDObj->GetBoneIndexInternal(g_tag_left_tread_hash);
    if (leftTread < 0)
        return;
    {
        math::Mat43* mat =
            (math::Mat43*)&m_owner->mDObj->GetMat(leftTread);
        __m128 delta = _mm_sub_ps(posMid, posFront);
        __m128 d2 = _mm_mul_ps(delta, delta);
        float len_sq =
            d2.m128_f32[0]
            + (_mm_shuffle_ps(d2, d2, 85).m128_f32[0]
               + _mm_shuffle_ps(d2, d2, 170).m128_f32[0]);
        __m128 dir = _mm_div_ps(delta, _mm_set1_ps(sqrtf(len_sq)));
        __m128 y = mat->y.v;
        __m128 cross = _mm_sub_ps(
            _mm_mul_ps(_mm_shuffle_ps(dir, dir, 9),
                       _mm_shuffle_ps(y, y, 18)),
            _mm_mul_ps(_mm_shuffle_ps(dir, dir, 18),
                       _mm_shuffle_ps(y, y, 9)));
        float zAvg =
            ((_mm_shuffle_ps(posMid, posMid, 0xAA).m128_f32[0]
              + _mm_shuffle_ps(posRear, posRear, 0xAA).m128_f32[0]
              + _mm_shuffle_ps(posFront, posFront, 0xAA).m128_f32[0])
             * 0.33333334f)
            - m_parameter->m_wheel_radius;
        __m128 pt = _mm_setr_ps(posRear.m128_f32[0], posRear.m128_f32[1],
                                zAvg, zAvg);
        math::Mat43 m;
        m.x.v = dir;
        m.y.v = y;
        m.z.v = cross;
        m.w.v = pt;
        phys_full_inv_multiply_mat(m, m_chassis_rbinf->m_transform, m);
        *mat = m;
        int leftGear =
            m_owner->mDObj->GetBoneIndexInternal(g_tag_left_gear_hash);
        *(math::Mat43*)&m_owner->mDObj->GetMat(leftGear) = m;
    }

    // Right side mirrors with wheels 3 (front), 5 (rear), 1 (mid).
    rigid_body_constraint_wheel* w3 =
        (rigid_body_constraint_wheel*)m_wheels[3];
    rigid_body_constraint_wheel* w5 =
        (rigid_body_constraint_wheel*)m_wheels[5];
    rigid_body_constraint_wheel* w1 =
        (rigid_body_constraint_wheel*)m_wheels[1];
    __m128 rPosFront = _mm_sub_ps(
        w3->m_b1_wheel_center_loc.v,
        _mm_mul_ps(w3->m_b1_suspension_dir_loc.v,
                   _mm_set1_ps(w3->m_wheel_displaced_center_dist)));
    __m128 rPosRear = _mm_sub_ps(
        w5->m_b1_wheel_center_loc.v,
        _mm_mul_ps(w5->m_b1_suspension_dir_loc.v,
                   _mm_set1_ps(w5->m_wheel_displaced_center_dist)));
    __m128 rPosMid = _mm_sub_ps(
        w1->m_b1_wheel_center_loc.v,
        _mm_mul_ps(w1->m_b1_suspension_dir_loc.v,
                   _mm_set1_ps(w1->m_wheel_displaced_center_dist)));
    int rightTread =
        m_owner->mDObj->GetBoneIndexInternal(g_tag_right_tread_hash);
    {
        math::Mat43* mat =
            (math::Mat43*)&m_owner->mDObj->GetMat(rightTread);
        __m128 delta = _mm_sub_ps(rPosMid, rPosFront);
        __m128 d2 = _mm_mul_ps(delta, delta);
        float len_sq =
            d2.m128_f32[0]
            + (_mm_shuffle_ps(d2, d2, 85).m128_f32[0]
               + _mm_shuffle_ps(d2, d2, 170).m128_f32[0]);
        __m128 dir = _mm_div_ps(delta, _mm_set1_ps(sqrtf(len_sq)));
        __m128 y = mat->y.v;
        __m128 cross = _mm_sub_ps(
            _mm_mul_ps(_mm_shuffle_ps(dir, dir, 9),
                       _mm_shuffle_ps(y, y, 18)),
            _mm_mul_ps(_mm_shuffle_ps(dir, dir, 18),
                       _mm_shuffle_ps(y, y, 9)));
        float zAvg =
            ((_mm_shuffle_ps(rPosMid, rPosMid, 0xAA).m128_f32[0]
              + _mm_shuffle_ps(rPosRear, rPosRear, 0xAA).m128_f32[0]
              + _mm_shuffle_ps(rPosFront, rPosFront, 0xAA).m128_f32[0])
             * 0.33333334f)
            - m_parameter->m_wheel_radius;
        __m128 pt = _mm_setr_ps(rPosRear.m128_f32[0], rPosRear.m128_f32[1],
                                zAvg, zAvg);
        math::Mat43 m;
        m.x.v = dir;
        m.y.v = y;
        m.z.v = cross;
        m.w.v = pt;
        phys_full_inv_multiply_mat(m, m_chassis_rbinf->m_transform, m);
        *mat = m;
        int rightGear =
            m_owner->mDObj->GetBoneIndexInternal(g_tag_right_gear_hash);
        *(math::Mat43*)&m_owner->mDObj->GetMat(rightGear) = m;
    }
}

// ea: 0x701B70
void rb_vehicle::get_wheel_matrix(int i, math::Mat43* mat) const
{
    rigid_body_constraint_wheel* v5 =
        (rigid_body_constraint_wheel*)m_wheels[i];
    float sin, cos;
    FastSinCos(v5->m_wheel_pos, &sin, &cos);
    __m128 susp = v5->m_b1_suspension_dir_loc.v;
    __m128 axis = v5->m_b1_wheel_axis_loc.v;
    __m128 cross = _mm_sub_ps(
        _mm_mul_ps(_mm_shuffle_ps(axis, axis, 9),
                   _mm_shuffle_ps(susp, susp, 18)),
        _mm_mul_ps(_mm_shuffle_ps(axis, axis, 18),
                   _mm_shuffle_ps(susp, susp, 9)));
    mat->y.v = axis;
    mat->z.v = _mm_add_ps(_mm_mul_ps(susp, _mm_set1_ps(cos)),
                          _mm_mul_ps(cross, _mm_set1_ps(sin)));
    mat->x.v = _mm_sub_ps(_mm_mul_ps(cross, _mm_set1_ps(cos)),
                          _mm_mul_ps(susp, _mm_set1_ps(sin)));
    if (i != 0
        || VEH_GetInfo(((scr_vehicle_t*)m_owner->scr_vehicle)->infoIdx)
                   ->subtype
               != 2)
    {
        mat->w.v = _mm_sub_ps(
            v5->m_b1_wheel_center_loc.v,
            _mm_mul_ps(v5->m_b1_suspension_dir_loc.v,
                       _mm_set1_ps(v5->m_wheel_displaced_center_dist)));
    }
    else
    {
        // Steering-wheel subtype: wheel 0 is anchored to the steering wheel
        // bone. The center comes from the base-pose bone matrices (lowered by
        // the suspension adjustment), transformed into the animated steering
        // bone frame and then into rb space by the chassis m_transform.
        void** model_slot = (void**)((char*)m_owner->mDObj->models + 0);
        ValidatePakId((TPakId)(uintptr_t)model_slot[1]);
        XModelLocal* mValue = (XModelLocal*)model_slot[0];
        XModelLodLocal* lod = mValue->lod[0];
        int v27 = 0;
        if (lod == nullptr)
        {
            do
            {
                lod = mValue->lod[++v27];
            } while (lod == nullptr);
        }
        XModelPartsLocal* parts =
            (XModelPartsLocal*)lod->xmodelParts;
        unsigned int steeringBone =
            xmodel_parts_get_bone_index(parts, g_tag_steeringwheel_hash);
        math::Mat43::Packed* wheel0Base =
            &iv_packed_at(&parts->mTransforms,
                          (unsigned int)m_wheel_bone_indices[0]);
        math::Mat43::Packed* steerBase =
            &iv_packed_at(&parts->mTransforms, steeringBone);

        __m128 lwc = _mm_setr_ps(wheel0Base->w.x, wheel0Base->w.y,
                                 wheel0Base->w.z - m_parameter->m_susp_adj,
                                 0.0f);
        __m128 xr = _mm_setr_ps(steerBase->x.x, steerBase->x.y,
                                steerBase->x.z, 0.0f);
        __m128 yr = _mm_setr_ps(steerBase->y.x, steerBase->y.y,
                                steerBase->y.z, 0.0f);
        __m128 zr = _mm_setr_ps(steerBase->z.x, steerBase->z.y,
                                steerBase->z.z, 0.0f);
        __m128 wr = _mm_setr_ps(steerBase->w.x, steerBase->w.y,
                                steerBase->w.z, 0.0f);
        // Transposed columns of the steering bone base pose. The binary's
        // shuffle sequence (0x88/0xA8/0xDD) reuses the z-row's z component in
        // lane 2 of column 0 and lane 0 of column 2 - preserved verbatim.
        __m128 v0 = _mm_shuffle_ps(xr, yr, 0x44);  // [x0,x1,y0,y1]
        __m128 v3 = _mm_shuffle_ps(xr, yr, 0xEE);  // [x2,0,y2,0]
        __m128 v2 = v0;
        __m128 col1 = _mm_shuffle_ps(v0, zr, 0xDD);   // [x1,y1,z1,0]
        __m128 col2 = _mm_shuffle_ps(v3, zr, 0xA8);   // [x2,y2,z2,z2]
        __m128 col0 = _mm_shuffle_ps(v2, zr, 0x88);   // [x0,y0,z2,z2]
        __m128 w0 = _mm_shuffle_ps(wr, wr, 0);
        __m128 w1 = _mm_shuffle_ps(wr, wr, 0x55);
        __m128 w2 = _mm_shuffle_ps(wr, wr, 0xAA);
        __m128 s = _mm_add_ps(
            _mm_add_ps(_mm_mul_ps(w0, col0), _mm_mul_ps(w1, col1)),
            _mm_mul_ps(w2, col2));
        __m128 pos = _mm_add_ps(
            _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(lwc, lwc, 0), col0),
                       _mm_mul_ps(_mm_shuffle_ps(lwc, lwc, 0x55), col1)),
            _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(lwc, lwc, 0xAA), col2),
                       _mm_xor_ps(Float4_SignMask_12, s)));
        math::Position3 model_pos;  // toPosition3 (0x6F0AE0) - 16-byte copy
        model_pos.v = pos;

        // Animated steering wheel bone matrix (DSkel.mat[steeringBone]).
        DObjSkelMat* skelMat =
            (DObjSkelMat*)((char*)m_owner->mDObj->skel + 0x30
                           + steeringBone * 0x40);
        __m128 sx = _mm_load_ps(skelMat->axis[0]);
        __m128 sy = _mm_load_ps(skelMat->axis[1]);
        __m128 sz = _mm_load_ps(skelMat->axis[2]);
        __m128 sw = _mm_load_ps(skelMat->origin);
        float displaced = ((rigid_body_constraint_wheel*)m_wheels[0])
                              ->m_wheel_displaced_center_dist;
        __m128 rb_pos = _mm_add_ps(
            _mm_add_ps(
                _mm_add_ps(
                    _mm_mul_ps(
                        _mm_shuffle_ps(model_pos.v, model_pos.v, 0), sx),
                    _mm_mul_ps(
                        _mm_shuffle_ps(model_pos.v, model_pos.v, 0x55), sy)),
                _mm_add_ps(
                    _mm_mul_ps(
                        _mm_shuffle_ps(model_pos.v, model_pos.v, 0xAA), sz),
                    sw)),
            _mm_mul_ps(sz, _mm_set1_ps(displaced)));
        const math::Mat43& T = m_chassis_rbinf->m_transform;
        mat->w.v = _mm_add_ps(
            _mm_add_ps(
                _mm_mul_ps(_mm_shuffle_ps(rb_pos, rb_pos, 0), T.x.v),
                _mm_mul_ps(_mm_shuffle_ps(rb_pos, rb_pos, 0x55), T.y.v)),
            _mm_add_ps(
                _mm_mul_ps(_mm_shuffle_ps(rb_pos, rb_pos, 0xAA), T.z.v),
                T.w.v));
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

// EffectEventSys (core.o; ?StopEffect@EffectEventSys@@QAEXVHandle@@_N@Z)
class EffectEventSys {
public:
    static EffectEventSys* sInst;  // ?sInst@EffectEventSys@@2PAV1@A
    void StopEffect(Handle handle, bool kill);
};

// ea: 0x700D50
void rb_vehicle::pause_physics(bool shutdown)
{
    if ((m_flags.mMask & 1) == 0)
    {
        if (!shutdown)
        {
            m_owner->Notify(hash_const.physicsdone);
            m_prev_rb_mat = m_chassis_rbinf->m_rb->get_mat();
            remove_wheels();
            if (m_orientation_constraint != nullptr)
                phys_sys::destroy(m_orientation_constraint);
            m_orientation_constraint = nullptr;
        }
        if (m_vpc != nullptr)
        {
            path_constraint_destroy(m_vpc);
            m_vpc = nullptr;
        }
        scr_vehicle_t* scr_vehicle = (scr_vehicle_t*)m_owner->scr_vehicle;
        scr_vehicle->phys.vel.v.m128_f32[2] = 0.0f;
        scr_vehicle->phys.vel.v.m128_f32[1] = 0.0f;
        scr_vehicle->phys.vel.v.m128_f32[0] = 0.0f;
        scr_vehicle->phys.rotVel.v.m128_f32[2] = 0.0f;
        scr_vehicle->phys.rotVel.v.m128_f32[1] = 0.0f;
        scr_vehicle->phys.rotVel.v.m128_f32[0] = 0.0f;
        m_flags.mMask |= 1u;
        rb_prop_system::remove_entity(m_owner);
        m_chassis_rbinf = nullptr;
        m_owner->flags &= ~0x400000u;
        m_owner->speed = 0.0f;
        for (int i = 0; i < 4; ++i)
        {
            if (m_wheel_effects[i].mVal != 0)
            {
                EffectEventSys::sInst->StopEffect(m_wheel_effects[i], false);
                m_wheel_effects[i].mVal = 0;
            }
        }
        if (m_exhaust_effect.mVal != 0)
        {
            EffectEventSys::sInst->StopEffect(m_exhaust_effect, false);
            m_exhaust_effect.mVal = 0;
        }
    }
}

// ea: 0x708E30
void rb_vehicle::start_physics()
{
    math::Mat43 v21;
    v21 = m_owner->CalcRotTranMat43();
    unsigned int v3 = m_flags.mMask & 0xFFFFFFFC;
    m_flags.mMask &= ~1u;
    m_flags.mMask = v3;
    _set_default_pose_wheels_only();
    if (m_owner->r.bmodel == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::JRS;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\RBVehicle.cpp";
        AeAssert::gCurrentLine = 207;
        AeAssert::gCurrentExpr = "m_owner->r.bmodel";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert(
                   "Can not create a vehicle without a collmap"))
            __debugbreak();
    }
    Entity* m_owner_ = m_owner;
    // bmodel is a collmap (DCGSet); nboxes lives at +0x30 (bbox min/max).
    struct collmap_view {
        uint8_t _pad0[0x30];
        float nboxes[32];  // +0x30
    };
    float* p_nboxes = ((collmap_view*)m_owner_->r.bmodel)->nboxes;
    float* v6 = p_nboxes + 12;
    math::Position3 v23;
    v23.v.m128_f32[0] = p_nboxes[12];
    float v7 = p_nboxes[13];
    p_nboxes += 16;
    v23.v.m128_f32[1] = v7;
    float v8 = v6[2];
    v23.v.m128_f32[3] = v6[3];
    math::Position3 v22;
    v22.v.m128_f32[0] = *p_nboxes;
    v22.v.m128_f32[1] = p_nboxes[1];
    float v9 = p_nboxes[2];
    v22.v.m128_f32[3] = p_nboxes[3];
    DObj* mDObj = m_owner_->mDObj;
    v23.v.m128_f32[2] = v8;
    v22.v.m128_f32[2] = v9;
    if (mDObj == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::JRS;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\RBVehicle.cpp";
        AeAssert::gCurrentLine = 212;
        AeAssert::gCurrentExpr = "m_owner->GetDObj()";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Physics entity with no DObj."))
            __debugbreak();
    }
    __m128 v = v22.v;
    __m128 half = _mm_set1_ps(0.5f);
    v22.v = _mm_mul_ps(_mm_add_ps(v22.v, v23.v), half);
    __m128 v13 = _mm_mul_ps(_mm_sub_ps(v, v23.v), half);
    __m128 v14 = _mm_mul_ps(v13, v13);
    float v24 = v14.m128_f32[0]
        + (v14.m128_f32[1] + v14.m128_f32[2]);
    SetIdentity(v21);
    v21.w.v = _mm_xor_ps(_mm_set1_ps(-0.0f), v22.v);
    rb_extra_info* v15 =
        g_list_rb_extra_info.add(false, "phys memory pool add overflow.");
    float bs_radius = sqrtf(v24);
    rb_extra_info* v17 = v15;
    m_chassis_rbinf = v15;
    rigid_body* rb = phys_sys::create_rigid_body(false);
    v17->set(m_owner, rb, v21, bs_radius, v22);
    v17->m_priority = 2;
    int v19 = v17->m_flags.mMask | 8;
    v17->m_flags.mMask = v19;
    v17->m_flags.mMask = v19 | 0x10;
    v17->evaluate_effect_priority();
    m_chassis_rbinf->m_rb_vehicle = this;
    update_parms(m_parameter, true);
    m_owner->flags |= 0x400000;
    m_flags.mMask |= 4u;
    if (g_in_physics_collision_callback)
        v17->collision_prolog();
    m_owner->Notify(hash_const.physicsstart);
    mVehicleController.m_stuck_time = 0.0f;
}

// ea: 0x709090
void rb_vehicle::_update_unpause()
{
    int playersAttached = ((scr_vehicle_t*)m_owner->scr_vehicle)
                              ->playersAttached;
    unsigned int mMask = m_flags.mMask;
    bool v4 = playersAttached != 0;
    unsigned int v5 = playersAttached != 0 ? (mMask | 0x80)
                                            : (mMask & 0xFFFFFF7F);
    m_flags.mMask = v5;
    bool v6 = (v5 & 8) != 0 || (v5 & 0x100) != 0 || (v5 & 0x200) != 0;
    bool v7 = v6 || m_owner->flags < 0;
    rb_extra_info* m_chassis_rbinf = this->m_chassis_rbinf;
    bool v9 = v7 || v4;
    if (m_chassis_rbinf != nullptr)
    {
        rigid_body* m_rb = m_chassis_rbinf->m_rb;
        unsigned int m_flags = m_rb->m_flags;
        m_rb->m_flags = v9 ? (m_flags | 0x100) : (m_flags & 0xFFFFFEFF);
    }
    if ((m_flags.mMask & 1) != 0)
    {
        if (v9)
        {
            m_flags.mMask |= 2u;
            goto LABEL_22;
        }
    }
    else if (v9)
    {
        goto LABEL_22;
    }
    if ((m_flags.mMask & 1) == 0
        && ((m_chassis_rbinf->m_rb->m_flags & 8) != 0
            || !Entity_has_zone_collision(m_owner)))
    {
        pause_physics(false);
    }
LABEL_22:
    if ((m_flags.mMask & 2) != 0)
    {
        if (g_rb_vehicle_list.m_alloc_count > 10)
        {
            AeAssert::gCurrentAuthor = AeAssert::JRS;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\RBVehicle.cpp";
            AeAssert::gCurrentLine = 361;
            AeAssert::gCurrentExpr = "0";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert(
                       "Warning: Too many vehicles in physics. Can't unpause yet!"))
                __debugbreak();
        }
        else
        {
            start_physics();
        }
    }
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

// ea: 0x70C0A0
void rb_vehicle::start_path(int attach_mode)
{
    unsigned int v4 = m_flags.mMask & 0xFFFFFDF7;
    m_flags.mMask &= ~8u;
    m_flags.mMask = v4;
    m_flags.mMask = v4 & 0xFFFFFEFF;
    if (attach_mode != 0)
    {
        unsigned int v19 = m_flags.mMask | 0x200;
        unsigned char mMask = (unsigned char)m_flags.mMask;
        m_flags.mMask = v19;
        if ((mMask & 1) != 0)
        {
            m_flags.mMask = v19 | 2;
            _update_unpause();
        }
    }
    else
    {
        pause_physics(false);
        unsigned int v5 = m_flags.mMask | 0x100;
        unsigned char v6 = (unsigned char)m_flags.mMask;
        m_flags.mMask = v5;
        if ((v6 & 1) == 0
            || (m_flags.mMask = v5 | 2, _update_unpause(),
                (m_flags.mMask & 1) == 0))
        {
            scr_vehicle_t* scr_vehicle = (scr_vehicle_t*)m_owner->scr_vehicle;
            math::Position3 pos;
            math::Position3 angles;
            pos.v = _mm_setr_ps(scr_vehicle->pathPos.origin[0],
                                scr_vehicle->pathPos.origin[1],
                                scr_vehicle->pathPos.origin[2], 0.0f);
            angles.v = _mm_setr_ps(scr_vehicle->pathPos.angles[0],
                                   scr_vehicle->pathPos.angles[1],
                                   scr_vehicle->pathPos.angles[2], 0.0f);
            AnglesToAxis(angles, pos, m_prev_rb_mat);
            math::Mat43& mat = m_chassis_rbinf->m_rb->dangerous_get_mat();
            memcpy(&mat, &m_prev_rb_mat, sizeof(math::Mat43));
            if (m_vpc == nullptr)
            {
                m_vpc = path_constraint_create(m_owner);
                for (int w = 0; w < 8; ++w)
                {
                    rigid_body_constraint_wheel* wheel =
                        (rigid_body_constraint_wheel*)m_wheels[w];
                    if (wheel != nullptr)
                    {
                        wheel->m_fwd_fric_k = 0.0099999998f;
                        wheel->m_side_fric_k = 0.0099999998f;
                    }
                }
            }
        }
    }
}

// ea: 0x701B40
void rb_vehicle::debug_render_all()
{
    int count = g_rb_vehicle_list.m_alloc_count;
    for (int i = 0; i < count; ++i)
        g_rb_vehicle_list.m_alloc_list[i]->debug_render();
}

// ea: 0x6FCDF0
void rb_vehicle::debug_render()
{
    if (sRenderAllVehicles != 0 || (m_flags.mMask & 0x10) != 0)
    {
        rb_extra_info* m_chassis_rbinf = this->m_chassis_rbinf;
        if (m_chassis_rbinf != nullptr)
        {
            rigid_body* m_rb = m_chassis_rbinf->m_rb;
            if ((~(m_rb->m_flags >> 6) & 1) == 0
                && _tlAssert(
                       "c:\\cod\\code\\tl\\physics\\include\\rigid_body.h",
                       79, "debug_flag_is_not_in_collision()",
                       defaultFileName))
                __debugbreak();
            math::Position3 rb_pos;
            rb_pos.v = m_rb->m_mat.w.v;
            DebugRender::RenderSphere(
                rb_pos, 10.0f,
                Color(1.0f, 1.0f, 0.0f, 1.0f));  // yellow-white

            vehicle_rb_parameter* m_parameter = this->m_parameter;
            if (m_parameter->m_bbox_min.v.m128_f32[0] != 0.0f)
            {
                // bbox center/size from min/max, translated by origin
                __m128 bmin = m_parameter->m_bbox_min.v;
                __m128 bmax = m_parameter->m_bbox_max.v;
                __m128 size = _mm_sub_ps(bmax, bmin);
                __m128 center = _mm_mul_ps(
                    _mm_add_ps(bmax, bmin), _mm_set1_ps(0.5f));
                math::Mat43 transform;  // SetIdentity (0x6E4D00)
                SetIdentity(transform);
                transform.w.v = center;
                math::Position3 origin = m_owner->r.currentOrigin;
                math::Mat43 basis;  // dir basis scaled by 0.9
                basis.x.v = _mm_mul_ps(transform.x.v, _mm_set1_ps(0.9f));
                basis.y.v = _mm_mul_ps(transform.y.v, _mm_set1_ps(0.9f));
                basis.z.v = _mm_mul_ps(transform.z.v, _mm_set1_ps(0.9f));
                basis.w.v = _mm_add_ps(
                    _mm_add_ps(
                        _mm_add_ps(
                            _mm_mul_ps(
                                _mm_shuffle_ps(origin.v, origin.v, 0),
                                transform.x.v),
                            _mm_mul_ps(
                                _mm_shuffle_ps(origin.v, origin.v, 0x55),
                                transform.y.v)),
                        _mm_mul_ps(
                            _mm_shuffle_ps(origin.v, origin.v, 0xAA),
                            transform.z.v)),
                    transform.w.v);
                basis.w.v = _mm_add_ps(basis.w.v, _mm_setr_ps(0.0f, 10.0f,
                                                              0.0f, 0.0f));
                math::DiagMat33 dsize;
                dsize.v = size;
                dsize.v = _mm_mul_ps(dsize.v, _mm_set1_ps(0.9f));
                dsize.v = _mm_mul_ps(
                    dsize.v,
                    _mm_setr_ps(m_parameter->m_inertia_scale_x, 1.0f, 1.0f,
                                0.0f));
                DebugRender::RenderLineBox(
                    basis, dsize, Color(0.0f, 1.0f, 0.0f, 1.0f));
            }

            for (int w = 0; w < 8; ++w)
            {
                rigid_body_constraint_wheel* v21 =
                    (rigid_body_constraint_wheel*)m_wheels[w];
                Color wheel_col(0.0f, 1.0f, 0.0f, 1.0f);
                Color sphere_col(1.0f, 1.0f, 1.0f, 1.0f);
                if (v21 != nullptr)
                {
                    rigid_body* b1 = v21->b1;
                    float wheel_radius = v21->m_wheel_radius;
                    if ((~(b1->m_flags >> 6) & 1) == 0
                        && _tlAssert(
                               "c:\\cod\\code\\tl\\physics\\include\\rigid_body.h",
                               79, "debug_flag_is_not_in_collision()",
                               defaultFileName))
                        __debugbreak();

                    // wheel center displaced by suspension
                    math::Position3 center;
                    center.v = _mm_sub_ps(
                        v21->m_b1_wheel_center_loc.v,
                        _mm_mul_ps(
                            v21->m_b1_suspension_dir_loc.v,
                            _mm_set1_ps(v21->m_wheel_displaced_center_dist)));
                    // to rb world
                    center.v = _mm_add_ps(
                        _mm_add_ps(
                            _mm_mul_ps(
                                _mm_shuffle_ps(center.v, center.v, 0),
                                b1->m_mat.x.v),
                            _mm_mul_ps(
                                _mm_shuffle_ps(center.v, center.v, 0x55),
                                b1->m_mat.y.v)),
                        _mm_add_ps(
                            _mm_mul_ps(
                                _mm_shuffle_ps(center.v, center.v, 0xAA),
                                b1->m_mat.z.v),
                            b1->m_mat.w.v));

                    // hit point (b2_hitp_loc)
                    math::Position3 hitp;
                    hitp.v = v21->m_b2_hitp_loc.v;
                    if ((~(b1->m_flags >> 6) & 1) == 0
                        && _tlAssert(
                               "c:\\cod\\code\\tl\\physics\\include\\rigid_body.h",
                               79, "debug_flag_is_not_in_collision()",
                               defaultFileName))
                        __debugbreak();
                    // hitp offset along suspension by roll stability
                    __m128 susp = v21->m_b1_suspension_dir_loc.v;
                    __m128 s_world = _mm_add_ps(
                        _mm_add_ps(
                            _mm_mul_ps(_mm_shuffle_ps(susp, susp, 0),
                                       b1->m_mat.x.v),
                            _mm_mul_ps(_mm_shuffle_ps(susp, susp, 0x55),
                                       b1->m_mat.y.v)),
                        _mm_mul_ps(_mm_shuffle_ps(susp, susp, 0xAA),
                                   b1->m_mat.z.v));
                    math::Position3 hit_pos;
                    hit_pos.v = _mm_sub_ps(
                        hitp.v,
                        _mm_mul_ps(s_world,
                                   _mm_set1_ps(v21->m_roll_stability_factor)));
                    if ((v21->m_wheel_flags & 1) != 0)
                        wheel_col = Color(1.0f, 1.0f, 0.0f, 1.0f);
                    if ((v21->m_wheel_flags & 4) != 0)
                        wheel_col = Color(1.0f, 0.0f, 0.0f, 1.0f);

                    // hard limit point
                    __m128 lim = _mm_sub_ps(
                        v21->m_b1_wheel_center_loc.v,
                        _mm_mul_ps(
                            susp,
                            _mm_set1_ps(v21->m_hard_limit_dist
                                        + wheel_radius)));
                    if ((~(b1->m_flags >> 6) & 1) == 0
                        && _tlAssert(
                               "c:\\cod\\code\\tl\\physics\\include\\rigid_body.h",
                               79, "debug_flag_is_not_in_collision()",
                               defaultFileName))
                        __debugbreak();
                    math::Position3 limit_world;
                    limit_world.v = _mm_add_ps(
                        _mm_add_ps(
                            _mm_mul_ps(_mm_shuffle_ps(lim, lim, 0),
                                       b1->m_mat.x.v),
                            _mm_mul_ps(_mm_shuffle_ps(lim, lim, 0x55),
                                       b1->m_mat.y.v)),
                        _mm_add_ps(
                            _mm_mul_ps(_mm_shuffle_ps(lim, lim, 0xAA),
                                       b1->m_mat.z.v),
                            b1->m_mat.w.v));
                    bool has_contact = (v21->m_wheel_flags & 2) == 0;
                    Color contact_col = has_contact
                                            ? Color(0.0f, 1.0f, 0.0f, 1.0f)
                                            : Color(1.0f, 0.0f, 0.0f, 1.0f);
                    DebugRender::RenderSphere(limit_world, sphere_size,
                                              contact_col);
                    DebugRender::RenderSphere(hitp, sphere_size,
                                              Color(1.0f, 0.0f, 0.0f, 1.0f));
                    DebugRender::RenderSphere(hit_pos, sphere_size,
                                              Color(0.0f, 1.0f, 1.0f, 1.0f));
                    DebugRender::RenderSphere(center, wheel_radius, wheel_col);

                    if ((m_flags.mMask & 0x20) != 0)
                    {
                        if ((~(b1->m_flags >> 6) & 1) == 0
                            && _tlAssert(
                                   "c:\\cod\\code\\tl\\physics\\include\\rigid_body.h",
                                   79, "debug_flag_is_not_in_collision()",
                                   defaultFileName))
                            __debugbreak();
                        math::Dir3 axis_world;
                        axis_world.v = _mm_add_ps(
                            _mm_add_ps(
                                _mm_mul_ps(
                                    _mm_shuffle_ps(
                                        v21->m_b1_wheel_axis_loc.v,
                                        v21->m_b1_wheel_axis_loc.v, 0),
                                    b1->m_mat.x.v),
                                _mm_mul_ps(
                                    _mm_shuffle_ps(
                                        v21->m_b1_wheel_axis_loc.v,
                                        v21->m_b1_wheel_axis_loc.v, 0x55),
                                    b1->m_mat.y.v)),
                            _mm_mul_ps(
                                _mm_shuffle_ps(
                                    v21->m_b1_wheel_axis_loc.v,
                                    v21->m_b1_wheel_axis_loc.v, 0xAA),
                                b1->m_mat.z.v));
                        G_DebugCircleEx(
                            &center.v.m128_f32[0], wheel_radius,
                            &axis_world.v.m128_f32[0], colorWhite, 0, 0);
                        int leftTread = m_owner->mDObj->GetBoneIndexInternal(
                            g_tag_left_tread_hash);
                        math::Mat43 left_mat = m_owner->CalcAbsMat(leftTread);
                        DebugRender::RenderAxis(left_mat, 50.0f, 1.0f);
                        int rightTread = m_owner->mDObj->GetBoneIndexInternal(
                            g_tag_right_tread_hash);
                        math::Mat43 right_mat = m_owner->CalcAbsMat(rightTread);
                        DebugRender::RenderAxis(right_mat, 50.0f, 1.0f);
                    }

                    float hitp_z = hit_pos.v.m128_f32[2];
                    float center_z = center.v.m128_f32[2];
                    Color line_col(0.0f, 1.0f, 1.0f, 1.0f);
                    if (hitp_z > center_z - 5.0f)
                        line_col = Color(1.0f, 0.0f, 0.0f, 1.0f);
                    DebugRender::RenderLine(center, hit_pos, line_col,
                                            0.050000001f);
                }
            }
        }
    }
}

// ea: 0x6F2910 (inline COMDAT)
void rb_vehicle::set_brake(float braking)
{
    if (braking < 0.0f || braking > 1.0f)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\RBVehicle.h";
        AeAssert::gCurrentLine = 376;
        AeAssert::gCurrentExpr = "braking>= 0.0f && braking<= 1.0f";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Out of range"))
            __debugbreak();
    }
    m_brake = braking;
}

// ea: 0x6F2A50 (inline COMDAT)
void rb_vehicle::set_throttle(float throttle)
{
    if (throttle < -1.0f || throttle > 1.0f)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\RBVehicle.h";
        AeAssert::gCurrentLine = 388;
        AeAssert::gCurrentExpr = "throttle>= -1.0f && throttle<= 1.0f";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Out of range"))
            __debugbreak();
    }
    m_throttle = throttle;
}

// ea: 0x6E69B0 (inline COMDAT)
void rb_vehicle::set_hand_brake(float braking)
{
    if (braking < 0.0f || braking > 1.0f)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\RBVehicle.h";
        AeAssert::gCurrentLine = 382;
        AeAssert::gCurrentExpr = "braking>= 0.0f && braking<= 1.0f";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Out of range"))
            __debugbreak();
    }
    m_hand_brake = braking;
}

// ea: 0x71A0D0 (inline COMDAT)
void rb_vehicle::set_steer_factor(float steer_factor)
{
    float beg = -1.0f, end = 1.0f;
    if (steer_factor < beg)
        steer_factor = beg;
    else if (steer_factor > end)
        steer_factor = end;
    m_steer_factor = steer_factor;
}

// ea: 0x6FC200
math::Dir3 rb_vehicle::get_velocity() const
{
    math::Dir3 result;
    if ((m_flags.mMask & 1) != 0)
        result.v = _mm_setzero_ps();
    else
        result.v = m_chassis_rbinf->m_rb->m_t_vel.v;
    return result;
}

// ea: 0x6F5860
void rb_vehicle::update_from_network(const math::Position3& position,
                                     const math::Position3& angles,
                                     const math::Dir3& vel,
                                     const math::Dir3& aVel)
{
    G_SetOrigin(m_owner, &position);
    G_SetAngle(m_owner, &angles);
    m_owner->CalcRotTranMat43();
    rb_extra_info* m_chassis_rbinf = this->m_chassis_rbinf;
    if (m_chassis_rbinf != nullptr)
    {
        rigid_body* m_rb = m_chassis_rbinf->m_rb;
        if ((~(m_rb->m_flags >> 6) & 1) == 0
            && _tlAssert("c:\\cod\\code\\tl\\physics\\include\\rigid_body.h",
                         85, "debug_flag_is_not_in_collision()",
                         defaultFileName))
            __debugbreak();
        m_rb->m_t_vel.v = vel.v;
        m_rb->m_a_vel.v = aVel.v;
        float axis[3][3];
        AnglesToAxis(&angles.v.m128_f32[0], axis);
        m_rb->m_mat.x.v = _mm_setr_ps(axis[0][0], axis[0][1], axis[0][2], 0.0f);
        m_rb->m_mat.y.v = _mm_setr_ps(axis[1][0], axis[1][1], axis[1][2], 0.0f);
        m_rb->m_mat.z.v = _mm_setr_ps(axis[2][0], axis[2][1], axis[2][2], 0.0f);
        // chassis offset from rb origin (rb_extra_info +0x30 = m_transform.w)
        __m128 chassis_offset = m_chassis_rbinf->m_transform.w.v;
        math::Position3 predicted;
        predicted.v = _mm_sub_ps(position.v, chassis_offset);
        __m128 diff = _mm_sub_ps(m_rb->m_mat.w.v, predicted.v);
        __m128 d2 = _mm_mul_ps(diff, diff);
        float dist_sq = d2.m128_f32[0]
                        + (_mm_shuffle_ps(d2, d2, 85).m128_f32[0]
                           + _mm_shuffle_ps(d2, d2, 170).m128_f32[0]);
        if (sqrtf(dist_sq) > 2.0f)
            m_rb->m_mat.w.v = predicted.v;
    }
}

// DCGSet (sv_stubs.h view; local copy - objects array + aabb)
class DCGSet {
public:
    uint16_t nboxes;           // +0x00
    uint16_t _pad02;           // +0x02
    int   objects_m_count;     // +0x04
    void* objects_m_elements;  // +0x08
    int   brushes_m_count;     // +0x0C
    void* brushes_m_elements;  // +0x10
    uint8_t _pad14[0x1C - 0x14];
    int   brush_sides_m_count;  // +0x1C
    void* brush_sides_m_elements;  // +0x20
    uint8_t _pad24[0x30 - 0x24];
    math::Position3 min;       // +0x30
    math::Position3 max;       // +0x40

    // get_type - ea: 0x718570 (physics.o inline COMDAT); 1 = brush, 0 = box
    int get_type(unsigned int index) const
    {
        if (index >= (unsigned int)objects_m_count)
        {
            AeAssert::gCurrentAuthor = AeAssert::JSV;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\cgbank.h";
            AeAssert::gCurrentLine = 83;
            AeAssert::gCurrentExpr = "index < size()";
            if (!AeAssert::IsIgnored() && AeAssert::Assert(defaultFileName))
                __debugbreak();
        }
        return index >= nboxes;
    }
};

// cdl_brush_t (cdl_mem.h view; objects[] element - center +0x08, half
// extents +0x14, 36 bytes)
struct cdl_brush_record {
    uint8_t _pad[8];
    float   center[3];   // +0x08
    float   extents[3];  // +0x14
};
// brushes[] side record (4 bytes: { uint16 first_side; uint16 nsides; })
struct cdl_brush_side_rec {
    uint16_t first_side;
    uint16_t nsides;
};

// ea: 0x6FD850
void rb_vehicle::update_from_scene_anim(const math::Position3& position,
                                        const math::Position3& angles,
                                        const math::Dir3& vel)
{
    G_SetOrigin(m_owner, &position);
    G_SetAngle(m_owner, &angles);
    m_owner->CalcRotTranMat43();
    rb_extra_info* m_chassis_rbinf = this->m_chassis_rbinf;
    if (m_chassis_rbinf != nullptr)
    {
        rigid_body* m_rb = m_chassis_rbinf->m_rb;
        if ((~(m_rb->m_flags >> 6) & 1) == 0
            && _tlAssert("c:\\cod\\code\\tl\\physics\\include\\rigid_body.h",
                         85, "debug_flag_is_not_in_collision()",
                         defaultFileName))
            __debugbreak();
        m_chassis_rbinf->m_rb->m_t_vel.v = vel.v;
        phys_full_inv_multiply_mat(m_rb->m_mat,
                                   m_chassis_rbinf->m_transform,
                                   m_chassis_rbinf->m_ent->r.currentMat);
    }
    else
    {
        // no chassis: encode the DCGSet aabb into m_prev_rb_mat rows
        DCGSet* bmodel = (DCGSet*)m_owner->r.bmodel;
        float mn[4], mx[4];
        mn[0] = bmodel->min.v.m128_f32[0];
        mn[1] = bmodel->min.v.m128_f32[1];
        mn[2] = bmodel->min.v.m128_f32[2];
        mn[3] = bmodel->min.v.m128_f32[3];
        mx[0] = bmodel->max.v.m128_f32[0];
        mx[1] = bmodel->max.v.m128_f32[1];
        mx[2] = bmodel->max.v.m128_f32[2];
        mx[3] = bmodel->max.v.m128_f32[3];
        m_prev_rb_mat.x.v = _mm_setr_ps(mx[0], mx[1], mx[2], mx[3]);
        m_prev_rb_mat.y.v = _mm_setr_ps(mn[0], mn[1], mn[2], mn[3]);
        m_prev_rb_mat.z.v = _mm_setzero_ps();
        m_prev_rb_mat.w.v = _mm_mul_ps(
            _mm_add_ps(m_prev_rb_mat.x.v, m_prev_rb_mat.y.v),
            _mm_set1_ps(0.5f));
    }
}

// ea: 0x70C350
void rb_vehicle::teleport(const Broc::vector& vSpawnPos,
                          const Broc::vector* vAngles)
{
    pause_physics(false);
    if ((m_flags.mMask & 1) != 0)
    {
        m_flags.mMask |= 2u;
        _update_unpause();
    }
    math::Position3 pos;
    pos.v = _mm_setr_ps(vSpawnPos.x, vSpawnPos.y, vSpawnPos.z, 0.0f);
    if (vAngles != nullptr)
    {
        math::Position3 angles;
        angles.v = _mm_setr_ps(vAngles->x, vAngles->y, vAngles->z, 0.0f);
        AnglesToAxis(angles, pos, m_prev_rb_mat);
    }
    else
    {
        m_prev_rb_mat.w.v = pos.v;
    }
    if ((m_flags.mMask & 1) == 0)
    {
        rigid_body* m_rb = m_chassis_rbinf->m_rb;
        if ((~(m_rb->m_flags >> 6) & 1) == 0
            && _tlAssert("c:\\cod\\code\\tl\\physics\\include\\rigid_body.h",
                         85, "debug_flag_is_not_in_collision()",
                         defaultFileName))
            __debugbreak();
        memcpy(&m_rb->m_mat, &m_prev_rb_mat, sizeof(math::Mat43));
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

// ea: 0x70CA40
void rb_vehicle::_update_prolog(float delta_t)
{
    mVehicleController.UpdateControls(*this);
    unsigned int m_state_flags = this->m_state_flags;
    if (m_throttle <= 0.0f)
        m_state_flags &= ~4u;
    else
        m_state_flags |= 4u;
    if (m_hand_brake <= 0.0f)
        m_state_flags &= ~1u;
    else
        m_state_flags |= 1u;
    this->m_state_flags = m_state_flags;

    vehicle_rb_parameter* m_parameter = this->m_parameter;
    float m_accel_max = m_parameter->m_accel_max;
    float m_speed_max = m_parameter->m_speed_max;
    if (g_speed.integer > 300)
    {
        m_speed_max = m_parameter->m_speed_max * 4.0f;
        m_accel_max = m_parameter->m_accel_max * 4.0f;
    }
    float v17;
    if ((m_flags.mMask & 0x20) != 0)
    {
        int v9 = 0;
        for (int i = 0; i < 8; ++i)
        {
            rigid_body_constraint_wheel* w =
                (rigid_body_constraint_wheel*)m_wheels[i];
            if (w != nullptr && (w->m_wheel_flags & 1) != 0)
                ++v9;
        }
        if (v9 <= 1)
            v9 = 2;
        v17 = fabs(m_throttle) * (6.0f / v9) * m_accel_max;
        if (m_throttle >= 0.0f)
            goto accel_done;
    }
    else
    {
        v17 = fabs(m_throttle) * m_accel_max;
        if (m_throttle >= 0.0f)
            goto accel_done;
    }
    v17 = m_parameter->m_reverse_scale * v17;
    m_speed_max = m_parameter->m_reverse_scale * m_speed_max;
accel_done:
    rb_extra_info* m_chassis_rbinf = this->m_chassis_rbinf;
    if (m_chassis_rbinf != nullptr)
    {
        rigid_body* m_rb = m_chassis_rbinf->m_rb;
        if ((~(m_rb->m_flags >> 6) & 1) == 0
            && _tlAssert("c:\\cod\\code\\tl\\physics\\include\\rigid_body.h",
                         79, "debug_flag_is_not_in_collision()",
                         defaultFileName))
            __debugbreak();
        float v20 =
            _mm_shuffle_ps(m_rb->m_mat.x.v, m_rb->m_mat.x.v, 170)
                .m128_f32[0]
            * 2.5f;
        float v36;
        if (v20 >= -1.0f)
        {
            v36 = 1.0f;
            if (v20 <= 1.0f)
                v36 = v20;
        }
        else
        {
            v36 = -1.0f;
        }
        float v21 = (1.0f - fabs(m_steer_factor)) * m_throttle * v36;
        float v22;
        if (v21 >= 0.0f)
        {
            v22 = v21;
            if (v22 > 1.0f)
                v22 = 1.0f;
        }
        else
        {
            v22 = 0.0f;
        }
        v17 = (m_parameter->m_accel_max * v22) + v17;
    }
    if ((m_flags.mMask & 1) == 0)
    {
        float v23 = fabs(m_forward_vel);
        if (low_end_speed > v23)
            v17 = (((1.0f - (v23 / low_end_speed)) * percent_to_give) + 1.0f)
                  * v17;
    }
    float v24 = v17;
    if (v17 <= 2.0f)
        v24 = 2.0f;
    m_acceleration_factor = v24;
    m_desired_speed_factor = m_speed_max;
    if (m_hand_brake > 0.0f || m_throttle > 0.0f)
        m_state_flags &= ~0x10u;
    else
        m_state_flags |= 0x10u;
    if (m_brake <= 0.0f)
        m_state_flags &= ~2u;
    else
        m_state_flags |= 2u;
    m_state_flags &= ~8u;
    if (m_parameter->m_traction_type == 2)  // TRACTION_TYPE_ALL_WD
    {
        if (m_hand_brake > 0.0f)
            v17 = v17 * 2.0f;
        if (v17 <= 2.0f)
            v17 = 2.0f;
        m_acceleration_factor = v17;
    }
    if (m_chassis_rbinf != nullptr)
    {
        rigid_body* v29 = m_chassis_rbinf->m_rb;
        if ((~(v29->m_flags >> 6) & 1) == 0
            && _tlAssert("c:\\cod\\code\\tl\\physics\\include\\rigid_body.h",
                         79, "debug_flag_is_not_in_collision()",
                         defaultFileName))
            __debugbreak();
        __m128 v31 = _mm_mul_ps(m_chassis_rbinf->m_rb->m_t_vel.v,
                                v29->m_mat.x.v);
        float v32 = v31.m128_f32[0]
                    + (_mm_shuffle_ps(v31, v31, 85).m128_f32[0]
                       + _mm_shuffle_ps(v31, v31, 170).m128_f32[0]);
        m_forward_vel = v32;
        if (m_throttle >= 0.0f)
        {
            if (m_throttle > 0.0f
                && (0.0f - m_parameter->m_peel_out_max_speed) > v32)
                m_state_flags |= 2u;
        }
        else if (v32 <= 10.0f || m_num_colliding_wheels <= 1)
        {
            m_state_flags |= 8u;
        }
        else
        {
            m_state_flags |= 2u;
        }
    }
    else
    {
        m_forward_vel = 0.0f;
    }
    if (m_script_brake > 0.0f)
        m_state_flags |= 2u;
    if ((m_flags.mMask & 1) == 0)
    {
        update_braking_and_acceleration(delta_t);
        update_steering(delta_t);
        _update_fakey_stuff(delta_t);
        _update_friction(delta_t);
        _update_orientation_constraint();
        if (m_vpc != nullptr)
            path_constraint_update(m_vpc, m_owner);
    }
}

// ea: 0x7091E0
void rb_vehicle::_update_epilog(float delta_t)
{
    _update_unpause();
    _update_wheel_effects(delta_t);
    if ((m_flags.mMask & 1) != 0)
    {
        ((scr_vehicle_t*)m_owner->scr_vehicle)->phys.vel.v.m128_f32[2] = 0.0f;
        ((scr_vehicle_t*)m_owner->scr_vehicle)->phys.vel.v.m128_f32[1] = 0.0f;
        ((scr_vehicle_t*)m_owner->scr_vehicle)->phys.vel.v.m128_f32[0] = 0.0f;
        ((scr_vehicle_t*)m_owner->scr_vehicle)->phys.rotVel.v.m128_f32[2] = 0.0f;
        ((scr_vehicle_t*)m_owner->scr_vehicle)->phys.rotVel.v.m128_f32[1] = 0.0f;
        ((scr_vehicle_t*)m_owner->scr_vehicle)->phys.rotVel.v.m128_f32[0] = 0.0f;
        m_owner->speed = 0.0f;
        return;
    }
    if ((m_flags.mMask & 0x20) != 0)
    {
        calc_tread_matrices();
    }
    else
    {
        for (int i = 0; i < 8; ++i)
        {
            int bone = m_wheel_bone_indices[i];
            if ((bone & 0x80000000) == 0
                && m_owner->mDObj->numBones >= bone)
            {
                math::Mat43 mat;
                get_wheel_matrix(i, &mat);
                phys_full_inv_multiply_mat(mat,
                                           m_chassis_rbinf->m_transform,
                                           mat);
                math::Mat43& dst = const_cast<math::Mat43&>(
                    m_owner->mDObj->GetMat(bone));
                memcpy(&dst, &mat, sizeof(math::Mat43));
            }
        }
    }
    scr_vehicle_t* sv = (scr_vehicle_t*)m_owner->scr_vehicle;
    sv->phys.vel.v.m128_f32[0] = m_chassis_rbinf->m_rb->m_t_vel.v.m128_f32[0];
    sv->phys.vel.v.m128_f32[1] = m_chassis_rbinf->m_rb->m_t_vel.v.m128_f32[1];
    sv->phys.vel.v.m128_f32[2] = m_chassis_rbinf->m_rb->m_t_vel.v.m128_f32[2];
    sv->phys.vel.v.m128_f32[3] = m_chassis_rbinf->m_rb->m_t_vel.v.m128_f32[3];
    sv->phys.rotVel.v.m128_f32[0] =
        m_chassis_rbinf->m_rb->m_a_vel.v.m128_f32[0];
    sv->phys.rotVel.v.m128_f32[1] =
        m_chassis_rbinf->m_rb->m_a_vel.v.m128_f32[1];
    sv->phys.rotVel.v.m128_f32[2] =
        m_chassis_rbinf->m_rb->m_a_vel.v.m128_f32[2];
    sv->phys.rotVel.v.m128_f32[3] =
        m_chassis_rbinf->m_rb->m_a_vel.v.m128_f32[3];
    if ((m_flags.mMask & 0x100) == 0)
    {
        rigid_body* m_rb = m_chassis_rbinf->m_rb;
        if ((~(m_rb->m_flags >> 6) & 1) == 0
            && _tlAssert("c:\\cod\\code\\tl\\physics\\include\\rigid_body.h",
                         79, "debug_flag_is_not_in_collision()",
                         defaultFileName))
            __debugbreak();
        sv->phys.origin.v.m128_f32[0] = m_rb->m_mat.w.v.m128_f32[0];
        sv->phys.origin.v.m128_f32[1] = m_rb->m_mat.w.v.m128_f32[1];
        sv->phys.origin.v.m128_f32[2] = m_rb->m_mat.w.v.m128_f32[2];
        sv->phys.origin.v.m128_f32[3] = m_rb->m_mat.w.v.m128_f32[3];
        sv->phys.angles.v.m128_f32[0] = m_owner->r.currentAngles.v.m128_f32[0];
        sv->phys.angles.v.m128_f32[1] = m_owner->r.currentAngles.v.m128_f32[1];
        sv->phys.angles.v.m128_f32[2] = m_owner->r.currentAngles.v.m128_f32[2];
        sv->phys.angles.v.m128_f32[3] = m_owner->r.currentAngles.v.m128_f32[3];
        sv->current.mSteeringAngle =
            (m_steer_current_angle * 180.0f) * 0.31830987f;
        sv->next.mSteeringAngle =
            (m_steer_current_angle * 180.0f) * 0.31830987f;
        if ((m_flags.mMask & 0x200) == 0)
        {
            rigid_body* v20 = m_chassis_rbinf->m_rb;
            m_owner->speed = sqrtf(
                v20->m_t_vel.v.m128_f32[0] * v20->m_t_vel.v.m128_f32[0]
                + v20->m_t_vel.v.m128_f32[1] * v20->m_t_vel.v.m128_f32[1]
                + v20->m_t_vel.v.m128_f32[2] * v20->m_t_vel.v.m128_f32[2]);
        }
    }
    if ((m_flags.mMask & 0x200) != 0)
    {
        rigid_body* v21 = m_chassis_rbinf->m_rb;
        if ((~(v21->m_flags >> 6) & 1) == 0
            && _tlAssert("c:\\cod\\code\\tl\\physics\\include\\rigid_body.h",
                         79, "debug_flag_is_not_in_collision()",
                         defaultFileName))
            __debugbreak();
        sv->mRBVeh = (void*)(int)v21->m_mat.w.v.m128_f32[0];
        rigid_body* v23 = m_chassis_rbinf->m_rb;
        if ((~(v23->m_flags >> 6) & 1) == 0
            && _tlAssert("c:\\cod\\code\\tl\\physics\\include\\rigid_body.h",
                         79, "debug_flag_is_not_in_collision()",
                         defaultFileName))
            __debugbreak();
        sv->mRBVeh = (void*)(int)v23->m_mat.w.v.m128_f32[1];
        rigid_body* v24 = m_chassis_rbinf->m_rb;
        if ((~(v24->m_flags >> 6) & 1) == 0
            && _tlAssert("c:\\cod\\code\\tl\\physics\\include\\rigid_body.h",
                         79, "debug_flag_is_not_in_collision()",
                         defaultFileName))
            __debugbreak();
        sv->mRBVeh = (void*)(int)v24->m_mat.w.v.m128_f32[2];
    }
    if ((m_chassis_rbinf->m_rb->m_flags & 4) != 0)
    {
        float z = _mm_shuffle_ps(m_owner->r.currentMat.z.v,
                                 m_owner->r.currentMat.z.v, 170)
                      .m128_f32[0];
        if (z < 0.34999999f)
            m_owner->Notify(hash_const.flipped);
    }
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
rigid_body* add_entity(Entity* e, float mass, float fric);  // ?add_entity@rb_prop_system@@YAPAVrigid_body@@PAVEntity@@MM@Z
void hit_entity(Entity* e, const math::Position3& hitp,
                const math::Dir3& hitd, float fmag, float tmag);  // ?hit_entity@rb_prop_system@@YAXPAVEntity@@ABVPosition3@math@@ABVDir3@4@MM@Z
static bool frame_advance_flag();
void add_environmental_point_constraint(Entity* e,
                                        const math::Position3& ent_r_loc,
                                        const math::Position3& abs_coord);  // ?add_environmental_point_constraint@rb_prop_system@@YAXPAVEntity@@ABVPosition3@math@@1@Z
void add_environmental_dist_constraint(Entity* e,
                                       const math::Position3& ent_r_loc,
                                       const math::Position3& abs_coord,
                                       float min_dist, float max_dist);  // ?add_environmental_dist_constraint@rb_prop_system@@YAXPAVEntity@@ABVPosition3@math@@1MM@Z
void add_entity_dist_constraint(Entity* e1, const math::Position3& e1_loc_pt,
                                Entity* e2, const math::Position3& e2_loc_pt,
                                float min_dist, float max_dist);  // ?add_entity_dist_constraint@rb_prop_system@@YAXPAVEntity@@ABVPosition3@math@@01MM@Z
void add_entity_point_constraint(Entity* e1,
                                 const math::Position3& e1_loc_pt,
                                 Entity* e2,
                                 const math::Position3& e2_loc_pt);  // ?add_entity_point_constraint@rb_prop_system@@YAXPAVEntity@@ABVPosition3@math@@01@Z
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

// ea: 0x706D10
void rb_prop_system::add_environmental_point_constraint(
    Entity* e, const math::Position3& ent_r_loc,
    const math::Position3& abs_coord)
{
    rigid_body* entity_rb = (rigid_body*)get_entity_rb(e);
    if (entity_rb != nullptr)
    {
        math::Dir3 b2_r_loc;
        math::Dir3 b1_r_loc;
        b2_r_loc.v = abs_coord.v;
        b1_r_loc.v = ent_r_loc.v;
        rigid_body_constraint_point* rbc_point = phys_sys::create_rbc_point(
            entity_rb, phys_sys::get_environment_rigid_body(), false);
        rbc_point->set(b1_r_loc, b2_r_loc);
    }
}

// ea: 0x707010
void rb_prop_system::add_environmental_dist_constraint(
    Entity* e, const math::Position3& ent_r_loc,
    const math::Position3& abs_coord, float min_dist, float max_dist)
{
    rigid_body* entity_rb = (rigid_body*)get_entity_rb(e);
    if (entity_rb != nullptr)
    {
        math::Dir3 b2_r_loc;
        math::Dir3 b1_r_loc;
        b2_r_loc.v = abs_coord.v;
        b1_r_loc.v = ent_r_loc.v;
        rigid_body_constraint_distance* rbc_dist = phys_sys::create_rbc_dist(
            entity_rb, phys_sys::get_environment_rigid_body(), false);
        rbc_dist->set(b1_r_loc, b2_r_loc, min_dist, max_dist);
    }
}

// ea: 0x707080
void rb_prop_system::add_entity_dist_constraint(
    Entity* e1, const math::Position3& e1_loc_pt, Entity* e2,
    const math::Position3& e2_loc_pt, float min_dist, float max_dist)
{
    rigid_body* entity_rb = (rigid_body*)get_entity_rb(e1);
    rigid_body* v8 = (rigid_body*)get_entity_rb(e2);
    if (entity_rb != nullptr && v8 != nullptr)
    {
        math::Dir3 b2_r_loc;
        math::Dir3 b1_r_loc;
        b2_r_loc.v = e2_loc_pt.v;
        b1_r_loc.v = e1_loc_pt.v;
        rigid_body_constraint_distance* rbc_dist =
            phys_sys::create_rbc_dist(entity_rb, v8, false);
        rbc_dist->set(b1_r_loc, b2_r_loc, min_dist, max_dist);
    }
}

// ea: 0x706D80
void rb_prop_system::add_entity_point_constraint(
    Entity* e1, const math::Position3& e1_loc_pt, Entity* e2,
    const math::Position3& e2_loc_pt)
{
    rigid_body* entity_rb = (rigid_body*)get_entity_rb(e1);
    rigid_body* v5 = (rigid_body*)get_entity_rb(e2);
    rigid_body* v24 = v5;
    if (entity_rb != nullptr && v5 != nullptr)
    {
        __m128 v = e1_loc_pt.v;
        math::Dir3 v39;
        v39.v = _mm_add_ps(
            _mm_add_ps(
                _mm_mul_ps(_mm_shuffle_ps(v, v, 0), e1->r.currentMat.x.v),
                _mm_mul_ps(_mm_shuffle_ps(v, v, 85), e1->r.currentMat.y.v)),
            _mm_add_ps(
                _mm_mul_ps(_mm_shuffle_ps(v, v, 170), e1->r.currentMat.z.v),
                e1->r.currentMat.w.v));
        const math::Mat43& mat = entity_rb->get_mat();
        math::Position3 v38;
        v38.v = v39.v;  // toPosition3 (inline 16-byte copy)
        __m128 y = mat.y.v;
        __m128 z = mat.z.v;
        __m128 v11 = _mm_shuffle_ps(mat.x.v, y, 68);
        __m128 v12 = _mm_shuffle_ps(_mm_shuffle_ps(mat.x.v, y, 238), z, 168);
        __m128 v13 = v11;
        __m128 v14 = _mm_shuffle_ps(v11, z, 221);
        __m128 v15 = _mm_shuffle_ps(v13, z, 136);
        __m128 v34 = _mm_xor_ps(
            Float4_SignMask_12,
            _mm_add_ps(
                _mm_add_ps(
                    _mm_mul_ps(_mm_shuffle_ps(mat.w.v, mat.w.v, 0), v15),
                    _mm_mul_ps(_mm_shuffle_ps(mat.w.v, mat.w.v, 85), v14)),
                _mm_mul_ps(_mm_shuffle_ps(mat.w.v, mat.w.v, 170), v12)));
        __m128 v16 = _mm_mul_ps(_mm_shuffle_ps(v38.v, v38.v, 170), v12);
        __m128 v17 = _mm_mul_ps(_mm_shuffle_ps(v38.v, v38.v, 85), v14);
        math::Position3 v18;
        v18.v = e2->r.currentMat.w.v;
        __m128 v19 = _mm_mul_ps(_mm_shuffle_ps(v38.v, v38.v, 0), v15);
        math::Dir3 v20;
        v20.v = e2->r.currentMat.z.v;
        v38.v = v17;
        __m128 v36 = v19;
        __m128 v21 = e2_loc_pt.v;
        __m128 v22 =
            _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(v21, v21, 170), v20.v), v18.v);
        math::Dir3 v23;
        v23.v = e2->r.currentMat.y.v;
        __m128 v37 = v16;
        v39.v = _mm_add_ps(
            _mm_add_ps(
                _mm_mul_ps(_mm_shuffle_ps(v21, v21, 0), e2->r.currentMat.x.v),
                _mm_mul_ps(_mm_shuffle_ps(v21, v21, 85), v23.v)),
            v22);
        const math::Mat43& v25 = v24->get_mat();
        math::Position3 v35;
        v35.v = v39.v;  // toPosition3
        __m128 v27 = v25.y.v;
        __m128 v28 = v25.z.v;
        __m128 v29 = _mm_shuffle_ps(v25.x.v, v27, 68);
        __m128 v30 = _mm_shuffle_ps(v29, v28, 221);
        __m128 v31 =
            _mm_shuffle_ps(_mm_shuffle_ps(v25.x.v, v27, 238), v28, 168);
        __m128 v32 = _mm_shuffle_ps(v29, v28, 136);
        v39.v = _mm_add_ps(
            _mm_add_ps(
                _mm_mul_ps(_mm_shuffle_ps(v35.v, v35.v, 0), v32),
                _mm_mul_ps(_mm_shuffle_ps(v35.v, v35.v, 85), v30)),
            _mm_add_ps(
                _mm_mul_ps(_mm_shuffle_ps(v35.v, v35.v, 170), v31),
                _mm_xor_ps(
                    Float4_SignMask_12,
                    _mm_add_ps(
                        _mm_add_ps(
                            _mm_mul_ps(_mm_shuffle_ps(v25.w.v, v25.w.v, 0),
                                       v32),
                            _mm_mul_ps(_mm_shuffle_ps(v25.w.v, v25.w.v, 85),
                                       v30)),
                        _mm_mul_ps(_mm_shuffle_ps(v25.w.v, v25.w.v, 170),
                                   v31)))));
        v38.v = _mm_add_ps(_mm_add_ps(v36, v38.v), _mm_add_ps(v37, v34));
        rigid_body_constraint_point* rbc_point =
            phys_sys::create_rbc_point(entity_rb, v24, false);
        rbc_point->set(reinterpret_cast<const math::Dir3&>(v38),
                       reinterpret_cast<const math::Dir3&>(v39));
    }
}

// physics.o data globals (RBPropSys.cpp)
float minDim = 8.0f;   // 0xE36B18
float thresh_0 = 27.0f; // 0xE36B14
static const math::Dir3 PHYSICS_GRAVITY_DIRECTION_3_data = {
    _mm_setr_ps(0.0f, 0.0f, -1.0f, 0.0f)
};
const math::Dir3& PHYSICS_GRAVITY_DIRECTION_3 =
    PHYSICS_GRAVITY_DIRECTION_3_data;

// ea: 0x706150
rigid_body* rb_prop_system::add_entity(Entity* e, float mass, float fric)
{
    if ((_fpclass(e->r.currentAngles.v.m128_f32[0]) & 0x297) != 0
        || (_fpclass(e->r.currentAngles.v.m128_f32[1]) & 0x297) != 0
        || (_fpclass(e->r.currentAngles.v.m128_f32[2]) & 0x297) != 0)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\RBPropSys.cpp";
        AeAssert::gCurrentLine = 335;
        AeAssert::gCurrentExpr =
            "!IS_NAN((e->r.currentAngles)[0]) && !IS_NAN((e->r.currentAngles)[1]) && !IS_NAN((e->r.currentAngles)[2])";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid vector"))
            __debugbreak();
    }
    if ((_fpclass(e->r.currentOrigin.v.m128_f32[0]) & 0x297) != 0
        || (_fpclass(e->r.currentOrigin.v.m128_f32[1]) & 0x297) != 0
        || (_fpclass(e->r.currentOrigin.v.m128_f32[2]) & 0x297) != 0)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\RBPropSys.cpp";
        AeAssert::gCurrentLine = 336;
        AeAssert::gCurrentExpr =
            "!IS_NAN((e->r.currentOrigin)[0]) && !IS_NAN((e->r.currentOrigin)[1]) && !IS_NAN((e->r.currentOrigin)[2])";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid vector"))
            __debugbreak();
    }
    if ((_fpclass(mass) & 0x297) != 0)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\RBPropSys.cpp";
        AeAssert::gCurrentLine = 337;
        AeAssert::gCurrentExpr = "!IS_NAN(mass)";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid number!"))
            __debugbreak();
    }
    if ((_fpclass(fric) & 0x297) != 0)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\RBPropSys.cpp";
        AeAssert::gCurrentLine = 338;
        AeAssert::gCurrentExpr = "!IS_NAN(fric)";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid number!"))
            __debugbreak();
    }
    int m_alloc_count = g_list_rb_extra_info.m_alloc_count;
    for (int i = 0; i < g_list_rb_extra_info.m_alloc_count; ++i)
    {
        if ((i < 0 || i >= m_alloc_count)
            && _tlAssert(
                   "c:\\cod\\code\\tl\\physics\\include\\phys_memory_pool_base.inc",
                   178, "i >= 0 && i < m_alloc_count", defaultFileName))
            __debugbreak();
        if (g_list_rb_extra_info.m_alloc_list[i]->m_ent == e)
        {
            AeAssert::gCurrentAuthor = AeAssert::JRS;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\RBPropSys.cpp";
            AeAssert::gCurrentLine = 344;
            AeAssert::gCurrentExpr = "ptr->m_ent != e";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert(
                       "Adding entity to physics that is already in physics"))
                __debugbreak();
        }
        m_alloc_count = g_list_rb_extra_info.m_alloc_count;
    }
    if (e->mDObj == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\RBPropSys.cpp";
        AeAssert::gCurrentLine = 351;
        AeAssert::gCurrentExpr = nullptr;
        if (!AeAssert::IsIgnored()
            && AeAssert::Warning("Physics entity with no DObj."))
            __debugbreak();
        return nullptr;
    }
    DCGSet* bmodel = (DCGSet*)e->r.bmodel;
    if (bmodel == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\RBPropSys.cpp";
        AeAssert::gCurrentLine = 357;
        AeAssert::gCurrentExpr = nullptr;
        if (!AeAssert::IsIgnored()
            && AeAssert::Warning("Physics entity with no collmap"))
            __debugbreak();
        return nullptr;
    }
    unsigned int v42 = *(unsigned int*)((char*)bmodel + 0x4);
    if (v42 > 10)
    {
        AeAssert::gCurrentAuthor = AeAssert::JRS;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\RBPropSys.cpp";
        AeAssert::gCurrentLine = 366;
        AeAssert::gCurrentExpr = nullptr;
        if (!AeAssert::IsIgnored())
        {
            DObj* mDObj = e->mDObj;
            void** model0 = (void**)mDObj->models;
            ValidatePakId((TPakId)(uintptr_t)model0[1]);
            if (AeAssert::Warning(
                    "Model %s has too many brushes (%d). Will not go into physics.",
                    ((XModelLocal*)model0[0])->mStr, v42))
                __debugbreak();
        }
        return nullptr;
    }
    unsigned int clip_i = 0;
    while (clip_i < v42)
    {
        if (clip_i >= *(unsigned int*)((char*)bmodel + 0x4))
        {
            AeAssert::gCurrentAuthor = AeAssert::JSV;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\cgbank.h";
            AeAssert::gCurrentLine = 77;
            AeAssert::gCurrentExpr = "index < size()";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert(defaultFileName))
                __debugbreak();
            if (clip_i >= *(unsigned int*)((char*)bmodel + 0x4)
                && _tlAssert(
                       "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h", 89,
                       "index >= 0 && index < size()", "invalid index"))
                __debugbreak();
        }
        cdl_object_t* obj =
            &((cdl_object_t*)*(void**)((char*)bmodel + 0x8))[clip_i];
        if ((obj->cflags & 0x241) != 0)
            break;
        ++clip_i;
    }
    if (clip_i == v42)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\RBPropSys.cpp";
        AeAssert::gCurrentLine = 379;
        AeAssert::gCurrentExpr = nullptr;
        if (!AeAssert::IsIgnored()
            && AeAssert::Warning("no physics clips in the collmap"))
            __debugbreak();
        return nullptr;
    }
    if (m_alloc_count > g_rb_vehicle_list.m_alloc_count + 25)
    {
        AeAssert::gCurrentAuthor = AeAssert::JRS;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\RBPropSys.cpp";
        AeAssert::gCurrentLine = 385;
        AeAssert::gCurrentExpr = nullptr;
        if (!AeAssert::IsIgnored()
            && AeAssert::Warning("Ran out of rb_infos"))
            __debugbreak();
        return nullptr;
    }
    rb_extra_info* v8 = g_list_rb_extra_info.add(
        true, "phys memory pool add overflow.");
    if (v8 == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::JRS;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\RBPropSys.cpp";
        AeAssert::gCurrentLine = 392;
        AeAssert::gCurrentExpr = nullptr;
        if (!AeAssert::IsIgnored()
            && AeAssert::Warning("Ran out of rb_infos"))
            __debugbreak();
        return nullptr;
    }

    math::Position3 mins, maxs;
    mins.v = ((math::Position3*)((char*)bmodel + 0x30))->v;
    maxs.v = ((math::Position3*)((char*)bmodel + 0x40))->v;
    math::Dir3 dims;
    dims.v = _mm_sub_ps(maxs.v, mins.v);
    dims.v.m128_f32[0] =
        (dims.v.m128_f32[0] < minDim) ? minDim : dims.v.m128_f32[0];
    dims.v.m128_f32[1] =
        (dims.v.m128_f32[1] < minDim) ? minDim : dims.v.m128_f32[1];
    dims.v.m128_f32[2] =
        (dims.v.m128_f32[2] < minDim) ? minDim : dims.v.m128_f32[2];
    math::Position3 center;
    center.v = _mm_mul_ps(_mm_add_ps(maxs.v, mins.v), _mm_set1_ps(0.5f));

    math::Dir3 unit_inertia;
    float volume;
    nuge::calc_box_inertia(&dims, &unit_inertia, &volume);
    __m128 v18 = _mm_mul_ps(dims.v, dims.v);
    float dim2 =
        v18.m128_f32[0] + (v18.m128_f32[1] + v18.m128_f32[2]);

    math::Mat43 transform;
    SetIdentity(transform);
    transform.w.v = _mm_xor_ps(Float4_SignMask_12, center.v);
    float bs_radius = sqrtf(dim2) * 0.5f;
    rigid_body* rb = phys_sys::create_rigid_body(false);
    if (rb == nullptr)
    {
        g_list_rb_extra_info.remove(v8);
        AeAssert::gCurrentAuthor = AeAssert::JRS;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\RBPropSys.cpp";
        AeAssert::gCurrentLine = 431;
        AeAssert::gCurrentExpr = nullptr;
        if (!AeAssert::IsIgnored()
            && AeAssert::Warning("Ran out of rigid bodies."))
            __debugbreak();
        return nullptr;
    }
    v8->set(e, rb, transform, bs_radius, center);

    DObj* mDObj = e->mDObj;
    void* mValue = mDObj->mPhysDataValue;
    TPakId mPakId = (TPakId)mDObj->mPhysDataPakId;
    ValidatePakId(mPakId);
    if (mValue != nullptr)
    {
        ValidatePakId(mPakId);
        mass = ((PhysData*)mValue)->mMass;
        ValidatePakId(mPakId);
        fric = ((PhysData*)mValue)->mFric;
        if (mass > 0.0f && fric >= 0.0f)
            goto have_mass;
        AeAssert::gCurrentAuthor = AeAssert::JRS;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\RBPropSys.cpp";
        AeAssert::gCurrentLine = 451;
        AeAssert::gCurrentExpr = "mass > 0.0f && fric >= 0.0f";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Bad mass or fiction on physData."))
            __debugbreak();
    }
    else
    {
        float bounce;
        CalculatePhysData(e, mass, fric, bounce);
    }
have_mass:
    if (mass < 0.0f)
        mass = 0.1f;
    float fric_coef = 0.5f;
    if (bs_radius <= 40.0f)
        fric_coef = 0.2f;
    if (fric >= 0.0f && fric <= 1.0f)
        fric_coef = fric;

    transform = e->CalcRotTranMat43();
    __m128 v24 = center.v;
    __m128 v25 = _mm_add_ps(
        _mm_mul_ps(_mm_shuffle_ps(v24, v24, 170), transform.z.v),
        transform.w.v);
    __m128 v26 = _mm_mul_ps(_mm_shuffle_ps(v24, v24, 85), transform.y.v);
    __m128 v27 = _mm_mul_ps(_mm_shuffle_ps(v24, v24, 0), transform.x.v);
    __m128 inertia = _mm_mul_ps(unit_inertia.v, _mm_set1_ps(mass / volume));
    float iy = inertia.m128_f32[1];
    float iz = inertia.m128_f32[2];
    float ix = inertia.m128_f32[0];
    transform.w.v = _mm_add_ps(_mm_add_ps(v27, v26), v25);
    // smallest moment
    float smallest = (iz <= ix) ? iz : ix;
    if (iy <= smallest)
        smallest = iy;
    if (smallest < 0.0099999998f)
    {
        if (smallest <= 0.0000099999997f)
        {
            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\RBPropSys.cpp";
            AeAssert::gCurrentLine = 483;
            AeAssert::gCurrentExpr = "smallest_moment > 0.00001f";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert(defaultFileName))
                __debugbreak();
        }
        inertia = _mm_mul_ps(inertia,
                             _mm_set1_ps(0.01f / smallest));
    }
    math::Dir3 zero_tvel, zero_avel;
    zero_tvel.v = _mm_setzero_ps();
    zero_avel.v = _mm_setzero_ps();
    FindInitialVel(e, zero_tvel, zero_avel);
    G_EntUnlink(e);
    if (e->scr_vehicle != nullptr)
        G_VehInitPathPos(
            (vehicle_pathpos_t*)&((scr_vehicle_t*)e->scr_vehicle)->pathPos);
    rb->set(mass, reinterpret_cast<const math::Dir3&>(inertia), transform,
            zero_tvel, zero_avel, fric_coef, 3);
    rb->set_gravity_dir(PHYSICS_GRAVITY_DIRECTION_3);
    rb->m_gravity_multiplier = PHYSICS_GRAVITY_SCALE_1;
    rb->set_max_avel(8.0f);
    e->flags |= 0x400000;
    if (g_in_physics_collision_callback)
        v8->collision_prolog();
    int big_dims = 0;
    for (int j = 0; j < 3; ++j)
        if (dims.v.m128_f32[j] > thresh_0)
            ++big_dims;
    if (big_dims < 2)
    {
        v8->m_priority = 0;
        v8->m_ent->mFlags &= ~1u;
        v8->m_flags.mMask &= ~2u;
        v8->m_flags.mMask &= ~4u;
        v8->m_flags.mMask &= ~8u;
    }
    v8->evaluate_effect_priority();
    SetupConstraints(v8);
    e->Notify(hash_const.physicsstart);
    return rb;
}

// ea: 0x6FE9B0
void rb_prop_system::hit_entity(Entity* e, const math::Position3& hitp,
                                const math::Dir3& hitd, float fmag,
                                float tmag)
{
    if (!rb_prop_system::frame_advance_flag())
    {
        math::Dir3 force;
        force.v = _mm_mul_ps(hitd.v, _mm_set1_ps(fmag));
        int count = g_list_rb_extra_info.m_alloc_count;
        for (int i = 0; i < count; ++i)
        {
            rb_extra_info* inf = g_list_rb_extra_info.m_alloc_list[i];
            if (inf->m_ent == e)
            {
                math::Dir3 pt;
                pt.v = hitp.v;
                inf->m_rb->add_force(force, pt, tmag);
            }
        }
    }
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
    math::Dir3 m_normal;        // +0x40
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
    void* m_first_geom;    // +0x20

    phys_gjk_geom_list() {}  // ??0phys_gjk_geom_list@@QAE@XZ (no-op)
    // ea: 0x6F1EC0 - walk the geometry list, call each geom's virtual
    // comp_aabb (vtable+0x10), union the per-geom AABBs.
    void comp_aabb(const math::Mat43& cg_to_world_xform)
    {
        for (void* g = m_first_geom; g != nullptr;
             g = *(void**)((char*)g + 0x34))
        {
            void** vtable = *(void***)g;
            ((void (*)(void*, const math::Mat43&))vtable[4])(
                g, cg_to_world_xform);
            if (g == m_first_geom)
            {
                m_aabb_mn.v = ((math::Dir3*)((char*)g + 0x10))->v;
                m_aabb_mx.v = ((math::Dir3*)((char*)g + 0x20))->v;
            }
            else
            {
                m_aabb_mn.v = _mm_min_ps(
                    m_aabb_mn.v, ((math::Dir3*)((char*)g + 0x10))->v);
                m_aabb_mx.v = _mm_max_ps(
                    m_aabb_mx.v, ((math::Dir3*)((char*)g + 0x20))->v);
            }
        }
    }
};

class DCGSet;
bool dcg_type_is_brush(DCGSet* dcg, unsigned int index)
{
    (void)dcg; (void)index;
    return false;
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

// ?wheel_surface_type_fric@@... surface friction table (physics.o data
// @ 0xDF812C; float[32] of per-surface friction multipliers)
static const float s_wheel_surface_fric[32] = {
    0.0f, 0.8f, 1.0f, 0.9f, 0.9f, 1.0f, 0.8f, 0.5f,
    1.0f, 1.0f, 0.7f, 0.6f, 0.1f, 1.0f, 0.6f, 0.9f,
    1.0f, 1.0f, 0.7f, 0.4f, 0.4f, 0.9f, 1.0f, 0.5f,
    0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
};

// ea: 0x6FE3A0
void wheel_collision_info::process(rb_extra_info* rb_inf, int wheel_i)
{
    if (m_did_hit)
    {
        if (m_hit_rb == nullptr
            && _tlAssert("c:\\cod\\code\\game\\vehicle_collision.cpp", 30,
                         "m_hit_rb", defaultFileName))
            __debugbreak();
        rigid_body* m_hit_rb = this->m_hit_rb;
        math::Dir3 v27;
        v27.v = _mm_add_ps(
            _mm_mul_ps(m_p0.v, _mm_set1_ps(1.0f - m_t)),
            _mm_mul_ps(m_p1.v, _mm_set1_ps(m_t)));
        if ((m_hit_rb->m_flags & 0x10) == 0)
        {
            if ((m_hit_rb->m_flags & 0x50) == 0
                && _tlAssert(
                       "c:\\cod\\code\\tl\\physics\\include\\rigid_body.h",
                       109, "debug_flag_is_in_collision()", defaultFileName))
                __debugbreak();
            // world -> body-local transform (transpose of m_col_mat)
            math::Mat43 mm = m_hit_rb->m_col_mat;
            math::Dir3 local;
            for (int i = 0; i < 3; ++i)
            {
                float row[4];
                row[0] = mm.x.v.m128_f32[i];
                row[1] = mm.y.v.m128_f32[i];
                row[2] = mm.z.v.m128_f32[i];
                row[3] = mm.w.v.m128_f32[i];
                local.v.m128_f32[i] = row[0] * v27.v.m128_f32[0]
                                      + row[1] * v27.v.m128_f32[1]
                                      + row[2] * v27.v.m128_f32[2] + row[3];
            }
            v27.v = local.v;
            rigid_body* v16 = m_hit_rb;
            if ((v16->m_flags & 0x50) == 0
                && _tlAssert(
                       "c:\\cod\\code\\tl\\physics\\include\\rigid_body.h",
                       109, "debug_flag_is_in_collision()", defaultFileName))
                __debugbreak();
            // rotate m_normal by the hit rb's col_mat
            math::Mat43 m = v16->m_col_mat;
            __m128 n = m_normal.v;
            m_normal.v = _mm_add_ps(
                _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(n, n, 0), m.x.v),
                           _mm_mul_ps(_mm_shuffle_ps(n, n, 85), m.y.v)),
                _mm_mul_ps(_mm_shuffle_ps(n, n, 170), m.z.v));
        }
        rigid_body_constraint_wheel* m_rbc_wheel = this->m_rbc_wheel;
        rigid_body* v25 = m_hit_rb;
        math::Dir3 v26;
        v26.v = m_normal.v;
        m_rbc_wheel->set_collision(v25, &v27, &v26);
        int m_surface_flags = this->m_surface_flags;
        float v21 = 1.0f;
        if (m_surface_flags != 0)
        {
            int v22 = (m_surface_flags >> 20) & 0x1F;
            v21 = s_wheel_surface_fric[v22];
            ((scr_vehicle_t*)rb_inf->m_ent->scr_vehicle)
                ->phys.wheelSurfType[wheel_i] = v22;
        }
        rb_vehicle* m_rb_vehicle = rb_inf->m_rb_vehicle;
        float m_current_fwd_fric_scale =
            m_rb_vehicle->m_current_fwd_fric_scale;
        m_rbc_wheel->m_side_fric_k =
            m_rb_vehicle->m_current_side_fric_scale * v21;
        m_rbc_wheel->m_fwd_fric_k = m_current_fwd_fric_scale * v21;
    }
    else
    {
        ((scr_vehicle_t*)rb_inf->m_ent->scr_vehicle)
            ->phys.wheelSurfType[wheel_i] = 0;
        m_rbc_wheel->set_no_collision();
    }
}

// ea: 0x884FE0
void rigid_body_constraint_wheel::get_wheel_collide_segment(
    const math::Mat43& b1_mat, math::Dir3* const p0,
    math::Dir3* const p1) const
{
    if (((unsigned int)p0 & 0xF) != 0
        && _tlAssert(
               "c:\\cod\\code\\tl\\physics\\include\\phys_math.h", 365,
               "uint(v) % PHYS_ALIGNOF(phys_vec3) == 0", defaultFileName))
        __debugbreak();
    if (((unsigned int)p1 & 0xF) != 0
        && _tlAssert(
               "c:\\cod\\code\\tl\\physics\\include\\phys_math.h", 365,
               "uint(v) % PHYS_ALIGNOF(phys_vec3) == 0", defaultFileName))
        __debugbreak();
    __m128 v5 = b1_mat.z.v;
    __m128 v6 = b1_mat.y.v;
    __m128 v7 = _mm_add_ps(
        _mm_add_ps(
            _mm_mul_ps(
                _mm_shuffle_ps(m_b1_wheel_center_loc.v,
                               m_b1_wheel_center_loc.v, 0),
                b1_mat.x.v),
            _mm_mul_ps(
                _mm_shuffle_ps(m_b1_wheel_center_loc.v,
                               m_b1_wheel_center_loc.v, 85),
                v6)),
        _mm_add_ps(
            _mm_mul_ps(
                _mm_shuffle_ps(m_b1_wheel_center_loc.v,
                               m_b1_wheel_center_loc.v, 170),
                v5),
            b1_mat.w.v));
    __m128 v8 = _mm_mul_ps(
        _mm_add_ps(
            _mm_add_ps(
                _mm_mul_ps(
                    _mm_shuffle_ps(m_b1_suspension_dir_loc.v,
                                   m_b1_suspension_dir_loc.v, 0),
                    b1_mat.x.v),
                _mm_mul_ps(
                    _mm_shuffle_ps(m_b1_suspension_dir_loc.v,
                                   m_b1_suspension_dir_loc.v, 85),
                    v6)),
            _mm_mul_ps(
                _mm_shuffle_ps(m_b1_suspension_dir_loc.v,
                               m_b1_suspension_dir_loc.v, 170),
                v5)),
        _mm_set1_ps(m_wheel_radius));
    p0->v = _mm_sub_ps(v7, v8);
    p1->v = _mm_add_ps(v7, v8);
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

// gjk_geom_database (physics.o; RBPropSys/collision tree). Members with
// verified manglings.
struct gjk_geom_info {
    void* m_geom_id;      // +0x00
    void* m_gjk_geom;     // +0x04
    void* m_avl_left;     // +0x08
    void* m_avl_right;    // +0x0C
    int   m_avl_balance;  // +0x10
};
class CGBank;
// phys_gjk_geom_cod_base (phys_gjk.h view; 64 bytes, IDA ordinal). Base of
// all cod geometry; m_aabb_mn +0x10, m_aabb_mx +0x20, m_geom_id +0x30,
// m_next_geom +0x34, m_dcg +0x38, m_dcg_index +0x3C.
class phys_gjk_geom_cod_base {
public:
    void*         __vftable;   // +0x00
    uint8_t       _pad4[0x10 - 0x04];
    math::Dir3    m_aabb_mn;   // +0x10
    math::Dir3    m_aabb_mx;   // +0x20
    void*         m_geom_id;   // +0x30
    phys_gjk_geom_cod_base* m_next_geom;  // +0x34
    DCGSet*       m_dcg;       // +0x38
    int           m_dcg_index; // +0x3C
};

// phys_gjk_geom_aabb / phys_gjk_geom_vert_list factory stubs
// (?create@phys_gjk_geom_aabb@@SAPAV1@ABVDir3@math@@0@Z /
//  ?create@phys_gjk_geom_vert_list@@SAPAV1@HPAVDCGSet@@H@Z; phys_xboxr
// phys_gjk.o helpers, port later).
class phys_gjk_geom_aabb : public phys_gjk_geom_cod_base {
public:
    math::Dir3 m_center_local;  // +0x40
    math::Dir3 m_dims;          // +0x50
    phys_gjk_geom_aabb(const math::Dir3& center,
                       const math::Dir3& dims);  // ??0phys_gjk_geom_aabb@@QAE@ABVDir3@math@@0@Z
    static phys_gjk_geom_aabb* create(const math::Dir3& center,
                                      const math::Dir3& dims);
};
class phys_gjk_geom_vert_list : public phys_gjk_geom_cod_base {
public:
    math::Dir3* m_vert_list;  // +0x40
    int         m_vert_list_count;  // +0x44
    static phys_gjk_geom_vert_list* create(int num_verts, DCGSet* dcg,
                                           int dcg_index);
};
// ea: 0x71BD70
phys_gjk_geom_aabb::phys_gjk_geom_aabb(const math::Dir3& center,
                                      const math::Dir3& dims)
{
    m_center_local.v = center.v;
    m_dims.v = dims.v;
    m_dcg = nullptr;
    m_dcg_index = -1;
}
// ea: 0x719260
phys_gjk_geom_aabb* phys_gjk_geom_aabb::create(const math::Dir3& center,
                                               const math::Dir3& dims)
{
    void* mem = g_collision_memory_allocater.allocate(
        0x60, 16, false, "phys_collision_allocater overflow.");
    if (mem == nullptr)
        return nullptr;
    return new (mem) phys_gjk_geom_aabb(center, dims);
}
// ea: 0x71C200
phys_gjk_geom_vert_list* phys_gjk_geom_vert_list::create(int num_verts,
                                                         DCGSet* dcg,
                                                         int dcg_index)
{
    if (num_verts <= 0)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\RBCollision.h";
        AeAssert::gCurrentLine = 0xE9;
        AeAssert::gCurrentExpr = "num_verts > 0";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("physics object with no vertex data"))
            __debugbreak();
    }
    phys_gjk_geom_vert_list* result =
        (phys_gjk_geom_vert_list*)g_collision_memory_allocater.allocate(
            0x50, 16, false, "phys_collision_allocater overflow.");
    if (result == nullptr)
        return nullptr;
    result->m_vert_list =
        (math::Dir3*)g_collision_memory_allocater.allocate(
            num_verts * 16, 16, false, "phys_collision_allocater overflow.");
    if (result->m_vert_list == nullptr)
        return nullptr;
    result->m_vert_list_count = num_verts;
    result->m_dcg = dcg;
    result->m_dcg_index = dcg_index;
    return result;
}
// ea: 0x719260 (phys_gjk_geom_list::create; ctor is a no-op)
void* phys_gjk_geom_list_create()
{
    return g_collision_memory_allocater.allocate(
        0x30, 16, false, "phys_collision_allocater overflow.");
}

// cdl_vinfo_t / cdl_array<vi4> minimal views for unpack
struct cdl_vinfo_t {
    float vbase[3];       // +0x00 (x/y/z)
    int   num_verts;      // +0x0C
    int   first_vert;     // +0x10
};
struct vi4 {
    int v;  // +0x00
};
template <typename T>
class cdl_array {
public:
    int   m_count;      // +0x00
    T*    m_elements;   // +0x04
};

// ea: 0x6FF680
void unpack(const cdl_vinfo_t& vinfo, const cdl_array<vi4>& verts,
            math::Dir3* vert_list)
{
    math::Position3 base;
    base.v = _mm_setr_ps(vinfo.vbase[0], vinfo.vbase[1], vinfo.vbase[2],
                         0.0f);
    int num_verts = vinfo.num_verts;
    unsigned int first_vert = vinfo.first_vert;
    if (first_vert >= (unsigned int)verts.m_count
        && _tlAssert("c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h", 89,
                     "index >= 0 && index < size()", "invalid index"))
        __debugbreak();
    vi4* v5 = &verts.m_elements[first_vert];
    if (num_verts != 0)
    {
        do
        {
            math::Position3 q;
            q.v = _mm_setr_ps((float)((v5->v & 0x7FF) * 0.25f),
                              (float)(((v5->v >> 11) & 0x7FF) * 0.25f),
                              (float)((v5->v >> 22) * 0.25f), 0.0f);
            vert_list->v = _mm_add_ps(base.v, q.v);
            ++vert_list;
            ++v5;
            --num_verts;
        } while (num_verts != 0);
    }
}
unsigned int make_unique_id(Entity* ent, unsigned int object_id)
{
    return (unsigned int)(uintptr_t)ent + object_id;
}

// ea: 0x6FEDB0
class gjk_geom_database {
public:
    gjk_geom_info* m_tree_root;  // +0x00 (m_ggi_search_tree.m_tree_root)
    int   m_terrain_count;  // +0x04
    int   m_entity_count;   // +0x08
    int   m_actor_count;    // +0x0C
    int   m_patch_count;    // +0x10
    int   m_brush_count;    // +0x14
    int   m_aabb_count;     // +0x18

    phys_gjk_geom_list* create_gjk_geom(Entity* ent);  // ?create_gjk_geom@gjk_geom_database@@QAEPAVphys_gjk_geom_list@@PAVEntity@@@Z
    phys_gjk_geom_cod_base* create_gjk_geom(const cdl_object_t* obj, int index,
                                            CGBank* bank);  // ?create_gjk_geom@gjk_geom_database@@QAEPAVphys_gjk_geom_cod_base@@PBUcdl_object_t@@HPAVCGBank@@@Z
    phys_gjk_geom_cod_base* create_actor_gjk_geom(Entity* ent);  // ?create_actor_gjk_geom@gjk_geom_database@@QAEPAVphys_gjk_geom_cod_base@@PAVEntity@@@Z
    gjk_geom_info* add_to_sorted_list(void* gjk_geom, unsigned int geom_id);  // ?add_to_sorted_list@gjk_geom_database@@QAEPAUgjk_geom_info@1@PAXI@Z
    phys_gjk_geom_cod_base* try_get_gjk_geom(const cdl_object_t* obj, int index,
                                             CGBank* bank);  // ?try_get_gjk_geom@gjk_geom_database@@QAEPAVphys_gjk_geom_cod_base@@PBUcdl_object_t@@HPAVCGBank@@@Z
    phys_gjk_geom_list* try_get_gjk_geom(Entity* ent, const math::Mat43* cg_to_world_xform);  // ?try_get_gjk_geom@gjk_geom_database@@QAEPAVphys_gjk_geom_list@@PAVEntity@@PBVMat43@math@@@Z
    phys_gjk_geom_cod_base* try_get_actor_gjk_geom(Entity* ent, const math::Mat43* cg_to_world_xform);  // ?try_get_actor_gjk_geom@gjk_geom_database@@QAEPAVphys_gjk_geom_cod_base@@PAVEntity@@PBVMat43@math@@@Z
    phys_gjk_geom_list* get_gjk_geom(Entity* ent, const math::Mat43* cg_to_world_xform);  // ?get_gjk_geom@gjk_geom_database@@QAEPAVphys_gjk_geom_list@@PAVEntity@@PBVMat43@math@@@Z
    phys_gjk_geom_cod_base* get_gjk_geom(const cdl_object_t* obj, int index,
                                         CGBank* bank);  // ?get_gjk_geom@gjk_geom_database@@QAEPAVphys_gjk_geom_cod_base@@PBUcdl_object_t@@HPAVCGBank@@@Z
    phys_gjk_geom_cod_base* get_actor_gjk_geom(Entity* ent, const math::Mat43* cg_to_world_xform);  // ?get_actor_gjk_geom@gjk_geom_database@@QAEPAVphys_gjk_geom_cod_base@@PAVEntity@@PBVMat43@math@@@Z
};
// ?g_gjk_geom_database@@3PAUgjk_geom_database@@A (physics.o data @ 0xF79488)
gjk_geom_database* g_gjk_geom_database = nullptr;

void comp_aabb_stub(void* geom, const math::Mat43* xform)
{
    (void)geom;
    (void)xform;
}

// ea: 0x705F90
phys_gjk_geom_list* rb_extra_info::try_collision_prolog()
{
    if (m_ent == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\RBPropSys.cpp";
        AeAssert::gCurrentLine = 149;
        AeAssert::gCurrentExpr = "m_ent";
        if (!AeAssert::IsIgnored() && AeAssert::Assert(defaultFileName))
            __debugbreak();
    }
    rigid_body* m_rb = this->m_rb;
    if ((m_rb->m_flags & 0x50) == 0
        && _tlAssert(
               "c:\\cod\\code\\tl\\physics\\include\\rigid_body.h", 109,
               "debug_flag_is_in_collision()", defaultFileName))
        __debugbreak();
    phys_full_multiply_mat(*m_cg_mesh_mat, m_rb->m_col_mat, m_transform);
    if (m_gjk_geom_list != nullptr
        && _tlAssert("c:\\cod\\code\\game\\RBPropSys.cpp", 153,
                     "m_gjk_geom_list == NULL", defaultFileName))
        __debugbreak();
    if (m_ent == nullptr
        && _tlAssert("c:\\cod\\code\\game\\RBPropSys.cpp", 154, "m_ent",
                     defaultFileName))
        __debugbreak();
    phys_gjk_geom_list* gjk_geom =
        g_gjk_geom_database->try_get_gjk_geom(m_ent, nullptr);
    m_gjk_geom_list = gjk_geom;
    if (gjk_geom == nullptr)
        return nullptr;
    gjk_geom->comp_aabb(*m_cg_mesh_mat);
    rb_vehicle* m_rb_vehicle = this->m_rb_vehicle;
    if (m_rb_vehicle == nullptr)
        return (phys_gjk_geom_list*)m_gjk_geom_list;
    if (m_rb_vehicle->m_vci != nullptr
        && _tlAssert("c:\\cod\\code\\game\\RBPropSys.cpp", 160,
                     "m_rb_vehicle->m_vci == NULL", defaultFileName))
        __debugbreak();
    create_vehicle_collision_info(this);
    if (m_rb_vehicle->m_vci != nullptr)
        return (phys_gjk_geom_list*)m_gjk_geom_list;
    return nullptr;
}

// ea: 0x703DC0
gjk_geom_info* gjk_geom_database::add_to_sorted_list(void* gjk_geom,
                                                     unsigned int geom_id)
{
    gjk_geom_info* result = (gjk_geom_info*)
        g_collision_memory_allocater.allocate(
            20, 4, false, "phys_collision_allocater overflow.");
    gjk_geom_info* v5 = result;
    if (result != nullptr)
    {
        result->m_geom_id = (void*)(uintptr_t)geom_id;
        result->m_gjk_geom = gjk_geom;
        // phys_inplace_avl_tree<unsigned int, gjk_geom_info>::add
        // (stub: append as root leaf)
        result->m_avl_left = nullptr;
        result->m_avl_right = nullptr;
        result->m_avl_balance = 0;
        if (m_tree_root == nullptr)
            m_tree_root = result;
        else
        {
            gjk_geom_info* n = m_tree_root;
            for (;;)
            {
                if ((uintptr_t)geom_id < (uintptr_t)n->m_geom_id)
                {
                    if (n->m_avl_left == nullptr)
                    {
                        n->m_avl_left = result;
                        break;
                    }
                    n = (gjk_geom_info*)n->m_avl_left;
                }
                else
                {
                    if (n->m_avl_right == nullptr)
                    {
                        n->m_avl_right = result;
                        break;
                    }
                    n = (gjk_geom_info*)n->m_avl_right;
                }
            }
        }
    }
    return v5;
}

phys_gjk_geom_list* gjk_geom_database::create_gjk_geom(Entity* ent)
{
    if (ent == nullptr
        && _tlAssert("c:\\cod\\code\\game\\RBCollision.cpp", 165, "ent",
                     defaultFileName))
        __debugbreak();
    DCGSet* bmodel = (DCGSet*)ent->r.bmodel;
    if (bmodel == nullptr
        && _tlAssert("c:\\cod\\code\\game\\RBCollision.cpp", 167, "dcg",
                     defaultFileName))
        __debugbreak();
    unsigned int m_count = bmodel->objects_m_count;
    ++m_entity_count;
    phys_gjk_geom_list* result =
        (phys_gjk_geom_list*)phys_gjk_geom_list_create();
    phys_gjk_geom_list* v20 = result;
    if (result == nullptr)
        return nullptr;
    result->m_first_geom = nullptr;
    void* gjk_geom_list = nullptr;
    if (m_count != 0)
    {
        do
        {
            unsigned int v7 = (unsigned int)gjk_geom_list;
            if (v7 >= bmodel->objects_m_count)
            {
                AeAssert::gCurrentAuthor = AeAssert::JSV;
                AeAssert::gCurrentFile = "c:\\cod\\code\\game\\cgbank.h";
                AeAssert::gCurrentLine = 77;
                AeAssert::gCurrentExpr = "index < size()";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert(defaultFileName))
                    __debugbreak();
                if (v7 >= bmodel->objects_m_count
                    && _tlAssert(
                           "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h", 89,
                           "index >= 0 && index < size()", "invalid index"))
                    __debugbreak();
            }
            cdl_object_t* objarr =
                (cdl_object_t*)bmodel->objects_m_elements;
            cdl_object_t* obj = &objarr[v7];
            if ((obj->cflags & 0x241) != 0)
            {
                void* v12;
                if (dcg_type_is_brush(bmodel, v7))
                {
                    ++m_brush_count;
                    // brush: vert-list geometry
                    v12 = phys_gjk_geom_vert_list::create(0, bmodel, (int)v7);
                }
                else
                {
                    ++m_aabb_count;
                    math::Dir3 center, dims;
                    center.v = _mm_setr_ps(obj->center[0], obj->center[1],
                                           obj->center[2], 0.0f);
                    dims.v = _mm_setr_ps(obj->box_radius[0],
                                         obj->box_radius[1],
                                         obj->box_radius[2], 0.0f);
                    v12 = phys_gjk_geom_aabb::create(center, dims);
                }
                if (v12 == nullptr)
                    return nullptr;
                ((phys_gjk_geom_cod_base*)v12)->m_geom_id =
                    (void*)(uintptr_t)make_unique_id(
                        ent, (unsigned int)gjk_geom_list);
                ((phys_gjk_geom_cod_base*)v12)->m_next_geom =
                    (phys_gjk_geom_cod_base*)v20->m_first_geom;
                v20->m_first_geom = v12;
            }
            gjk_geom_list = (char*)gjk_geom_list + 1;
        } while ((unsigned int)gjk_geom_list < m_count);
    }
    return v20;
}

// ea: 0x6FF020
phys_gjk_geom_cod_base* gjk_geom_database::create_actor_gjk_geom(Entity* ent)
{
    ++m_actor_count;
    ++m_aabb_count;
    math::Position3 absmin = ent->r.absmin;
    math::Position3 absmax = ent->r.absmax;
    math::Dir3 center, dims;
    center.v = _mm_mul_ps(_mm_add_ps(absmin.v, absmax.v),
                          _mm_set1_ps(0.5f));
    dims.v = _mm_sub_ps(absmax.v, center.v);
    phys_gjk_geom_cod_base* result =
        phys_gjk_geom_aabb::create(center, dims);
    if (result != nullptr)
    {
        result->m_geom_id = ent;
        result->m_next_geom = nullptr;
    }
    return result;
}

// ea: 0x703E70
phys_gjk_geom_list* gjk_geom_database::try_get_gjk_geom(
    Entity* ent, const math::Mat43* cg_to_world_xform)
{
    gjk_geom_info* i = m_tree_root;
    for (; i != nullptr;
         i = ent >= i->m_geom_id
                 ? (gjk_geom_info*)i->m_avl_right
                 : (gjk_geom_info*)i->m_avl_left)
    {
        if (ent == i->m_geom_id)
            break;
    }
    gjk_geom_info* v6 = i;
    if (i == nullptr)
    {
        phys_gjk_geom_list* gjk_geom = create_gjk_geom(ent);
        phys_gjk_geom_list* v8 = gjk_geom;
        if (gjk_geom == nullptr)
            return nullptr;
        v6 = add_to_sorted_list(gjk_geom, (unsigned int)(uintptr_t)ent);
        if (v6 == nullptr)
            return nullptr;
        if (cg_to_world_xform != nullptr)
            comp_aabb_stub(v8, cg_to_world_xform);
    }
    return (phys_gjk_geom_list*)v6->m_gjk_geom;
}

// ea: 0x703EF0
phys_gjk_geom_cod_base* gjk_geom_database::try_get_actor_gjk_geom(
    Entity* ent, const math::Mat43* cg_to_world_xform)
{
    gjk_geom_info* i = m_tree_root;
    for (; i != nullptr;
         i = ent >= i->m_geom_id
                 ? (gjk_geom_info*)i->m_avl_right
                 : (gjk_geom_info*)i->m_avl_left)
    {
        if (ent == i->m_geom_id)
            break;
    }
    gjk_geom_info* v6 = i;
    if (i == nullptr)
    {
        phys_gjk_geom_cod_base* actor_gjk_geom = create_actor_gjk_geom(ent);
        phys_gjk_geom_cod_base* v8 = actor_gjk_geom;
        if (actor_gjk_geom == nullptr)
            return nullptr;
        v6 = add_to_sorted_list(actor_gjk_geom,
                                (unsigned int)(uintptr_t)ent);
        if (v6 == nullptr)
            return nullptr;
        if (cg_to_world_xform != nullptr)
            comp_aabb_stub(v8, cg_to_world_xform);
    }
    return (phys_gjk_geom_cod_base*)v6->m_gjk_geom;
}

// ea: 0x703FE0
phys_gjk_geom_list* gjk_geom_database::get_gjk_geom(
    Entity* ent, const math::Mat43* cg_to_world_xform)
{
    phys_gjk_geom_list* result = try_get_gjk_geom(ent, cg_to_world_xform);
    if (result == nullptr)
    {
        phys_collision_allocater_ballistic_reinit();
        result = try_get_gjk_geom(ent, cg_to_world_xform);
        if (result == nullptr)
        {
            if (!_tlAssert("c:\\cod\\code\\game\\RBCollision.cpp", 296,
                           "gjk_geom_list", defaultFileName))
                __debugbreak();
            result = nullptr;
        }
    }
    return result;
}

// ea: 0x704040
phys_gjk_geom_cod_base* gjk_geom_database::get_actor_gjk_geom(
    Entity* ent, const math::Mat43* cg_to_world_xform)
{
    phys_gjk_geom_cod_base* result =
        try_get_actor_gjk_geom(ent, cg_to_world_xform);
    if (result == nullptr)
    {
        phys_collision_allocater_ballistic_reinit();
        result = try_get_actor_gjk_geom(ent, cg_to_world_xform);
        if (result == nullptr)
        {
            if (!_tlAssert("c:\\cod\\code\\game\\RBCollision.cpp", 308,
                           "gjk_geom", defaultFileName))
                __debugbreak();
            result = nullptr;
        }
    }
    return result;
}

// ?g_in_physics_collision_callback@@3_NA (physics.o data @ 0xF79475)
bool g_in_physics_collision_callback = false;
// ?gPhysicsFrameAdvance@@3_NA (physics.o data @ 0xF79474)
bool gPhysicsFrameAdvance = false;
namespace rb_prop_system {
// lookup helper for the global (used by hit_entity)
static bool frame_advance_flag()
{
    return ::gPhysicsFrameAdvance;
}
}
// ?g_physics_memory_buffer@@3PADA (physics.o data @ 0xF79478)
char* g_physics_memory_buffer = nullptr;

// ea: 0x6F1950
void phys_gjk_cache_info::update_swapped(bool swapped)
{
    unsigned int m_flags = this->m_flags;
    if (swapped != ((m_flags & 2) != 0))
    {
        unsigned int v3 = swapped ? (m_flags | 2) : (m_flags & 0xFFFFFFFD);
        m_flags = v3;
        if ((v3 & 4) != 0)
            m_support_dir.v = _mm_xor_ps(Float4_SignMask_12, m_support_dir.v);
        if ((v3 & 8) != 0)
        {
            int v4 = 0;
            if (m_support_count > 0)
            {
                math::Dir3* m_support_b = this->m_support_b;
                do
                {
                    __m128 temp = m_support_b[-3].v;
                    m_support_b[-3].v = m_support_b->v;
                    m_support_b->v = temp;
                    ++v4;
                    ++m_support_b;
                } while (v4 < m_support_count);
            }
        }
    }
}

// phys_gjk_cache_info_internal (physics.o; phys_gjk_cache_info + AVL node,
// 144 bytes)
struct phys_gjk_cache_info_internal : phys_gjk_cache_info {
    struct avl_tree_node {
        phys_gjk_cache_info_internal* m_left;    // +0x00
        phys_gjk_cache_info_internal* m_right;   // +0x04
        int                            m_balance; // +0x08
    } m_avl_tree_node;  // +0x80
};
static_assert(sizeof(phys_gjk_cache_info_internal) == 0x90,
              "phys_gjk_cache_info_internal size mismatch");

// phys_gjk_cache_system_avl_tree<N> (physics.o collision/gjk cache system).
// Binary layout: m_list_phys_gjk_cache_info_internal (pool, 0x128F0 bytes for
// N=500) + m_search_tree (phys_inplace_avl_tree root, +0x128F0).
// find/add/remove are the phys_avl_tree.h AVL instantiations
// phys_inplace_avl_tree<phys_gjk_geom_id_pair_key,
//   phys_gjk_cache_info_internal>::find/add/remove (0x71B6A0/0x71CE70/0x71D2F0).
template <int N>
class phys_gjk_cache_system_avl_tree {
public:
    struct stack_item {
        phys_gjk_cache_info_internal** m_node;
        int                            m_child;
    };

    phys_static_memory_pool<phys_gjk_cache_info_internal, N>
        m_list_phys_gjk_cache_info_internal;  // +0x00
    phys_gjk_cache_info_internal*
        m_search_tree_root;  // +0x128F0 (phys_inplace_avl_tree m_tree_root)

    static bool key_lt(const phys_gjk_geom_id_pair_key& a,
                       const phys_gjk_geom_id_pair_key& b)
    {
        if (a.m_id1 != b.m_id1)
            return a.m_id1 < b.m_id1;
        return a.m_id2 < b.m_id2;
    }

    // find - ea: 0x71B6A0
    phys_gjk_cache_info_internal* find(const phys_gjk_geom_id_pair_key& key)
    {
        phys_gjk_cache_info_internal* result = m_search_tree_root;
        if (m_search_tree_root != nullptr)
        {
            unsigned int m_id1 = key.m_id1;
            do
            {
                if (m_id1 == result->m_key.m_id1
                    && key.m_id2 == result->m_key.m_id2)
                    break;
                unsigned int v4 = result->m_key.m_id1;
                bool v5 = m_id1 < v4;
                if (m_id1 == v4)
                    v5 = key.m_id2 < result->m_key.m_id2;
                result = v5 ? result->m_avl_tree_node.m_left
                            : result->m_avl_tree_node.m_right;
            } while (result != nullptr);
        }
        return result;
    }

    // add - ea: 0x71CE70
    void add(const phys_gjk_geom_id_pair_key& key,
             phys_gjk_cache_info_internal* data)
    {
        stack_item the_stack[32];
        stack_item* cur = the_stack;
        the_stack[0].m_node = &m_search_tree_root;
        bool done = (m_search_tree_root == nullptr);
        for (; !done; ++cur)
        {
            phys_gjk_cache_info_internal* node = *cur->m_node;
            if (((cur - the_stack + 8) & 0xFFFFFFF8) >= 256
                && _tlAssert(
                       "c:\\cod\\code\\tl\\physics\\include\\phys_avl_tree.h",
                       586, "cur_item + 1 - the_stack < 32", defaultFileName))
                __debugbreak();
            if (key_lt(key, node->m_key))
            {
                cur->m_child = -1;
                cur[1].m_node = &node->m_avl_tree_node.m_left;
            }
            else
            {
                if (!key_lt(node->m_key, key)
                    && _tlAssert(
                           "c:\\cod\\code\\tl\\physics\\include\\phys_avl_tree.h",
                           595, "key > root->get_avl_key()",
                           defaultFileName))
                    __debugbreak();
                cur->m_child = 1;
                cur[1].m_node = &node->m_avl_tree_node.m_right;
            }
            done = (*cur[1].m_node == nullptr);
        }
        *cur->m_node = data;
        data->m_avl_tree_node.m_left = nullptr;
        data->m_avl_tree_node.m_right = nullptr;
        data->m_avl_tree_node.m_balance = 0;
        if (cur > the_stack)
        {
            phys_gjk_cache_info_internal** m_node;
            do
            {
                m_node = cur[-1].m_node;
                int m_child = cur[-1].m_child;
                --cur;
                (*m_node)->m_avl_tree_node.m_balance += m_child;
                phys_gjk_cache_info_internal* root = *m_node;
                int m_balance = root->m_avl_tree_node.m_balance;
                if (m_balance == -2)
                {
                    phys_gjk_cache_info_internal* m_left =
                        root->m_avl_tree_node.m_left;
                    if (m_left->m_avl_tree_node.m_balance == 1)
                    {
                        phys_gjk_cache_info_internal* m_right =
                            m_left->m_avl_tree_node.m_right;
                        root->m_avl_tree_node.m_left
                            ->m_avl_tree_node.m_right =
                            m_right->m_avl_tree_node.m_left;
                        int v20 = m_right->m_avl_tree_node.m_balance;
                        m_right->m_avl_tree_node.m_left =
                            root->m_avl_tree_node.m_left;
                        root->m_avl_tree_node.m_left
                            ->m_avl_tree_node.m_balance +=
                            v20 <= 0 ? 1 : v20 + 1;
                        int v21 = -root->m_avl_tree_node.m_left
                                       ->m_avl_tree_node.m_balance;
                        m_right->m_avl_tree_node.m_balance +=
                            (v21 < 0
                             || root->m_avl_tree_node.m_left
                                        ->m_avl_tree_node.m_balance
                                        == 0)
                                ? 1
                                : v21 + 1;
                        root->m_avl_tree_node.m_left = m_right;
                    }
                    phys_gjk_cache_info_internal* v22 =
                        root->m_avl_tree_node.m_left;
                    root->m_avl_tree_node.m_left =
                        v22->m_avl_tree_node.m_right;
                    v22->m_avl_tree_node.m_right = root;
                    root->m_avl_tree_node.m_balance +=
                        ((-v22->m_avl_tree_node.m_balance < 0
                          || v22->m_avl_tree_node.m_balance == 0)
                             ? -v22->m_avl_tree_node.m_balance + 1
                             : 1);
                    v22->m_avl_tree_node.m_balance +=
                        root->m_avl_tree_node.m_balance <= 0
                            ? 1
                            : root->m_avl_tree_node.m_balance + 1;
                    *m_node = v22;
                    if (v22->m_avl_tree_node.m_balance == 0)
                        continue;
                    if (_tlAssert(
                            "c:\\cod\\code\\tl\\physics\\include\\phys_avl_tree.h",
                            617, "GAVLN(root)->m_balance == 0",
                            defaultFileName))
                        __debugbreak();
                }
                else if (m_balance == 2)
                {
                    phys_gjk_cache_info_internal* v24 =
                        root->m_avl_tree_node.m_right;
                    if (v24->m_avl_tree_node.m_balance == -1)
                    {
                        phys_gjk_cache_info_internal* v25 =
                            v24->m_avl_tree_node.m_left;
                        root->m_avl_tree_node.m_right
                            ->m_avl_tree_node.m_left =
                            v25->m_avl_tree_node.m_right;
                        int v26 = v25->m_avl_tree_node.m_balance;
                        v25->m_avl_tree_node.m_right =
                            root->m_avl_tree_node.m_right;
                        root->m_avl_tree_node.m_right
                            ->m_avl_tree_node.m_balance +=
                            v26 >= 0 ? 1 : -v26 + 1;
                        v25->m_avl_tree_node.m_balance +=
                            root->m_avl_tree_node.m_right
                                        ->m_avl_tree_node.m_balance
                                    <= 0
                                ? 1
                                : root->m_avl_tree_node.m_right
                                          ->m_avl_tree_node.m_balance
                                      + 1;
                        root->m_avl_tree_node.m_right = v25;
                    }
                    phys_gjk_cache_info_internal* v27 =
                        root->m_avl_tree_node.m_right;
                    root->m_avl_tree_node.m_right =
                        v27->m_avl_tree_node.m_left;
                    v27->m_avl_tree_node.m_left = root;
                    root->m_avl_tree_node.m_balance +=
                        v27->m_avl_tree_node.m_balance <= 0
                            ? 1
                            : v27->m_avl_tree_node.m_balance + 1;
                    v27->m_avl_tree_node.m_balance +=
                        (-root->m_avl_tree_node.m_balance < 0
                         || root->m_avl_tree_node.m_balance == 0)
                            ? 1
                            : -root->m_avl_tree_node.m_balance + 1;
                    *m_node = v27;
                    if (v27->m_avl_tree_node.m_balance == 0)
                        continue;
                    if (_tlAssert(
                            "c:\\cod\\code\\tl\\physics\\include\\phys_avl_tree.h",
                            625, "GAVLN(root)->m_balance == 0",
                            defaultFileName))
                        __debugbreak();
                }
                else
                {
                    continue;
                }
            } while ((*m_node)->m_avl_tree_node.m_balance != 0
                     && cur > the_stack);
        }
    }

    // remove - ea: 0x71D2F0
    void remove(const phys_gjk_geom_id_pair_key& key)
    {
        stack_item the_stack[32];
        stack_item* cur = the_stack;
        the_stack[0].m_node = &m_search_tree_root;
        while (1)
        {
            phys_gjk_cache_info_internal* node = *cur->m_node;
            if (node == nullptr
                && _tlAssert(
                       "c:\\cod\\code\\tl\\physics\\include\\phys_avl_tree.h",
                       644, "root", defaultFileName))
                __debugbreak();
            if (((cur - the_stack + 8) & 0xFFFFFFF8) >= 256
                && _tlAssert(
                       "c:\\cod\\code\\tl\\physics\\include\\phys_avl_tree.h",
                       645, "cur_item + 1 - the_stack < 32",
                       defaultFileName))
                __debugbreak();
            if (!key_lt(key, node->m_key))
                break;
            cur->m_child = -1;
            cur[1].m_node = &node->m_avl_tree_node.m_left;
            ++cur;
        }
        phys_gjk_cache_info_internal* node = *cur->m_node;
        if (key_lt(node->m_key, key))
        {
            cur->m_child = 1;
            cur[1].m_node = &node->m_avl_tree_node.m_right;
            ++cur;
        }
        phys_gjk_cache_info_internal* found = *cur->m_node;
        if ((key.m_id1 != found->m_key.m_id1
             || key.m_id2 != found->m_key.m_id2)
            && _tlAssert(
                   "c:\\cod\\code\\tl\\physics\\include\\phys_avl_tree.h", 659,
                   "key == root->get_avl_key()", defaultFileName))
            __debugbreak();
        phys_gjk_cache_info_internal** v9 = cur->m_node;
        phys_gjk_cache_info_internal* v10 = *cur->m_node;
        stack_item* right_item = nullptr;
        if (v10->m_avl_tree_node.m_right != nullptr)
        {
            cur->m_child = 1;
            ++cur;
            cur->m_node = &v10->m_avl_tree_node.m_right;
            right_item = cur;
            while ((*cur->m_node)->m_avl_tree_node.m_left != nullptr)
            {
                if (((cur - the_stack + 8) & 0xFFFFFFF8) >= 256
                    && _tlAssert(
                           "c:\\cod\\code\\tl\\physics\\include\\phys_avl_tree.h",
                           674, "cur_item + 1 - the_stack < 32",
                           defaultFileName))
                    __debugbreak();
                cur->m_child = -1;
                cur[1].m_node =
                    &(*cur->m_node)->m_avl_tree_node.m_left;
                ++cur;
            }
            phys_gjk_cache_info_internal* m_left = *cur->m_node;
            *cur->m_node = m_left->m_avl_tree_node.m_right;
            m_left->m_avl_tree_node.m_left =
                (*v9)->m_avl_tree_node.m_left;
            m_left->m_avl_tree_node.m_right =
                (*v9)->m_avl_tree_node.m_right;
            m_left->m_avl_tree_node.m_balance =
                (*v9)->m_avl_tree_node.m_balance;
            right_item->m_node = &m_left->m_avl_tree_node.m_right;
            *v9 = m_left;
        }
        else
        {
            *v9 = v10->m_avl_tree_node.m_left;
        }
        if (cur > the_stack)
        {
            phys_gjk_cache_info_internal** v15;
            do
            {
                v15 = cur[-1].m_node;
                int m_child = cur[-1].m_child;
                --cur;
                (*v15)->m_avl_tree_node.m_balance -= m_child;
                phys_gjk_cache_info_internal* root = *v15;
                int m_balance = root->m_avl_tree_node.m_balance;
                if (m_balance == -2)
                {
                    phys_gjk_cache_info_internal* v19 =
                        root->m_avl_tree_node.m_left;
                    if (v19->m_avl_tree_node.m_balance == 1)
                    {
                        phys_gjk_cache_info_internal* m_right =
                            v19->m_avl_tree_node.m_right;
                        root->m_avl_tree_node.m_left
                            ->m_avl_tree_node.m_right =
                            m_right->m_avl_tree_node.m_left;
                        int v21 = m_right->m_avl_tree_node.m_balance;
                        m_right->m_avl_tree_node.m_left =
                            root->m_avl_tree_node.m_left;
                        root->m_avl_tree_node.m_left
                            ->m_avl_tree_node.m_balance +=
                            v21 <= 0 ? 1 : v21 + 1;
                        int v22 = -root->m_avl_tree_node.m_left
                                       ->m_avl_tree_node.m_balance;
                        m_right->m_avl_tree_node.m_balance +=
                            (v22 < 0
                             || root->m_avl_tree_node.m_left
                                        ->m_avl_tree_node.m_balance
                                        == 0)
                                ? 1
                                : v22 + 1;
                        root->m_avl_tree_node.m_left = m_right;
                    }
                    phys_gjk_cache_info_internal* v23 =
                        root->m_avl_tree_node.m_left;
                    root->m_avl_tree_node.m_left =
                        v23->m_avl_tree_node.m_right;
                    v23->m_avl_tree_node.m_right = root;
                    root->m_avl_tree_node.m_balance +=
                        ((-v23->m_avl_tree_node.m_balance < 0
                          || v23->m_avl_tree_node.m_balance == 0)
                             ? -v23->m_avl_tree_node.m_balance + 1
                             : 1);
                    int v24 = v23->m_avl_tree_node.m_balance;
                    int v25 = root->m_avl_tree_node.m_balance <= 0
                                  ? 1
                                  : root->m_avl_tree_node.m_balance + 1;
                    v23->m_avl_tree_node.m_balance = v25 + v24;
                    *v15 = v23;
                    int v26 = v23->m_avl_tree_node.m_balance;
                    if (v25 + v24 == 0 || v26 == 1)
                        continue;
                    if (_tlAssert(
                            "c:\\cod\\code\\tl\\physics\\include\\phys_avl_tree.h",
                            701,
                            "GAVLN(root)->m_balance == 0 || GAVLN(root)->m_balance == +1",
                            defaultFileName))
                        __debugbreak();
                }
                else if (m_balance == 2)
                {
                    phys_gjk_cache_info_internal* v28 =
                        root->m_avl_tree_node.m_right;
                    if (v28->m_avl_tree_node.m_balance == -1)
                    {
                        phys_gjk_cache_info_internal* v29 =
                            v28->m_avl_tree_node.m_left;
                        root->m_avl_tree_node.m_right
                            ->m_avl_tree_node.m_left =
                            v29->m_avl_tree_node.m_right;
                        int v30 = v29->m_avl_tree_node.m_balance;
                        v29->m_avl_tree_node.m_right =
                            root->m_avl_tree_node.m_right;
                        root->m_avl_tree_node.m_right
                            ->m_avl_tree_node.m_balance +=
                            v30 >= 0 ? 1 : -v30 + 1;
                        v29->m_avl_tree_node.m_balance +=
                            root->m_avl_tree_node.m_right
                                        ->m_avl_tree_node.m_balance
                                    <= 0
                                ? 1
                                : root->m_avl_tree_node.m_right
                                          ->m_avl_tree_node.m_balance
                                      + 1;
                        root->m_avl_tree_node.m_right = v29;
                    }
                    phys_gjk_cache_info_internal* v31 =
                        root->m_avl_tree_node.m_right;
                    root->m_avl_tree_node.m_right =
                        v31->m_avl_tree_node.m_left;
                    v31->m_avl_tree_node.m_left = root;
                    root->m_avl_tree_node.m_balance +=
                        v31->m_avl_tree_node.m_balance <= 0
                            ? 1
                            : v31->m_avl_tree_node.m_balance + 1;
                    int v33 = (-root->m_avl_tree_node.m_balance < 0
                               || root->m_avl_tree_node.m_balance == 0)
                                  ? 1
                                  : -root->m_avl_tree_node.m_balance + 1;
                    bool v34 =
                        (v33 + v31->m_avl_tree_node.m_balance == 0);
                    v31->m_avl_tree_node.m_balance += v33;
                    *v15 = v31;
                    int v35 = v31->m_avl_tree_node.m_balance;
                    if (v34 || v35 == -1)
                        continue;
                    if (_tlAssert(
                            "c:\\cod\\code\\tl\\physics\\include\\phys_avl_tree.h",
                            708,
                            "GAVLN(root)->m_balance == 0 || GAVLN(root)->m_balance == -1",
                            defaultFileName))
                        __debugbreak();
                }
                else
                {
                    continue;
                }
            } while ((*v15)->m_avl_tree_node.m_balance == 0
                     && cur > the_stack);
        }
    }

    // update_cache - ea: 0x71E1C0
    void update_cache()
    {
        phys_gjk_cache_info_internal** v2 =
            m_list_phys_gjk_cache_info_internal.m_alloc_list;
        phys_gjk_cache_info_internal** end =
            &v2[m_list_phys_gjk_cache_info_internal.m_alloc_count];
        while (end != v2)
        {
            phys_gjk_cache_info_internal* v3 = *v2;
            unsigned int v4 = v3->m_flags;
            if ((v4 & 1) != 0)
            {
                ++v2;
                v3->m_flags = v4 & 0xFFFFFFFE;
            }
            else
            {
                remove(v3->m_key);
                m_list_phys_gjk_cache_info_internal.remove(v3);
            }
        }
    }

    // get_gjk_cache_info - ea: 0x71E090
    phys_gjk_cache_info* get_gjk_cache_info(unsigned int id1,
                                            unsigned int id2, bool no_error)
    {
        unsigned int v4 = id1;
        unsigned int v5 = id2;
        bool swapped;
        if (id1 == id2)
        {
            if (_tlAssert(
                    "c:\\cod\\code\\tl\\physics\\include\\collision\\phys_gjk_cache_system.h",
                    238, "id1 != id2", defaultFileName))
                __debugbreak();
        }
        if (id1 <= id2)
        {
            swapped = false;
        }
        else
        {
            v4 = id2;
            v5 = id1;
            swapped = true;
        }
        if (v4 >= v5
            && _tlAssert(
                   "c:\\cod\\code\\tl\\physics\\include\\collision\\phys_gjk_cache_system.h",
                   23, "id1 < id2", defaultFileName))
            __debugbreak();
        phys_gjk_geom_id_pair_key key;
        key.m_id1 = v4;
        key.m_id2 = v5;
        phys_gjk_cache_info_internal* v9 = find(key);
        if (v9 != nullptr)
        {
            v9->update_swapped(swapped);
            return v9;
        }
        phys_gjk_cache_info_internal* v12 =
            m_list_phys_gjk_cache_info_internal.add(
                no_error,
                "POOL OUT OF MEMORY, phys_gjk_cache_system_avl_tree, INCREASE NUM_PHYS_GJK_CACHE_INFO.");
        if (v12 != nullptr)
        {
            v12->m_key.m_id1 = key.m_id1;
            v12->m_key.m_id2 = key.m_id2;
            v12->m_flags = 0;
            v12->m_flags = swapped ? 2 : 0;
            add(key, v12);
        }
        return v12;
    }
};
static_assert(sizeof(phys_gjk_cache_system_avl_tree<500>) == 0x12900,
              "phys_gjk_cache_system_avl_tree<500> size mismatch");
static_assert(offsetof(phys_gjk_cache_system_avl_tree<500>,
                       m_search_tree_root)
                  == 0x128F0,
              "m_search_tree_root offset mismatch");
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

// prop_phys_collision (physics.o; static collision pass helpers)
struct prop_phys_collision {
    static void get_all_collisions();  // ?get_all_collisions@prop_phys_collision@@SAXXZ
    static void collide_bodies(rb_extra_info* rb_inf1, rb_extra_info* rb_inf2);  // ?collide_bodies@prop_phys_collision@@SAXPAVrb_extra_info@@0@Z
    static void collide_terrain(rb_extra_info* rb_inf);  // ?collide_terrain@prop_phys_collision@@SAXPAVrb_extra_info@@@Z
    static void collide_entities(rb_extra_info* rb_inf);  // ?collide_entities@prop_phys_collision@@SAXPAVrb_extra_info@@@Z
};
void prop_phys_collision::get_all_collisions()
{
    int count = g_list_rb_extra_info.m_alloc_count;
    for (int j = 0; j < count; ++j)
    {
        if ((j < 0 || j >= g_list_rb_extra_info.m_alloc_count)
            && _tlAssert(
                   "c:\\cod\\code\\tl\\physics\\include\\phys_memory_pool_base.inc",
                   178, "i >= 0 && i < m_alloc_count", defaultFileName))
            __debugbreak();
        prop_phys_collision::collide_entities(
            g_list_rb_extra_info.m_alloc_list[j]);
    }
    count = g_list_rb_extra_info.m_alloc_count;
    for (int i = 0; i < count; ++i)
        prop_phys_collision::collide_terrain(
            g_list_rb_extra_info.m_alloc_list[i]);

    // all pairs
    int total = g_list_rb_extra_info.m_alloc_count;
    for (int i = 0; i < total - 1; ++i)
    {
        for (int v5 = i + 1; v5 < total; ++v5)
        {
            prop_phys_collision::collide_bodies(
                g_list_rb_extra_info.m_alloc_list[i],
                g_list_rb_extra_info.m_alloc_list[v5]);
        }
    }
    // vehicle wheel processing
    count = g_list_rb_extra_info.m_alloc_count;
    for (int i = 0; i < count; ++i)
    {
        rb_extra_info* rb_inf = g_list_rb_extra_info.m_alloc_list[i];
        if (rb_inf->m_rb_vehicle != nullptr
            && rb_inf->m_rb_vehicle->m_vci != nullptr)
        {
            vehicle_collision_info* m_vci =
                (vehicle_collision_info*)rb_inf->m_rb_vehicle->m_vci;
            for (int w = 0; w < m_vci->m_list_wheel_collision_info_count; ++w)
                m_vci->m_list_wheel_collision_info[w].process(rb_inf, w);
        }
    }
}

// ea: 0x70C8C0
void prop_system_collision_process()
{
    prop_phys_collision::get_all_collisions();
}
// cdl_brush_t / cdl_patch_t records (cdl_mem.h view; 4 bytes each)
struct cdl_brush_rec {
    uint16_t first_side;
    uint16_t nsides;
};
struct cdl_patch_rec {
    uint16_t first_index;
    uint16_t num_inds;
};

// ea: 0x7197E0
bool are_potentially_colliding(const math::Dir3* b1_mn,
                               const math::Dir3* b1_mx,
                               const math::Dir3* b2_mn,
                               const math::Dir3* b2_mx)
{
    static const math::Dir3 active_eps_vec = {
        _mm_setr_ps(3.4f, 3.4f, 3.4f, 0.0f)
    };
    return (_mm_movemask_ps(
                _mm_cmplt_ps(
                    _mm_max_ps(
                        _mm_sub_ps(b2_mn->v,
                                   _mm_add_ps(b1_mx->v, active_eps_vec.v)),
                        _mm_sub_ps(_mm_sub_ps(b1_mn->v, active_eps_vec.v),
                                   b2_mx->v)),
                    Float4_Zero_12)) & 7) == 7;
}
// ea: 0x6FF120 - wheel segment vs cod geom (brush) collide. The wheel
// segment is transformed into the geom's model space, then collide_brush_segment
// is run against the DCG brush. On hit the wheel info is updated and the
// collided entity (if a non-vehicle prop) takes 1 point of damage.
void collide_segment(wheel_collision_info* wci, Entity* wci_ent, Entity* ent1,
                     rigid_body* g1_rb, const math::Mat43* g1_xform,
                     phys_gjk_geom_cod_base* g1)
{
    int overlap = _mm_movemask_ps(
        _mm_cmplt_ps(
            _mm_max_ps(
                _mm_sub_ps(g1->m_aabb_mn.v, wci->m_aabb_mx.v),
                _mm_sub_ps(wci->m_aabb_mn.v, g1->m_aabb_mx.v)),
            Float4_Zero_12));
    if ((overlap & 7) != 7)
        return;

    // world -> model-space transform via the transposed (inverse) rotation.
    __m128 x = g1_xform->x.v, y = g1_xform->y.v, z = g1_xform->z.v;
    __m128 w = g1_xform->w.v;
    __m128 v11 = _mm_shuffle_ps(x, y, 68);
    __m128 col1 = _mm_shuffle_ps(v11, z, 221);
    __m128 col0 = _mm_shuffle_ps(v11, z, 136);
    __m128 col2 = _mm_shuffle_ps(_mm_shuffle_ps(x, y, 238), z, 168);
    __m128 negw = _mm_xor_ps(Float4_SignMask_12, w);
    __m128 trans = _mm_add_ps(
        _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(negw, negw, 0), col0),
                   _mm_mul_ps(_mm_shuffle_ps(negw, negw, 0x55), col1)),
        _mm_mul_ps(_mm_shuffle_ps(negw, negw, 0xAA), col2));
    math::Position3 p0_local, p1_local;
    p0_local.v = _mm_add_ps(
        _mm_add_ps(
            _mm_mul_ps(_mm_shuffle_ps(wci->m_p0.v, wci->m_p0.v, 0), col0),
            _mm_mul_ps(_mm_shuffle_ps(wci->m_p0.v, wci->m_p0.v, 0x55), col1)),
        _mm_add_ps(
            _mm_mul_ps(_mm_shuffle_ps(wci->m_p0.v, wci->m_p0.v, 0xAA), col2),
            trans));
    p1_local.v = _mm_add_ps(
        _mm_add_ps(
            _mm_mul_ps(_mm_shuffle_ps(wci->m_p1.v, wci->m_p1.v, 0), col0),
            _mm_mul_ps(_mm_shuffle_ps(wci->m_p1.v, wci->m_p1.v, 0x55), col1)),
        _mm_add_ps(
            _mm_mul_ps(_mm_shuffle_ps(wci->m_p1.v, wci->m_p1.v, 0xAA), col2),
            trans));

    float t = wci->m_t;
    if (g1->m_dcg != nullptr)
    {
        DCGSet* dcg = g1->m_dcg;
        unsigned int m_dcg_index = (unsigned short)g1->m_dcg_index;
        if (dcg->get_type(g1->m_dcg_index) != 1
            && _tlAssert("c:\\cod\\code\\game\\RBCollision.cpp", 633,
                         "g1->m_dcg->get_type(g1->m_dcg_index) == CDL_BRUSH",
                         defaultFileName))
            __debugbreak();
        cdl_brush_record* brush =
            &((cdl_brush_record*)dcg->objects_m_elements)[m_dcg_index];
        math::Position3 bmin, bmax;
        bmin.v = _mm_sub_ps(
            _mm_setr_ps(brush->center[0], brush->center[1], brush->center[2],
                        0.0f),
            _mm_setr_ps(brush->extents[0], brush->extents[1],
                        brush->extents[2], 0.0f));
        bmax.v = _mm_add_ps(
            _mm_setr_ps(brush->center[0], brush->center[1], brush->center[2],
                        0.0f),
            _mm_setr_ps(brush->extents[0], brush->extents[1],
                        brush->extents[2], 0.0f));
        unsigned int brush_index =
            m_dcg_index - (unsigned int)dcg->nboxes;
        cdl_brush_side_rec* side_rec =
            &((cdl_brush_side_rec*)dcg->brushes_m_elements)[brush_index];
        if (side_rec->first_side >= (unsigned int)dcg->brush_sides_m_count
            && _tlAssert(
                   "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h", 89,
                   "index >= 0 && index < size()", "invalid index"))
            __debugbreak();
        const cdlPlane* sides =
            (const cdlPlane*)dcg->brush_sides_m_elements
            + side_rec->first_side;
        math::Position3 normal;
        if (!collide_brush_segment(p0_local, p1_local, bmin, bmax, sides,
                                   side_rec->nsides, t, &normal))
            return;

        if (ent1 != nullptr && ent1->scr_vehicle == nullptr
            && wci_ent != nullptr)
        {
            if (wci_ent == nullptr
                && _tlAssert("c:\\cod\\code\\game\\RBCollision.cpp", 644,
                             "wci_ent", defaultFileName))
                __debugbreak();
            float dir[4] = { 0.0f, 1.0f, 0.0f, 0.0f };
            G_Damage(ent1, wci_ent, wci_ent, dir,
                     &wci->m_p0.v.m128_f32[0], 1, 0, 0x20,
                     (EHitLocation)0, -1);
            if ((ent1->mFlags & 0x400000) != 0 || ent1->actor != nullptr
                || ent1->client != nullptr || ent1->think == 0x0C)
                return;
        }

        wci->m_normal.v = normal.v;
        wci->m_t = t;
        wci->m_did_hit = true;
        wci->m_surface_flags = 0;
        wci->m_hit_rb = g1_rb;
        // model-space normal -> world (row-vector * matrix)
        wci->m_normal.v = _mm_add_ps(
            _mm_add_ps(
                _mm_mul_ps(_mm_shuffle_ps(normal.v, normal.v, 0),
                           g1_xform->x.v),
                _mm_mul_ps(_mm_shuffle_ps(normal.v, normal.v, 0x55),
                           g1_xform->y.v)),
            _mm_mul_ps(_mm_shuffle_ps(normal.v, normal.v, 0xAA),
                       g1_xform->z.v));
    }
}
void ValidatePakId(int pakId);  // streamer.o ?ValidatePakId@@YAXW4TPakId@@@Z
// ea: 0x719A90
bool rb_extra_info::can_have_event()
{
    if ((m_flags.mMask & 8) != 0 && m_time_since_last_event > 0.1f)
    {
        __m128 v1 = _mm_mul_ps(m_rb->m_t_vel.v, m_rb->m_t_vel.v);
        if ((v1.m128_f32[0]
             + (_mm_shuffle_ps(v1, v1, 85).m128_f32[0]
                + _mm_shuffle_ps(v1, v1, 170).m128_f32[0]))
            > 2500.0f)
            return true;
    }
    return false;
}
void create_prop_collide_callback(rb_extra_info* rb_inf, int surfaceFlags,
                                  rigid_body* b1,
                                  rigid_body* b2);  // 0x71DB60

// Temporary stand-in for phys_gjk_info::phys_collide_do_gjk_collide (0x87B280,
// phys_gjk.o). The real engine body is ported in src/physics/phys_gjk.cpp but
// its callees (init_gjk, comp_lambda_*, comp_v, cache updates, ...) are not
// ported yet, so pulling it into the link breaks. Swap this out when phys_gjk.o
// internals land.
int phys_collide_do_gjk_collide_stub(void* gjk_info, void* d,
                                     float sep_thresh)
{
    (void)gjk_info; (void)d; (void)sep_thresh;
    return 0;
}

// phys_collide_data_callback (phys_gjk.h view; vtable slot 0 = process)
class phys_collide_data_callback {
public:
    virtual bool process(phys_collide_data* d) = 0;  // (pcd_callback->process)(d)
};

// ea: 0x6F1D40 (physics.o inline; dead COMDAT - call sites inline the body)
void phys_collide_do_gjk_collide_and_contact_manifold(phys_collide_data* d)
{
    if (phys_collide_do_gjk_collide_stub(d->gjk_info, d, 3.4000001f))
    {
        phys_collide_data_callback* pcd_callback =
            (phys_collide_data_callback*)d->pcd_callback;
        if (pcd_callback == nullptr || pcd_callback->process(d))
            d->cman_process->process(d);
    }
}

// collide_entities_callback (physics.o; 12 bytes: vftable + ent1 + ent2).
// process - ea: 0x71EBA0
class collide_entities_callback : public phys_collide_data_callback {
public:
    Entity* ent1;  // +0x04 (the rb_inf's owner entity)
    Entity* ent2;  // +0x08 (the hit entity)

    bool process(phys_collide_data* d) override;
};

// list_phys_collide_data_add - ea: 0x71E2C0 (physics.o inline COMDAT)
void list_phys_collide_data_add(phys_collide_data* d)
{
    d->gjk_ci = g_phys_gjk_cache_system.get_gjk_cache_info(
        d->id1, d->id2, true);
    phys_collide_do_gjk_collide_and_contact_manifold(d);
}

// ea: 0x71EBA0
bool collide_entities_callback::process(phys_collide_data* d)
{
    const math::Mat43* cg1_to_world_xform = d->cg1_to_world_xform;
    math::Position3 hitp;
    hitp.v = _mm_add_ps(
        _mm_add_ps(
            _mm_mul_ps(
                _mm_shuffle_ps(d->cg1_cinfo_loc->m_p1.v,
                               d->cg1_cinfo_loc->m_p1.v, 0),
                cg1_to_world_xform->x.v),
            _mm_mul_ps(
                _mm_shuffle_ps(d->cg1_cinfo_loc->m_p1.v,
                               d->cg1_cinfo_loc->m_p1.v, 0x55),
                cg1_to_world_xform->y.v)),
        _mm_add_ps(
            _mm_mul_ps(
                _mm_shuffle_ps(d->cg1_cinfo_loc->m_p1.v,
                               d->cg1_cinfo_loc->m_p1.v, 0xAA),
                cg1_to_world_xform->z.v),
            cg1_to_world_xform->w.v));
    math::Dir3 normal;
    normal.v = _mm_add_ps(
        _mm_add_ps(
            _mm_mul_ps(
                _mm_shuffle_ps(d->cg1_cinfo_loc->m_n.v,
                               d->cg1_cinfo_loc->m_n.v, 0),
                cg1_to_world_xform->x.v),
            _mm_mul_ps(
                _mm_shuffle_ps(d->cg1_cinfo_loc->m_n.v,
                               d->cg1_cinfo_loc->m_n.v, 0x55),
                cg1_to_world_xform->y.v)),
        _mm_mul_ps(
            _mm_shuffle_ps(d->cg1_cinfo_loc->m_n.v,
                           d->cg1_cinfo_loc->m_n.v, 0xAA),
            cg1_to_world_xform->z.v));
    if (ent2 != nullptr)
    {
        if (ent2->scr_vehicle != nullptr
            && ((scr_vehicle_t*)ent2->scr_vehicle)->mRBVeh != nullptr)
        {
            if (ent1 != nullptr && ent1->scr_vehicle != nullptr)
            {
                Entity* physics_owner =
                    *(Entity**)((char*)((scr_vehicle_t*)ent1->scr_vehicle)
                                + 0x174);
                if (IsLocalPlayer(physics_owner))
                {
                    if (((scr_vehicle_t*)ent2->scr_vehicle)
                            ->seats[0].occupant
                        == 0)
                    {
                        Entity* v15 =
                            *(Entity**)((char*)((scr_vehicle_t*)ent1
                                                    ->scr_vehicle)
                                        + 0x174);
                        MultiplayerMgr::sInst->VehicleRequestOwnership(ent2,
                                                                       v15);
                        Entity* v16 =
                            *(Entity**)((char*)((scr_vehicle_t*)ent1
                                                    ->scr_vehicle)
                                        + 0x174);
                        ((scr_vehicle_t*)ent2->scr_vehicle)
                            ->AssignPhysics(v16);
                    }
                }
            }
            math::Dir3 neg_normal;
            neg_normal.v = _mm_xor_ps(Float4_SignMask_12, normal.v);
            ApplyPhysics(ent2, hitp, neg_normal, 0.1f, false,
                         (hitLocation_t)4 /* HITLOC_TORSO_UPR */);
        }
        else if (ent2->scr_vehicle == nullptr)
        {
            rigid_body* rb1 = d->rb1;
            int damage;
            if (rb1->m_inv_mass < 0.25f
                || (ent1->scr_vehicle != nullptr
                    && ((scr_vehicle_t*)ent1->scr_vehicle)->mRBVeh != nullptr
                    && (((rb_vehicle*)((scr_vehicle_t*)ent1->scr_vehicle)
                             ->mRBVeh)
                            ->m_flags.mMask
                        & 0x100)
                           != 0))
            {
                damage = 300;
            }
            else
            {
                __m128 v20 =
                    _mm_mul_ps(rb1->m_t_vel.v, rb1->m_t_vel.v);
                float v30 =
                    v20.m128_f32[0]
                    + (_mm_shuffle_ps(v20, v20, 85).m128_f32[0]
                       + _mm_shuffle_ps(v20, v20, 170).m128_f32[0]);
                float v21 = v30 * 0.60000002f;
                if (v21 < 1.0f)
                    v21 = 1.0f;
                damage = (int)v21;
            }
            math::Dir3 neg_normal;
            neg_normal.v = _mm_xor_ps(Float4_SignMask_12, normal.v);
            G_Damage(ent2, ent1, ent1, &neg_normal.v.m128_f32[0],
                     &hitp.v.m128_f32[0], damage, 0, 32,
                     (EHitLocation)0 /* HITLOC_NONE */, -1);
        }
    }
    rb_vehicle* v24 = ent1->scr_vehicle != nullptr
                          ? (rb_vehicle*)((scr_vehicle_t*)ent1->scr_vehicle)
                                ->mRBVeh
                          : nullptr;
    if (ent1->scr_vehicle == nullptr || v24 == nullptr
        || (v24->m_flags.mMask & 0x100) == 0)
    {
        Entity* v25 = ent2;
        if ((v25->flags & 0x400000) == 0 && v25->actor == nullptr
            && v25->client == nullptr && v25->think != 0x0C)
            return true;
    }
    return false;
}

void process_prop_collide_callbacks();  // 0x702740
void do_all_biped_system_collision_callback();  // 0x70C8D0
void do_all_biped_system_process_collision_events();  // 0x704A10

// Collision-memory globals (physics.o data @ 0xF7947C..0xF79498). Typed to
// match the binary V/U-tag manglings.
class phys_gjk_info;
class phys_contact_manifold_process;
class TouchEntityData;
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

// cdl_array_t (cdl_mem.h view; 8 bytes)
struct cdl_array_t {
    int   m_count;     // +0x00
    void* m_elements;  // +0x04
};

// CGBank (g_local.h view; full layout, IDA ordinal - get_type cgbank.h inline
// 0x65FB40: 0=box, 1=brush, 2=patch)
class CGBank {
public:
    math::Position3 min;      // +0x00
    math::Position3 max;      // +0x10
    math::Position3 center;   // +0x20
    float radius;             // +0x30
    float radius2;            // +0x34
    uint16_t nboxes;          // +0x38
    uint16_t nbrushes;        // +0x3A
    cdl_array_t objects;      // +0x3C
    cdl_array_t brushes;      // +0x44
    cdl_array_t gjk_brushes;  // +0x4C
    cdl_array_t patches;      // +0x54
    cdl_array_t gjk_patches;  // +0x5C
    cdl_array_t brush_sides;  // +0x64
    cdl_array_t brush_verts;  // +0x6C
    cdl_array_t patch_inds;   // +0x74
    cdl_array_t patch_verts;  // +0x7C
    uint8_t _pad84[0x90 - 0x84];
    rtree_root_t rtree_root;  // +0x90

    int get_type(unsigned int index) const;  // ?get_type@CGBank@@QBEHI@Z
};

// ea: 0x6FEB10
phys_gjk_geom_cod_base* gjk_geom_database::create_gjk_geom(
    const cdl_object_t* obj, int index, CGBank* bank)
{
    int m_terrain_count = this->m_terrain_count;
    this->m_terrain_count = m_terrain_count + 1;
    int type = bank->get_type(index);
    phys_gjk_geom_cod_base* result;
    if (type == 1)
    {
        ++m_brush_count;
        unsigned int m_count = bank->gjk_brushes.m_count;
        int v9 = index - bank->nboxes;
        if (v9 >= (int)m_count)
        {
            bool v10 = !_tlAssert(
                "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h", 91,
                "index >= 0 && index < size()", "invalid index");
            if (!v10)
                __debugbreak();
        }
        cdl_vinfo_t* m_elements = (cdl_vinfo_t*)bank->gjk_brushes.m_elements;
        cdl_array<vi4>* p_brush_verts = (cdl_array<vi4>*)&bank->brush_verts;
        goto LABEL_13;
    }
    if (type == 2)
    {
        ++m_patch_count;
        unsigned int v13 = bank->gjk_patches.m_count;
        int v9 = index - bank->nbrushes - bank->nboxes;
        if (v9 >= (int)v13)
        {
            bool v10 = !_tlAssert(
                "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h", 91,
                "index >= 0 && index < size()", "invalid index");
            if (!v10)
                __debugbreak();
        }
        cdl_vinfo_t* m_elements = (cdl_vinfo_t*)bank->gjk_patches.m_elements;
        cdl_array<vi4>* p_brush_verts = (cdl_array<vi4>*)&bank->patch_verts;
    LABEL_13:
        int num_verts = m_elements[v9].num_verts;
        cdl_vinfo_t* vinfo = &m_elements[v9];
        phys_gjk_geom_vert_list* v14 =
            phys_gjk_geom_vert_list::create(num_verts, nullptr, 0);
        if (v14 == nullptr)
            return nullptr;
        unpack(*vinfo, *p_brush_verts, v14->m_vert_list);
        result = v14;
    }
    else
    {
        ++m_aabb_count;
        math::Dir3 v22, v21;
        v22.v = _mm_setr_ps(obj->center[0], obj->center[1], obj->center[2],
                            0.0f);
        v21.v = _mm_setr_ps(obj->box_radius[0], obj->box_radius[1],
                            obj->box_radius[2], 0.0f);
        result = phys_gjk_geom_aabb::create(v22, v21);
        if (result == nullptr)
            return nullptr;
    }
    result->m_geom_id = (void*)obj;
    result->m_next_geom = nullptr;
    math::Dir3 v22, v23;
    v22.v = _mm_setr_ps(obj->center[0], obj->center[1], obj->center[2],
                        0.0f);
    v23.v = _mm_setr_ps(obj->box_radius[0], obj->box_radius[1],
                        obj->box_radius[2], 0.0f);
    __m128 v18 = _mm_add_ps(v22.v, v23.v);
    result->m_aabb_mn.v = _mm_sub_ps(v23.v, v22.v);
    result->m_aabb_mx.v = v18;
    return result;
}

// ea: 0x703E10
phys_gjk_geom_cod_base* gjk_geom_database::try_get_gjk_geom(
    const cdl_object_t* obj, int index, CGBank* bank)
{
    gjk_geom_info* m_tree_root = this->m_tree_root;
    if (this->m_tree_root != nullptr)
    {
        do
        {
            void* m_geom_id = m_tree_root->m_geom_id;
            if (obj == (const cdl_object_t*)m_geom_id)
                break;
            m_tree_root = (gjk_geom_info*)(obj >= (const cdl_object_t*)m_geom_id
                                               ? m_tree_root->m_avl_right
                                               : m_tree_root->m_avl_left);
        } while (m_tree_root != nullptr);
        if (m_tree_root != nullptr)
            return (phys_gjk_geom_cod_base*)m_tree_root->m_gjk_geom;
    }
    phys_gjk_geom_cod_base* gjk_geom = create_gjk_geom(obj, index, bank);
    if (gjk_geom != nullptr
        && (m_tree_root =
                add_to_sorted_list(gjk_geom, (unsigned int)obj)) != nullptr)
    {
        return (phys_gjk_geom_cod_base*)m_tree_root->m_gjk_geom;
    }
    return nullptr;
}

// ea: 0x703F70
phys_gjk_geom_cod_base* gjk_geom_database::get_gjk_geom(
    const cdl_object_t* obj, int index, CGBank* bank)
{
    phys_gjk_geom_cod_base* result = try_get_gjk_geom(obj, index, bank);
    if (result == nullptr)
    {
        phys_collision_allocater_ballistic_reinit();
        result = try_get_gjk_geom(obj, index, bank);
        if (result == nullptr)
        {
            bool v6 = !_tlAssert("c:\\cod\\code\\game\\RBCollision.cpp", 284,
                                 "gjk_geom", defaultFileName);
            result = nullptr;
            if (!v6)
                __debugbreak();
        }
    }
    return result;
}


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
    struct object_info {
        CGBank* m_bank;   // +0x00
        int     m_index;  // +0x04
    };
    object_info m_object_list[256];  // +0x04
    int     m_object_list_count;  // +0x804
    CGBank* m_cur_bank;           // +0x808
    int     m_mask;               // +0x80C

    visit_result_t visit(int cluster_offset);  // physics.o inline 0xB08E20
};

// ea: 0x65FB40 (cgbank.h inline)
int CGBank::get_type(unsigned int index) const
{
    if (index >= (unsigned int)objects.m_count)
    {
        AeAssert::gCurrentAuthor = AeAssert::JSV;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\cgbank.h";
        AeAssert::gCurrentLine = 239;
        AeAssert::gCurrentExpr = "index < size()";
        if (!AeAssert::IsIgnored() && AeAssert::Assert(defaultFileName))
            __debugbreak();
    }
    unsigned int nboxes = (unsigned int)this->nboxes;
    if (index >= nboxes)
        return 2 - (index < nboxes + (unsigned int)nbrushes);
    return 0;
}

// ea: 0x719920
visit_result_t physics_colgeom_visitor::visit(int index)
{
    if (m_cur_bank == nullptr
        && _tlAssert("c:\\cod\\code\\game\\RBCollision.h", 379, "m_cur_bank",
                     defaultFileName))
        __debugbreak();
    CGBank* m_cur_bank = this->m_cur_bank;
    if (index >= m_cur_bank->objects.m_count)
    {
        AeAssert::gCurrentAuthor = AeAssert::JSV;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\cgbank.h";
        AeAssert::gCurrentLine = 233;
        AeAssert::gCurrentExpr = "index < size()";
        if (!AeAssert::IsIgnored() && AeAssert::Assert(defaultFileName))
            __debugbreak();
        if (index >= m_cur_bank->objects.m_count
            && _tlAssert("c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h", 89,
                         "index >= 0 && index < size()", "invalid index"))
            __debugbreak();
    }
    if ((((cdl_object_t*)m_cur_bank->objects.m_elements)[index].cflags
         & m_mask) != 0)
    {
        if (m_object_list_count >= 256
            && _tlAssert("c:\\cod\\code\\game\\RBCollision.h", 382,
                         "m_object_list_count < MAX_NUM_OBJECTS",
                         defaultFileName))
            __debugbreak();
        int m_object_list_count = this->m_object_list_count;
        if (m_object_list_count < 256)
        {
            m_object_list[m_object_list_count].m_bank = m_cur_bank;
            m_object_list[m_object_list_count].m_index = index;
            this->m_object_list_count = m_object_list_count + 1;
        }
    }
    return CONTINUE_VISITING;
}

// traverse_rtree (physics.o; ported in g_rtree.cpp)
void traverse_rtree(const math::Position3& p0, const math::Position3& p1,
                    const rtree_root_t& root, subdivision_visitor& visitor);

struct physics_colgeom_visitor;
// prop_collide_callback (physics.o RBCollision.cpp; 28 bytes). Class tag (V)
// matches the binary manglings. Inline methods 0x6F2C60..0x6F2CB0.
class prop_collide_callback {
public:
    struct avl_tree_node {
        prop_collide_callback* m_left;    // +0x00
        prop_collide_callback* m_right;   // +0x04
        int                    m_balance; // +0x08
    };

    rb_extra_info*    m_rb_inf;         // +0x00
    int               mHitSurfaceFlags; // +0x04
    rigid_body_pair_key m_avl_key;      // +0x08
    avl_tree_node     m_avl_tree_node;  // +0x10

    // set - ea: 0x6F2C60
    void set(rb_extra_info* rb_inf, int surfaceFlags,
             const rigid_body_pair_key& key)
    {
        m_rb_inf = rb_inf;
        mHitSurfaceFlags = surfaceFlags;
        m_avl_key.m_b1 = key.m_b1;
        m_avl_key.m_b2 = key.m_b2;
    }
    // get_avl_key - ea: 0x6F2C90
    const rigid_body_pair_key& get_avl_key() const { return m_avl_key; }
    // set_avl_key - ea: 0x6F2CA0
    void set_avl_key(const rigid_body_pair_key& key)
    {
        m_avl_key.m_b1 = key.m_b1;
        m_avl_key.m_b2 = key.m_b2;
    }
};
static_assert(sizeof(prop_collide_callback) == 0x1C,
              "prop_collide_callback size mismatch");

// phys_inplace_avl_tree<K,T> (physics.o; phys_avl_tree.h template).
// Same AVL implementation as the pulse_sum.h instantiation (ea 0x7175C0 find /
// 0x71C950 add), verified against the physics.o inline COMDATs.
template <typename K, typename V> class phys_inplace_avl_tree;
template <typename K, typename V>
class phys_inplace_avl_tree {
public:
    V* m_tree_root;  // +0x00

    struct stack_item {
        V** m_node;
        int  m_child;
    };

    phys_inplace_avl_tree() : m_tree_root(nullptr) {}  // ea: 0x71ABE0

    static bool key_lt(const K& a, const K& b)
    {
        if (a.m_b1 != b.m_b1)
            return a.m_b1 < b.m_b1;
        return a.m_b2 < b.m_b2;
    }

    static int avl_max(int a, int b) { return a <= b ? b : a; }  // ea: 0x71B490

    // find - ea: 0x717160
    V* find(const K& key)
    {
        V* result = m_tree_root;
        if (m_tree_root != nullptr)
        {
            do
            {
                if (key.m_b1 == result->m_avl_key.m_b1
                    && key.m_b2 == result->m_avl_key.m_b2)
                    break;
                if (key_lt(key, result->m_avl_key))
                    result = result->m_avl_tree_node.m_left;
                else
                    result = result->m_avl_tree_node.m_right;
            } while (result != nullptr);
        }
        return result;
    }

    // add - ea: 0x71C950
    void add(const K& key, V* data)
    {
        stack_item the_stack[32];
        stack_item* cur = the_stack;
        the_stack[0].m_node = &m_tree_root;
        bool done = (m_tree_root == nullptr);
        for (; !done; ++cur)
        {
            V* node = *cur->m_node;
            if (((cur - the_stack + 8) & 0xFFFFFFF8) >= 256
                && _tlAssert(
                       "c:\\cod\\code\\tl\\physics\\include\\phys_avl_tree.h",
                       586, "cur_item + 1 - the_stack < 32", ""))
                __debugbreak();
            if (key_lt(key, node->m_avl_key))
            {
                cur->m_child = -1;
                cur[1].m_node = &node->m_avl_tree_node.m_left;
            }
            else
            {
                if (!key_lt(node->m_avl_key, key)
                    && _tlAssert(
                           "c:\\cod\\code\\tl\\physics\\include\\phys_avl_tree.h",
                           595, "key > root->get_avl_key()", ""))
                    __debugbreak();
                cur->m_child = 1;
                cur[1].m_node = &node->m_avl_tree_node.m_right;
            }
            done = (*cur[1].m_node == nullptr);
        }
        *cur->m_node = data;
        data->m_avl_tree_node.m_left = nullptr;
        data->m_avl_tree_node.m_right = nullptr;
        data->m_avl_tree_node.m_balance = 0;
        data->m_avl_key.m_b1 = key.m_b1;
        data->m_avl_key.m_b2 = key.m_b2;
        if (cur > the_stack)
        {
            V** m_node;
            do
            {
                m_node = cur[-1].m_node;
                int m_child = cur[-1].m_child;
                --cur;
                (*m_node)->m_avl_tree_node.m_balance += m_child;
                V* root = *m_node;
                int m_balance = root->m_avl_tree_node.m_balance;
                if (m_balance == -2)
                {
                    int v16 =
                        root->m_avl_tree_node.m_left->m_avl_tree_node.m_balance;
                    if (v16 != -1 && v16 != 1
                        && _tlAssert(
                               "c:\\cod\\code\\tl\\physics\\include\\phys_avl_tree.h",
                               613,
                               "GAVLN(GAVLN(root)->m_left)->m_balance == -1 || GAVLN(GAVLN(root)->m_left)->m_balance == 1",
                               ""))
                        __debugbreak();
                    V* m_left = root->m_avl_tree_node.m_left;
                    if (m_left->m_avl_tree_node.m_balance == 1)
                    {
                        V* m_right = m_left->m_avl_tree_node.m_right;
                        root->m_avl_tree_node.m_left
                            ->m_avl_tree_node.m_right =
                            m_right->m_avl_tree_node.m_left;
                        int v20 = m_right->m_avl_tree_node.m_balance;
                        m_right->m_avl_tree_node.m_left =
                            root->m_avl_tree_node.m_left;
                        root->m_avl_tree_node.m_left
                            ->m_avl_tree_node.m_balance +=
                            v20 <= 0 ? 1 : v20 + 1;
                        int v21 = -root->m_avl_tree_node.m_left
                                       ->m_avl_tree_node.m_balance;
                        m_right->m_avl_tree_node.m_balance +=
                            (v21 < 0
                             || root->m_avl_tree_node.m_left
                                        ->m_avl_tree_node.m_balance
                                        == 0)
                                ? 1
                                : v21 + 1;
                        root->m_avl_tree_node.m_left = m_right;
                    }
                    V* v22 = root->m_avl_tree_node.m_left;
                    root->m_avl_tree_node.m_left = v22->m_avl_tree_node.m_right;
                    v22->m_avl_tree_node.m_right = root;
                    root->m_avl_tree_node.m_balance +=
                        ((-v22->m_avl_tree_node.m_balance < 0
                          || v22->m_avl_tree_node.m_balance == 0)
                             ? -v22->m_avl_tree_node.m_balance + 1
                             : 1);
                    v22->m_avl_tree_node.m_balance +=
                        root->m_avl_tree_node.m_balance <= 0
                            ? 1
                            : root->m_avl_tree_node.m_balance + 1;
                    *m_node = v22;
                    if (v22->m_avl_tree_node.m_balance == 0)
                        continue;
                    if (_tlAssert(
                            "c:\\cod\\code\\tl\\physics\\include\\phys_avl_tree.h",
                            617, "GAVLN(root)->m_balance == 0", ""))
                        __debugbreak();
                }
                else if (m_balance == 2)
                {
                    int v24 =
                        root->m_avl_tree_node.m_right->m_avl_tree_node.m_balance;
                    if (v24 != -1 && v24 != 1
                        && _tlAssert(
                               "c:\\cod\\code\\tl\\physics\\include\\phys_avl_tree.h",
                               621,
                               "GAVLN(GAVLN(root)->m_right)->m_balance == -1 || GAVLN(GAVLN(root)->m_right)->m_balance == 1",
                               ""))
                        __debugbreak();
                    V* v26 = root->m_avl_tree_node.m_right;
                    if (v26->m_avl_tree_node.m_balance == -1)
                    {
                        V* v27 = v26->m_avl_tree_node.m_left;
                        v26->m_avl_tree_node.m_left =
                            v27->m_avl_tree_node.m_right;
                        int v28 = v27->m_avl_tree_node.m_balance;
                        v27->m_avl_tree_node.m_right =
                            root->m_avl_tree_node.m_right;
                        root->m_avl_tree_node.m_right
                            ->m_avl_tree_node.m_balance +=
                            v28 >= 0 ? 1 : -v28 + 1;
                        v27->m_avl_tree_node.m_balance +=
                            root->m_avl_tree_node.m_right
                                        ->m_avl_tree_node.m_balance
                                    <= 0
                                ? 1
                                : root->m_avl_tree_node.m_right
                                          ->m_avl_tree_node.m_balance
                                      + 1;
                        root->m_avl_tree_node.m_right = v27;
                    }
                    V* v29 = root->m_avl_tree_node.m_right;
                    root->m_avl_tree_node.m_right = v29->m_avl_tree_node.m_left;
                    v29->m_avl_tree_node.m_left = root;
                    root->m_avl_tree_node.m_balance +=
                        v29->m_avl_tree_node.m_balance <= 0
                            ? 1
                            : v29->m_avl_tree_node.m_balance + 1;
                    v29->m_avl_tree_node.m_balance +=
                        (-root->m_avl_tree_node.m_balance < 0
                         || root->m_avl_tree_node.m_balance == 0)
                            ? 1
                            : -root->m_avl_tree_node.m_balance + 1;
                    *m_node = v29;
                    if (v29->m_avl_tree_node.m_balance == 0)
                        continue;
                    if (_tlAssert(
                            "c:\\cod\\code\\tl\\physics\\include\\phys_avl_tree.h",
                            625, "GAVLN(root)->m_balance == 0", ""))
                        __debugbreak();
                }
                else
                {
                    continue;
                }
            } while ((*m_node)->m_avl_tree_node.m_balance != 0
                     && cur > the_stack);
        }
    }
};
phys_gjk_info* g_gjk_info = nullptr;                         // ?g_gjk_info@@3PAVphys_gjk_info@@A
phys_contact_manifold_process* g_cman_process = nullptr;     // ?g_cman_process@@3PAVphys_contact_manifold_process@@A
phys_collide_data* g_list_phys_collide_data = nullptr;       // ?g_list_phys_collide_data@@3PAVphys_collide_data@@A
physics_colgeom_visitor* g_physics_colgeom_visitor = nullptr;  // ?g_physics_colgeom_visitor@@3PAUphysics_colgeom_visitor@@A
TouchEntityData* g_phys_touch_entity_data = nullptr;         // ?g_phys_touch_entity_data@@3PAVTouchEntityData@@A
prop_collide_callback* g_list_prop_collide_callback = nullptr;  // ?g_list_prop_collide_callback@@3PAVprop_collide_callback@@A
int   g_list_prop_collide_callback_count = 0;       // ?g_list_prop_collide_callback_count@@3HA
phys_inplace_avl_tree<rigid_body_pair_key, prop_collide_callback>
    g_prop_collide_callback_database;  // ?g_prop_collide_callback_database@@3V?$phys_inplace_avl_tree@Vrigid_body_pair_key@@Vprop_collide_callback@@@@A
extern bool tlScratchpadLocked;  // ?tlScratchpadLocked@@3_NA (tl_system.o)

// ea: 0x71DB60
void create_prop_collide_callback(rb_extra_info* rb_inf, int surfaceFlags,
                                  rigid_body* b1, rigid_body* b2)
{
    rigid_body_pair_key key(b1, b2);
    if (g_prop_collide_callback_database.find(key) == nullptr
        && g_list_prop_collide_callback_count < 100)
    {
        prop_collide_callback* v4 =
            &g_list_prop_collide_callback[g_list_prop_collide_callback_count++];
        v4->m_rb_inf = rb_inf;
        v4->mHitSurfaceFlags = surfaceFlags;
        v4->m_avl_key.m_b1 = key.m_b1;
        v4->m_avl_key.m_b2 = key.m_b2;
        g_prop_collide_callback_database.add(key, v4);
    }
}

// ea: 0x702740
void process_prop_collide_callbacks()
{
    int count = g_list_prop_collide_callback_count;
    if (count > 0)
    {
        prop_collide_callback* p = g_list_prop_collide_callback;
        int i = 0;
        for (int processed = 0; processed < count; ++processed, ++p)
        {
            if (i >= 20)
                return;
            rigid_body_constraint_contact* v3 =
                ((phys_inplace_avl_tree<rigid_body_pair_key,
                                        rigid_body_constraint_contact>*)
                     ((char*)g_physics_system + 0x1E4))
                    ->find(p->m_avl_key);
            if (v3 != nullptr)
            {
                rigid_body* b2 = v3->b2;
                rigid_body* b1 = v3->b1;
                __m128 rel_vel = _mm_sub_ps(b1->m_t_vel.v, b2->m_t_vel.v);
                float intensity = 0.0f;
                contact_point_info* d = nullptr;
                for (contact_point_info* m_first =
                         v3->m_list_contact_point_info_buffer_1.m_first;
                     m_first != nullptr; m_first = m_first->m_next_link)
                {
                    if (m_first->m_point_pair_count <= 0
                        && _tlAssert(
                               "c:\\cod\\code\\game\\RBCollisionCallback.cpp",
                               77, "cpi.m_point_pair_count > 0",
                               defaultFileName))
                        __debugbreak();
                    __m128 v9 = _mm_mul_ps(m_first->m_normal.v, rel_vel);
                    float dot =
                        v9.m128_f32[0]
                        + (_mm_shuffle_ps(v9, v9, 85).m128_f32[0]
                           + _mm_shuffle_ps(v9, v9, 170).m128_f32[0]);
                    if (dot > intensity)
                    {
                        intensity = dot;
                        d = m_first;
                    }
                }
                if (d != nullptr && intensity > 50.0f)
                {
                    if (d == nullptr
                        && _tlAssert(
                               "c:\\cod\\code\\game\\RBCollisionCallback.cpp",
                               87, "max_intensity_cpi", defaultFileName))
                        __debugbreak();
                    rigid_body* v10 = v3->b2;
                    ++i;
                    if ((v10->m_flags & 0x50) == 0
                        && _tlAssert(
                               "c:\\cod\\code\\tl\\physics\\include\\rigid_body.h",
                               109, "debug_flag_is_in_collision()",
                               defaultFileName))
                        __debugbreak();
                    __m128 local_hit = d->m_list_b2_r_loc->v;
                    __m128 world_hit = _mm_add_ps(
                        _mm_add_ps(
                            _mm_mul_ps(
                                _mm_shuffle_ps(local_hit, local_hit, 0),
                                v10->m_col_mat.x.v),
                            _mm_mul_ps(
                                _mm_shuffle_ps(local_hit, local_hit, 85),
                                v10->m_col_mat.y.v)),
                        _mm_add_ps(
                            _mm_mul_ps(
                                _mm_shuffle_ps(local_hit, local_hit, 170),
                                v10->m_col_mat.z.v),
                            v10->m_col_mat.w.v));
                    __m128 normal;
                    if (v3->b1 == p->m_rb_inf->m_rb)
                        normal =
                            _mm_xor_ps(Float4_SignMask_12, d->m_normal.v);
                    else
                        normal = d->m_normal.v;
                    p->m_rb_inf->m_time_since_last_event = 0.0f;
                    DCGSet* m_dcg = (DCGSet*)p->m_rb_inf->m_ent->r.bmodel;
                    unsigned int material_index =
                        ((unsigned int)p->mHitSurfaceFlags >> 20) & 0x1F;
                    if (m_dcg->objects_m_count == 0)
                    {
                        AeAssert::gCurrentAuthor = AeAssert::DK;
                        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\cgbank.h";
                        AeAssert::gCurrentLine = 77;
                        AeAssert::gCurrentExpr = "index < size()";
                        if (!AeAssert::IsIgnored()
                            && AeAssert::Assert(defaultFileName))
                            __debugbreak();
                        if (m_dcg->objects_m_count == 0
                            && _tlAssert(
                                   "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h",
                                   89, "index >= 0 && index < size()",
                                   "invalid index"))
                            __debugbreak();
                    }
                    int material =
                        (((cdl_object_t*)m_dcg->objects_m_elements)[0].sflags
                         >> 20)
                        & 0x1F;
                    int col_material = 0;
                    if (material == 2)
                        col_material = 17;  // kCollisionMaterialFLESH
                    float scaled = intensity * 0.002f;
                    if (scaled < 0.0f)
                        scaled = 0.0f;
                    else if (scaled > 1.0f)
                        scaled = 1.0f;
                    math::Position3 coord;
                    coord.v = world_hit;
                    math::Dir3 normal_v;
                    normal_v.v = normal;
                    CollisionDesc col_desc;
                    col_desc.simple.coord.v = coord.v;
                    col_desc.simple.normal.v = normal_v.v;
                    col_desc.material = (ECollisionMaterial)material_index;
                    PostEffectEventPhysicsImpact(p->m_rb_inf->m_ent,
                                                 col_material, &col_desc,
                                                 scaled);

                    float v = intensity * 0.001f;
                    if (v > 0.0f && (v > 1.0f || v > 0.3f))
                    {
                        if (v > 1.0f)
                            v = 1.0f;
                        scr_vehicle_t* sv =
                            (scr_vehicle_t*)p->m_rb_inf->m_ent->scr_vehicle;
                        if (sv != nullptr)
                        {
                            float dot =
                                normal_v.v.m128_f32[0] * sv->phys.vel.v.m128_f32[0]
                                + normal_v.v.m128_f32[1]
                                      * sv->phys.vel.v.m128_f32[1]
                                + normal_v.v.m128_f32[2]
                                      * sv->phys.vel.v.m128_f32[2];
                            float scale;
                            if (dot > 0.0f)
                                scale = 1.0f;
                            else
                            {
                                float s2 = 1.0f - dot;
                                scale = (s2 > 2.0f) ? 2.0f : s2;
                            }
                            math::Position3 pos;
                            pos.v = world_hit;
                            sv->CollisionDamage(p->m_rb_inf->m_ent, &pos,
                                &coord, v / scale);
                        }
                    }
                }
            }
        }
    }
}

// ea: 0x702CB0
void collision_memory_prolog()
{
    if (tlScratchpadLocked
        && _tlAssert("c:/cod/code/tl/base/include\\tl_system.h", 294,
                     "!tlScratchpadLocked",
                     "Scratchpad is already locked!"))
        __debugbreak();
    tlScratchpadLocked = true;
    g_collision_memory_allocater.m_list_memory_buffer[0].set_buffer(
        g_physics_memory_buffer, 0, 1);  // dword_12AC0 (.textbss = 0)
    g_collision_memory_allocater.m_num_buffers = 1;
    g_collision_memory_allocater.m_high_buffer_count = 1;
    g_gjk_info = (phys_gjk_info*)
        g_collision_memory_allocater.allocate(
            816, 16, false, "phys_collision_allocater overflow.");
    void* v0 = g_collision_memory_allocater.allocate(
        4416, 16, false, "phys_collision_allocater overflow.");
    phys_contact_manifold_process* v1 = nullptr;
    if (v0 != nullptr)
        v1 = new (v0) phys_contact_manifold_process;
    g_cman_process = v1;
    g_cman_process->m_cpi_allocater = contact_point_info::get_cpi_allocater();
    g_list_phys_collide_data = (phys_collide_data*)
        g_collision_memory_allocater.allocate(
            84, 4, false, "phys_collision_allocater overflow.");
    gjk_geom_database* v2 = (gjk_geom_database*)
        g_collision_memory_allocater.allocate(
            28, 4, false, "phys_collision_allocater overflow.");
    if (v2 != nullptr)
        v2->m_tree_root = nullptr;
    else
        v2 = nullptr;
    g_gjk_geom_database = v2;
    physics_colgeom_visitor* v3 = (physics_colgeom_visitor*)
        g_collision_memory_allocater.allocate(
            2064, 4, false, "phys_collision_allocater overflow.");
    g_physics_colgeom_visitor = v3;
    g_phys_touch_entity_data = (TouchEntityData*)
        g_collision_memory_allocater.allocate(
            560, 16, false, "phys_collision_allocater overflow.");
    g_list_prop_collide_callback = (prop_collide_callback*)
        g_collision_memory_allocater.allocate(
            2800, 4, false, "phys_collision_allocater overflow.");
    gjk_geom_database* v4 = g_gjk_geom_database;
    v4->m_tree_root = nullptr;
    v4->m_terrain_count = 0;
    v4->m_entity_count = 0;
    v4->m_actor_count = 0;
    v4->m_patch_count = 0;
    v4->m_brush_count = 0;
    v4->m_aabb_count = 0;
    g_list_prop_collide_callback_count = 0;
    g_prop_collide_callback_database.m_tree_root = nullptr;
    for (int i = 0; i < g_collision_memory_allocater.m_num_buffers; ++i)
        g_collision_memory_allocater.m_list_memory_buffer[i].m_user_start =
            g_collision_memory_allocater.m_list_memory_buffer[i].m_buffer_cur;
}

// ea: 0x70AC40
void prop_phys_collision::collide_bodies(rb_extra_info* rb_inf1,
                                         rb_extra_info* rb_inf2)
{
    rb_extra_info* v2 = rb_inf2;
    rb_extra_info* v3 = rb_inf1;
    if ((rb_inf1->m_flags.mMask & 4) != 0
        || (rb_inf2->m_flags.mMask & 4) != 0)
    {
        Entity* m_ent = rb_inf2->m_ent;
        Entity* ent1 = rb_inf1->m_ent;
        if (ent1 == nullptr
            && _tlAssert("c:\\cod\\code\\game\\RBCollision.cpp", 851, "ent1",
                         defaultFileName))
            __debugbreak();
        if (m_ent == nullptr
            && _tlAssert("c:\\cod\\code\\game\\RBCollision.cpp", 852, "ent2",
                         defaultFileName))
            __debugbreak();
        math::Mat43* b2w = rb_inf2->m_cg_mesh_mat;
        math::Mat43* a2w = rb_inf1->m_cg_mesh_mat;
        rigid_body* rb1 = rb_inf1->m_rb;
        rigid_body* rb2 = rb_inf2->m_rb;
        if (rb_inf1->m_gjk_geom_list == nullptr
            && _tlAssert("c:\\cod\\code\\game\\RBCollision.cpp", 860,
                         "rb_inf1->m_gjk_geom_list", defaultFileName))
            __debugbreak();
        if (rb_inf2->m_gjk_geom_list == nullptr
            && _tlAssert("c:\\cod\\code\\game\\RBCollision.cpp", 861,
                         "rb_inf2->m_gjk_geom_list", defaultFileName))
            __debugbreak();
        if (are_potentially_colliding(
                &((phys_gjk_geom_list*)rb_inf1->m_gjk_geom_list)->m_aabb_mn,
                &((phys_gjk_geom_list*)rb_inf1->m_gjk_geom_list)->m_aabb_mx,
                &((phys_gjk_geom_list*)rb_inf2->m_gjk_geom_list)->m_aabb_mn,
                &((phys_gjk_geom_list*)rb_inf2->m_gjk_geom_list)->m_aabb_mx))
        {
            if (rb_inf1->m_rb_vehicle != nullptr
                && rb_inf1->m_rb_vehicle->m_vci != nullptr)
            {
                vehicle_collision_info* m_vci =
                    (vehicle_collision_info*)rb_inf1->m_rb_vehicle->m_vci;
                for (void* g1 = ((phys_gjk_geom_list*)rb_inf2->m_gjk_geom_list)
                                    ->m_first_geom;
                     g1 != nullptr; g1 = *(void**)g1)
                {
                    for (int i = 0; i < m_vci->m_list_wheel_collision_info_count; ++i)
                        collide_segment(
                            &m_vci->m_list_wheel_collision_info[i], nullptr,
                            nullptr, rb2, b2w,
                            (phys_gjk_geom_cod_base*)g1);
                    v3 = rb_inf1;
                }
                v2 = rb_inf2;
            }
            if (v2->m_rb_vehicle != nullptr
                && v2->m_rb_vehicle->m_vci != nullptr)
            {
                vehicle_collision_info* v11 =
                    (vehicle_collision_info*)v2->m_rb_vehicle->m_vci;
                for (void* g1a =
                         ((phys_gjk_geom_list*)v3->m_gjk_geom_list)
                             ->m_first_geom;
                     g1a != nullptr; g1a = *(void**)g1a)
                {
                    for (int i = 0; i < v11->m_list_wheel_collision_info_count; ++i)
                        collide_segment(&v11->m_list_wheel_collision_info[i],
                                        nullptr, nullptr, rb1, a2w,
                                        (phys_gjk_geom_cod_base*)g1a);
                    v3 = rb_inf1;
                }
            }
            for (void* m_first_geom =
                     ((phys_gjk_geom_list*)v3->m_gjk_geom_list)->m_first_geom;
                 m_first_geom != nullptr;
                 m_first_geom = *(void**)m_first_geom)
            {
                for (void* g2 = ((phys_gjk_geom_list*)rb_inf2->m_gjk_geom_list)
                                    ->m_first_geom;
                     g2 != nullptr; g2 = *(void**)g2)
                {
                    if (are_potentially_colliding(
                            &((phys_gjk_geom_list*)m_first_geom)->m_aabb_mn,
                            &((phys_gjk_geom_list*)m_first_geom)->m_aabb_mx,
                            &((phys_gjk_geom_list*)g2)->m_aabb_mn,
                            &((phys_gjk_geom_list*)g2)->m_aabb_mx))
                    {
                        DObj* mDObj = ent1->mDObj;
                        float M_FRICTION_COEF = rb1->m_fric_coef;
                        float M_BOUNCE_COEF = 0.0f;
                        ValidatePakId(mDObj->mPakId);
                        void* mValue = mDObj->mPhysDataValue;
                        if (mValue != nullptr)
                        {
                            ValidatePakId(mDObj->mPakId);
                            M_BOUNCE_COEF = ((PhysData*)mValue)->mBounce;
                        }
                        if (rb_inf1->can_have_event())
                            create_prop_collide_callback(rb_inf1, 0, rb1,
                                                         rb2);
                        phys_collide_data* v19 = g_list_phys_collide_data;
                        if ((rb2->m_flags & 0x50) == 0
                            && _tlAssert(
                                   "c:\\cod\\code\\tl\\physics\\include\\rigid_body.h",
                                   109, "debug_flag_is_in_collision()",
                                   defaultFileName))
                            __debugbreak();
                        int m_priority = rb_inf1->m_priority;
                        void* v21 = g_cman_process;
                        void* v27 = g_gjk_info;
                        unsigned int m_geom_id = *(unsigned int*)g2;
                        unsigned int v23 = *(unsigned int*)m_first_geom;
                        v19->gjk_cg1 = (const phys_gjk_geom*)m_first_geom;
                        v19->gjk_cg2 = (const phys_gjk_geom*)g2;
                        v19->cg1_to_world_xform = a2w;
                        v19->cg2_to_world_xform = b2w;
                        v19->cg1_to_rb1_xform = &rb_inf1->m_transform;
                        v19->rb2_to_world_xform = &rb2->m_col_mat;
                        v19->rb1 = rb1;
                        v19->rb2 = rb2;
                        v19->gjk_ci = nullptr;
                        v19->pcd_callback = nullptr;
                        v19->id1 = v23;
                        v19->fric_coef = M_FRICTION_COEF;
                        v19->id2 = m_geom_id;
                        v19->bounce_coef = M_BOUNCE_COEF;
                        v19->gjk_info = (phys_gjk_info*)v27;
                        v19->cman_process =
                            (phys_contact_manifold_process*)v21;
                        v19->no_overflow_error = true;
                        v19->solver_priority = m_priority;
                        v19->gjk_ci =
                            g_phys_gjk_cache_system.get_gjk_cache_info(
                                v23, m_geom_id, true);
                        phys_collide_do_gjk_collide_and_contact_manifold(v19);
                    }
                }
            }
        }
    }
}

// ea: 0x719050
phys_contact_manifold_process::phys_contact_manifold_process()
{
    *(void**)((char*)m_list_cpi + 0x00) = nullptr;  // m_list_cpi.m_first
    *(void**)((char*)m_list_cpi + 0x04) = nullptr;  // m_list_cpi.m_last
    phys_memory_heap* p_m_allocater = &m_allocater;
    m_allocater.m_buffer_start = nullptr;
    m_allocater.m_buffer_end = nullptr;
    m_allocater.m_buffer_cur = nullptr;
    m_allocater.m_user_start = nullptr;
    m_cpi_allocater = nullptr;
    *(void**)((char*)m_list_cpi + 0x00) = nullptr;
    *(void**)((char*)m_list_cpi + 0x04) = nullptr;
    m_cpi = nullptr;
    m_allocater.set_buffer(m_allocater_memory, 4096, 1);
    cman1.m_allocater = p_m_allocater;
    cman2.m_allocater = p_m_allocater;
}

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

// ea: 0x704A90
bool collide_ray_object_list(physics_colgeom_visitor* visitor,
                             const math::Dir3& aabb_mn,
                             const math::Dir3& aabb_mx,
                             const math::Dir3& p0, const math::Dir3& p1,
                             float* t_)
{
    *t_ = 1.0f;
    bool did_hit = false;
    int m_object_list_count = visitor->m_object_list_count;
    if (m_object_list_count > 0)
    {
        for (int obj_inf_i = 0; obj_inf_i < m_object_list_count; ++obj_inf_i)
        {
            CGBank* m_bank = visitor->m_object_list[obj_inf_i].m_bank;
            int index = visitor->m_object_list[obj_inf_i].m_index;
            if ((unsigned int)index >= (unsigned int)m_bank->objects.m_count)
            {
                AeAssert::gCurrentAuthor = AeAssert::JSV;
                AeAssert::gCurrentFile = "c:\\cod\\code\\game\\cgbank.h";
                AeAssert::gCurrentLine = 233;
                AeAssert::gCurrentExpr = "index < size()";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert(defaultFileName))
                    __debugbreak();
                if ((unsigned int)index
                        >= (unsigned int)m_bank->objects.m_count
                    && _tlAssert(
                           "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h", 89,
                           "index >= 0 && index < size()", "invalid index"))
                    __debugbreak();
            }
            cdl_object_t* obj =
                &((cdl_object_t*)m_bank->objects.m_elements)[index];
            int type = m_bank->get_type(index);
            math::Dir3 v15, v16;
            v15.v = _mm_setr_ps(obj->center[0] - obj->box_radius[0],
                                obj->center[1] - obj->box_radius[1],
                                obj->center[2] - obj->box_radius[2], 0.0f);
            v16.v = _mm_setr_ps(obj->center[0] + obj->box_radius[0],
                                obj->center[1] + obj->box_radius[1],
                                obj->center[2] + obj->box_radius[2], 0.0f);
            __m128 v17 = _mm_cmplt_ps(
                _mm_max_ps(_mm_sub_ps(v15.v, aabb_mx.v),
                           _mm_sub_ps(aabb_mn.v, v16.v)),
                _mm_setzero_ps());
            if ((_mm_movemask_ps(v17) & 7) == 7)
            {
                bool hit;
                math::Position3 normal;
                normal.v = _mm_setzero_ps();
                if (type == 1)
                {
                    int brush_idx = index - m_bank->nboxes;
                    if ((unsigned int)brush_idx
                        >= (unsigned int)m_bank->brushes.m_count)
                    {
                        bool v19 = _tlAssert(
                            "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h", 91,
                            "index >= 0 && index < size()", "invalid index");
                        if (v19)
                            __debugbreak();
                    }
                    unsigned char* brush =
                        &((unsigned char*)m_bank->brushes.m_elements)
                             [brush_idx * 8];
                    unsigned int first_side = *(unsigned short*)brush;
                    unsigned int nsides = *(unsigned short*)(brush + 2);
                    if (first_side
                        >= (unsigned int)m_bank->brush_sides.m_count)
                    {
                        if (_tlAssert(
                                "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h",
                                91, "index >= 0 && index < size()",
                                "invalid index"))
                            __debugbreak();
                    }
                    math::Position3 p0p, p1p, bmin, bmax;
                    p0p.v = p0.v;
                    p1p.v = p1.v;
                    bmin.v = v15.v;
                    bmax.v = v16.v;
                    hit = collide_brush_segment(
                        p0p, p1p, bmin, bmax,
                        (const cdlPlane*)m_bank->brush_sides.m_elements
                            + first_side,
                        nsides, *t_, &normal);
                }
                else if (type == 2)
                {
                    int patch_idx = index - m_bank->nbrushes - m_bank->nboxes;
                    if ((unsigned int)patch_idx
                        >= (unsigned int)m_bank->patches.m_count)
                    {
                        if (!_tlAssert(
                                "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h",
                                91, "index >= 0 && index < size()",
                                "invalid index"))
                            __debugbreak();
                    }
                    unsigned short* patch =
                        &((unsigned short*)m_bank->patches.m_elements)
                             [patch_idx * 2];
                    unsigned int first_index = patch[0];
                    unsigned int num_indices = patch[1];
                    if ((unsigned int)patch_idx
                        >= (unsigned int)m_bank->gjk_patches.m_count)
                    {
                        if (_tlAssert(
                                "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h",
                                91, "index >= 0 && index < size()",
                                "invalid index"))
                            __debugbreak();
                    }
                    phys_gjk_geom_cod_base* gjk_geom =
                        g_gjk_geom_database->get_gjk_geom(obj, index, m_bank);
                    if (gjk_geom != nullptr)
                    {
                        math::Position3 p0p, p1p;
                        p0p.v = p0.v;
                        p1p.v = p1.v;
                        hit = collide_segment(
                            *obj,
                            ((phys_gjk_geom_vert_list*)gjk_geom)->m_vert_list,
                            (const unsigned char*)m_bank->patch_inds
                                    .m_elements
                                + first_index,
                            0, (int)num_indices, p0p, p1p, *t_, normal,
                            nullptr);
                        did_hit |= hit;
                    }
                    continue;
                }
                else
                {
                    math::Position3 p0p, p1p, bmin, bmax;
                    p0p.v = p0.v;
                    p1p.v = p1.v;
                    bmin.v = v15.v;
                    bmax.v = v16.v;
                    hit = collide_box_segment(p0p, p1p, bmin, bmax, *t_,
                                              &normal);
                }
                did_hit |= hit;
            }
        }
    }
    return did_hit;
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
    g_prop_collide_callback_database.m_tree_root = nullptr;
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

// phys_anim_bone (physics.o; 96 bytes - mat_loc +0x00, qend +0x40,
// mBoneIndex +0x50, m_rb_index +0x54, m_rb_parent_index +0x58)
class phys_anim_bone {
public:
    math::Mat43 mat_loc;      // +0x00
    math::Quaternion qend;    // +0x40
    int     mBoneIndex;  // +0x50
    int     m_rb_index;  // +0x54
    int     m_rb_parent_index;  // +0x58
};
class phys_anim_bone_array {
public:
    static void copy_skeleton(Entity* owner, math::Mat43* const skeleton_pose);
    static void write_skeleton(Entity* owner, math::Mat43* const skeleton_pose);
    void copy_back_bones(Entity* owner);  // ?copy_back_bones@phys_anim_bone_array@@QAEXPAVEntity@@@Z
    void remove_rigid_body(int rb_index);  // ?remove_rigid_body@phys_anim_bone_array@@QAEXH@Z
    void copy_back_tween(Entity* owner, float t_);  // ?copy_back_tween@phys_anim_bone_array@@QAEXPAVEntity@@M@Z
    void attach_physics_bones(Entity* owner);  // ?attach_physics_bones@phys_anim_bone_array@@QAEXPAVEntity@@@Z
    void copy_tween_start(Entity* owner);      // ?copy_tween_start@phys_anim_bone_array@@QAEXPAVEntity@@@Z
    int  get_bone(int rb_index);  // ?get_bone@phys_anim_bone_array@@QAEHH@Z
    int  get_phys_bone(int bone_id);  // ?get_phys_bone@phys_anim_bone_array@@QAEHH@Z

    // ?m_list_phys_anim_bone@phys_anim_bone_array@@2V?$phys_static_array@Vphys_anim_bone@@$0FK@@@A
    // (physics.o data @ 0xE01F10)
    static phys_static_array<phys_anim_bone, 90> m_list_phys_anim_bone;

    phys_static_array<math::Quaternion, 90> m_list_qstart;  // +0x00
    phys_static_array<math::Position3, 90>  m_list_pstart;  // +0x5B0
};
phys_static_array<phys_anim_bone, 90>
    phys_anim_bone_array::m_list_phys_anim_bone;

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

// CopyMatrix (render.o inline COMDAT 0x6E7820) - DObjSkelMat -> Mat43
void CopyMatrix(math::Mat43& out, DObjSkelMat& in)
{
    memcpy(&out, &in, sizeof(DObjSkelMat));
}

// dword_E01E5C - physics.o bone/rb mapping table. First dword of each 8-byte
// pair is the bone index (-1 until runtime init), second is a name pointer.
int dword_E01E5C[20] = {
    -1, 0, -1, 0, -1, 0, -1, 0, -1, 0,
    -1, 0, -1, 0, -1, 0, -1, 0, -1, 0,
};

// phys_static_array::add for object-element arrays (the local rb_ragdoll
// template returns the slot value; the binary's object instantiations return
// a pointer, e.g. ?add@?$phys_static_array@Vphys_anim_bone@@$0FK@@@QAEPAV...)
template <typename T, int CAP>
static T* phys_static_array_add_ptr(phys_static_array<T, CAP>& arr,
                                    const char* error_msg)
{
    if (arr.m_alloc_count < CAP)
    {
        T* slot = &arr.m_slot_array[arr.m_alloc_count];
        arr.m_alloc_count += 1;
        return slot;
    }
    tlFatal(error_msg);
    return nullptr;
}

// Matrix -> quaternion (branch structure + constants from attach_physics_bones
// 0x6FFF40 / copy_tween_start 0x6F8190; threshold -1/3, 0.5/sqrt scale).
static math::Quaternion MatToQuat(const math::Mat43& m)
{
    float m00 = m.x.v.m128_f32[0], m01 = m.x.v.m128_f32[1],
          m02 = m.x.v.m128_f32[2];
    float m10 = m.y.v.m128_f32[0], m11 = m.y.v.m128_f32[1],
          m12 = m.y.v.m128_f32[2];
    float m20 = m.z.v.m128_f32[0], m21 = m.z.v.m128_f32[1],
          m22 = m.z.v.m128_f32[2];
    float trace = (m00 + m11) + m22;
    math::Quaternion q;
    if (trace >= -0.33333299f)
    {
        float s = 0.5f / sqrtf(trace + 1.0f);
        q.x = (m21 - m12) * s;
        q.y = (m02 - m20) * s;
        q.z = (m10 - m01) * s;
        q.w = (trace + 1.0f) * s;
    }
    else if (m11 <= m00)
    {
        if (m00 <= m22)
        {
            float s = 0.5f / sqrtf((m22 - m00 - m11) + 1.0f);
            q.x = (m20 + m02) * s;
            q.y = (m12 + m21) * s;
            q.z = (m10 - m01) * s;
            q.w = (m22 - m00 - m11 + 1.0f) * s;
        }
        else
        {
            float s = 0.5f / sqrtf((m00 - m11 - m22) + 1.0f);
            q.x = (m00 - m11 - m22 + 1.0f) * s;
            q.y = (m01 + m10) * s;
            q.z = (m20 + m02) * s;
            q.w = (m12 - m21) * s;
        }
    }
    else if (m11 <= m22)
    {
        float s = 0.5f / sqrtf((m22 - m00 - m11) + 1.0f);
        q.x = (m20 + m02) * s;
        q.y = (m12 + m21) * s;
        q.z = (m10 - m01) * s;
        q.w = (m22 - m00 - m11 + 1.0f) * s;
    }
    else
    {
        float s = 0.5f / sqrtf((m11 - m00 - m22) + 1.0f);
        q.x = (m01 + m10) * s;
        q.y = (m11 - m00 - m22 + 1.0f) * s;
        q.z = (m12 + m21) * s;
        q.w = (m02 - m20) * s;
    }
    return q;
}

// ea: 0x6FFF40
void phys_anim_bone_array::attach_physics_bones(Entity* owner)
{
    int v4 = 0;
    if (m_list_phys_anim_bone.m_alloc_count <= 0
        || owner->mDObj->numBones > m_list_phys_anim_bone.m_alloc_count)
    {
        m_list_phys_anim_bone.m_alloc_count = 0;
        int numBones = owner->mDObj->numBones;
        DObjSkelMat* MatrixArray = SV_DObjGetMatrixArray(owner);
        if (MatrixArray == nullptr)
        {
            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\RBRagdoll.cpp";
            AeAssert::gCurrentLine = 527;
            AeAssert::gCurrentExpr = "matrices";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("no matrices for dobj?"))
                __debugbreak();
        }
        DObjMatriceModelToLocal(owner);
        if (numBones > 0)
        {
            while (v4 < numBones)
            {
                phys_anim_bone* v5 = phys_static_array_add_ptr(
                    m_list_phys_anim_bone, "phys array add overflow.");
                v5->m_rb_index = -1;
                v5->m_rb_parent_index = -1;
                v5->mBoneIndex = v4;
                int v6 = 0;
                while (v4 != dword_E01E5C[2 * v6])
                {
                    if (++v6 >= 10)
                        goto LABEL_13;
                }
                v5->m_rb_index = v6;
            LABEL_13:
                int BoneParent = owner->mDObj->GetBoneParent(v4);
                if (BoneParent >= 0)
                {
                    if (BoneParent >= v4
                        && _tlAssert(
                               "c:\\cod\\code\\game\\RBRagdoll.cpp", 552,
                               "parentBoneIndex < bctr", defaultFileName))
                        __debugbreak();
                    v5->m_rb_parent_index = v5->m_rb_index;
                }
                CopyMatrix(v5->mat_loc, *MatrixArray);
                v5->qend = MatToQuat(v5->mat_loc);
                ++v4;
                ++MatrixArray;
            }
        }
        DObjGetBasePose(owner->mDObj);
    }
}

// ea: 0x6F8190
void phys_anim_bone_array::copy_tween_start(Entity* owner)
{
    int numBones = owner->mDObj->numBones;
    int v80 = numBones;
    int v110 = 0;
    if (numBones != 0)
    {
        while (true)
        {
            math::Quaternion* v4 = phys_static_array_add_ptr(
                m_list_qstart, "phys array add overflow.");
            math::Position3* v5 = phys_static_array_add_ptr(
                m_list_pstart, "phys array add overflow.");
            if (v4 == nullptr || v5 == nullptr)
            {
                AeAssert::gCurrentAuthor = AeAssert::JSV;
                AeAssert::gCurrentFile =
                    "c:\\cod\\code\\game\\RBRagdoll.cpp";
                AeAssert::gCurrentLine = 572;
                AeAssert::gCurrentExpr = "qstart && pstart";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert("the character has too many bones"))
                    __debugbreak();
            }
            int BoneParent = owner->mDObj->GetBoneParent(v110);
            if (BoneParent < 0)
            {
                const math::Mat43::Packed& BaseRelMat =
                    owner->GetBaseRelMat(v110);
                math::Mat43 m;
                m.x.v = _mm_setr_ps(BaseRelMat.x.x, BaseRelMat.x.y,
                                    BaseRelMat.x.z, 0.0f);
                m.y.v = _mm_setr_ps(BaseRelMat.y.x, BaseRelMat.y.y,
                                    BaseRelMat.y.z, 0.0f);
                m.z.v = _mm_setr_ps(BaseRelMat.z.x, BaseRelMat.z.y,
                                    BaseRelMat.z.z, 0.0f);
                m.w.v = _mm_setzero_ps();
                *v4 = MatToQuat(m);
                math::Position3 p;
                p.v = _mm_setr_ps(BaseRelMat.w.x, BaseRelMat.w.y,
                                  BaseRelMat.w.z, 0.0f);
                *v5 = p;
                goto LABEL_28;
            }
            const math::Mat43& parentMat =
                owner->mDObj->GetMat(BoneParent);
            const math::Mat43& Mat = owner->mDObj->GetMat(v110);
            math::Mat43 v46;
            phys_full_inv_multiply_mat(v46, parentMat, Mat);
            *v4 = MatToQuat(v46);
            *v5 = v46.w;
        LABEL_28:
            if (++v110 >= v80)
                return;
        }
    }
}

// DObjMatriceModelToLocal (0x6FF860) - disasm-driven reconstruction
// XBoneHierarchy view (12 bytes: mName +0, mNameHash +4, mParentIndex +8)
// Inverse-multiply helper matching DObjMatriceModelToLocal's SSE: the parent
// rotation is transposed (orthonormal => inverse) and each row of bone is
// dotted with its columns; the translation row is (bone.w - parent.w) Ãƒâ€šÃ‚Â· cols.
static void InverseMultiplyLocal(const math::Mat43& parent, math::Mat43& bone)
{
    __m128 x = parent.x.v, y = parent.y.v, z = parent.z.v, w = parent.w.v;
    __m128 v11 = _mm_shuffle_ps(x, y, 68);
    __m128 col1 = _mm_shuffle_ps(v11, z, 221);
    __m128 col0 = _mm_shuffle_ps(v11, z, 136);
    __m128 col2 = _mm_shuffle_ps(_mm_shuffle_ps(x, y, 238), z, 168);
    __m128 negw = _mm_xor_ps(Float4_SignMask_12, w);
    __m128 t = _mm_add_ps(
        _mm_add_ps(
            _mm_mul_ps(_mm_shuffle_ps(negw, negw, 0), col0),
            _mm_mul_ps(_mm_shuffle_ps(negw, negw, 85), col1)),
        _mm_mul_ps(_mm_shuffle_ps(negw, negw, 170), col2));

    __m128 r0 = bone.x.v;
    __m128 r1 = bone.y.v;
    __m128 r2 = bone.z.v;
    __m128 r3 = bone.w.v;
    bone.x.v = _mm_add_ps(
        _mm_add_ps(
            _mm_mul_ps(_mm_shuffle_ps(r0, r0, 0), col0),
            _mm_mul_ps(_mm_shuffle_ps(r0, r0, 85), col1)),
        _mm_mul_ps(_mm_shuffle_ps(r0, r0, 170), col2));
    bone.y.v = _mm_add_ps(
        _mm_add_ps(
            _mm_mul_ps(_mm_shuffle_ps(r1, r1, 0), col0),
            _mm_mul_ps(_mm_shuffle_ps(r1, r1, 85), col1)),
        _mm_mul_ps(_mm_shuffle_ps(r1, r1, 170), col2));
    bone.z.v = _mm_add_ps(
        _mm_add_ps(
            _mm_mul_ps(_mm_shuffle_ps(r2, r2, 0), col0),
            _mm_mul_ps(_mm_shuffle_ps(r2, r2, 85), col1)),
        _mm_mul_ps(_mm_shuffle_ps(r2, r2, 170), col2));
    bone.w.v = _mm_add_ps(
        _mm_add_ps(
            _mm_mul_ps(_mm_shuffle_ps(r3, r3, 0), col0),
            _mm_mul_ps(_mm_shuffle_ps(r3, r3, 85), col1)),
        _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(r3, r3, 170), col2), t));
}

// ea: 0x6FF860 (disasm-driven reconstruction; Hex-Rays output was garbled)
void DObjMatriceModelToLocal(Entity* owner)
{
    DObj* mDObj = owner->mDObj;
    DObjSkelMat* MatrixArray = DObjGetMatrixArray(mDObj, 0);
    int modelIndex = (int)mDObj->numModels - 1;
    if (modelIndex < 0)
        return;
    for (; modelIndex >= 0; --modelIndex)
    {
        // models[] is IVPointer<XModel>[8]: { value, pakId } per slot
        void** model_slot =
            (void**)((char*)mDObj->models + modelIndex * 8);
        XModelLocal* model = (XModelLocal*)model_slot[0];
        TPakId model_pak = (TPakId)(uintptr_t)model_slot[1];
        ValidatePakId(model_pak);

        // Find the first non-null LOD; from its parts derive the hierarchy
        // count and the parts used for the parent-index lookup.
        XModelLodLocal* lod = model->lod[0];
        int i = 0;
        if (lod == nullptr)
        {
            while (i < 4 && model->lod[++i] == nullptr)
                ;
            lod = model->lod[i];
        }
        XModelPartsLocal* parts = nullptr;
        unsigned int hierarchy_count = 0;
        if (lod != nullptr && lod->xmodelParts != nullptr)
        {
            XModelPartsLocal* p = (XModelPartsLocal*)lod->xmodelParts;
            hierarchy_count = p->mHierarchySize;
            parts = p;
            ValidatePakId(model_pak);
        }

        // parts used for the per-bone parent lookup (second scan result)
        XModelPartsLocal* parts2 = parts;
        if (parts2 == nullptr)
        {
            // fall back: re-scan for a LOD with parts
            for (int j = 0; j < 5; ++j)
            {
                if (model->lod[j] != nullptr
                    && model->lod[j]->xmodelParts != nullptr)
                {
                    parts2 = (XModelPartsLocal*)model->lod[j]->xmodelParts;
                    break;
                }
            }
        }
        unsigned int v49 = hierarchy_count;
        if (v49 == 0)
            continue;

        DObjSkelMat* v17 = DObjGetMatrixArray(mDObj, modelIndex);
        int parent_bone = -1;
        unsigned int v16 = v49 - 1;
        for (;;)
        {
            unsigned int idx = v16;
            if (v16 >= parts2->mHierarchySize)
            {
                AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
                AeAssert::gCurrentFile = "../ae\\inplace/InplaceVector.h";
                AeAssert::gCurrentLine = 81;
                AeAssert::gCurrentExpr = "index < mSize";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert("Bounds check"))
                    __debugbreak();
                if (v16 >= parts2->mHierarchySize)
                    idx = 0;
            }
            int parent =
                parts2->mHierarchy[idx].mParentIndex;
            if (v16 == 0)
            {
                // root bone: parent from model 0's array at modelParent
                unsigned int parent_model = mDObj->modelParents[modelIndex];
                math::Mat43 parent_mat;
                memcpy(&parent_mat, &MatrixArray[parent_model],
                       sizeof(DObjSkelMat));
                math::Mat43 bone;
                memcpy(&bone, &v17[0], sizeof(DObjSkelMat));
                InverseMultiplyLocal(parent_mat, bone);
                memcpy(&v17[0], &bone, sizeof(DObjSkelMat));
            }
            else
            {
                if (parent_bone != parent)
                {
                    parent_bone = parent;
                    math::Mat43 parent_mat;
                    memcpy(&parent_mat, &v17[parent],
                           sizeof(DObjSkelMat));
                    math::Mat43 bone;
                    memcpy(&bone, &v17[v16], sizeof(DObjSkelMat));
                    InverseMultiplyLocal(parent_mat, bone);
                    memcpy(&v17[v16], &bone, sizeof(DObjSkelMat));
                }
                else
                {
                    math::Mat43 parent_mat;
                    memcpy(&parent_mat, &v17[parent_bone],
                           sizeof(DObjSkelMat));
                    math::Mat43 bone;
                    memcpy(&bone, &v17[v16], sizeof(DObjSkelMat));
                    InverseMultiplyLocal(parent_mat, bone);
                    memcpy(&v17[v16], &bone, sizeof(DObjSkelMat));
                }
            }
            if (v16 == 0)
                break;
            --v16;
        }
    }
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

    // ??0biped_system@@QAE@PAVbiped_phys_info@@@Z (physics.o inline 0x71E370)
    biped_system() : biped_system(nullptr) {}
    biped_system(biped_phys_info* bp_info)
    {
        reset_stability();
        reset_ballistic_target();
        bp_bone_array.m_list_qstart.reset_buffer();
        bp_bone_array.m_list_pstart.reset_buffer();
        // callback sphere-list pool (m_collision_callback +0x10 = +0xCA0;
        // slot_array at pool+0xFF0, count at pool+0xFF4)
        *(void**)((char*)this + 0x1A90) = (char*)this + 0xCA0;
        *(int*)((char*)this + 0x1A94) = 0;
        // callback capsule-pair pool (m_collision_callback +0x1010 = +0x1CA0;
        // slot_array at pool+0x70, count at pool+0x74)
        *(void**)((char*)this + 0x1D10) = (char*)this + 0x1CA0;
        *(int*)((char*)this + 0x1D14) = 0;
        m_bp_info = bp_info;
    }

    void prolog_frame_advance(Entity* owner, float delta_t);  // ?prolog_frame_advance@biped_system@@QAEXPAVEntity@@M@Z
    void debug_render();  // ?debug_render@biped_system@@QAEXXZ
    void render_joint(int joint_id, Bitmask<unsigned int> render_flags);  // ?render_joint@biped_system@@QAEXHV?$Bitmask@I@@@Z
    void render_single_joint(const rigid_body_constraint_ragdoll* joint,
                             Bitmask<unsigned int> render_flags);  // ?render_single_joint@biped_system@@QAEXPBVrigid_body_constraint_ragdoll@@V?$Bitmask@I@@@Z
    void setup_initial_gjk_cache(unsigned int geom_id,
                                 const math::Dir3& separation_direction);  // ?setup_initial_gjk_cache@biped_system@@QAEXIABVDir3@math@@@Z
    void destroy_bps(Entity* owner);  // ?destroy_bps@biped_system@@QAEXPAVEntity@@@Z
    void recreate_bps(Entity* owner, int flags);  // ?recreate_bps@biped_system@@QAEXPAVEntity@@H@Z
    void remove_rigid_body(phys_bones rb_id);  // ?remove_rigid_body@biped_system@@QAEXW4phys_bones@@@Z
    void create_bps(Entity* owner, int flags);  // ?create_bps@biped_system@@QAEXPAVEntity@@H@Z
    void epilog_frame_advance(Entity* owner, float delta_t);  // ?epilog_frame_advance@biped_system@@QAEXPAVEntity@@M@Z
    void apply_pulse_to_bone(phys_bones bone_id, const math::Dir3& pulse);  // ?apply_pulse_to_bone@biped_system@@QAEXW4phys_bones@@ABVDir3@math@@@Z
    void apply_pulse_to_bone(phys_bones bone_id, const math::Position3& hitp,
                             const math::Dir3& pulse,
                             float torque_mult);  // ?apply_pulse_to_bone@biped_system@@QAEXW4phys_bones@@ABVPosition3@math@@ABVDir3@4@M@Z
private:
    void initialize_members();  // ?initialize_members@biped_system@@AAEXXZ
    void create_system(biped_phys_info* bp_info);  // ?create_system@biped_system@@AAEXPAVbiped_phys_info@@@Z
    void rdbi_calc_bone_mat_from_rb();  // ?rdbi_calc_bone_mat_from_rb@biped_system@@AAEXXZ
    void update_stability(float delta_t);  // ?update_stability@biped_system@@AAEXM@Z
    void setup_physics(Entity* owner);     // ?setup_physics@biped_system@@AAEXPAVEntity@@@Z
    void remove_system();  // ?remove_system@biped_system@@AAEXXZ
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
    void create_bp_sys(int flags);  // ?create_bp_sys@biped_phys_info@@QAEXH@Z
private:
    void reset_bone_vel_info(float delta_t);   // ?reset_bone_vel_info@biped_phys_info@@AAEXM@Z
    void update_bone_vel_info(float delta_t);  // ?update_bone_vel_info@biped_phys_info@@AAEXM@Z
    bool setup(Entity* owner);                 // ?setup@biped_phys_info@@AAE_NPAVEntity@@@Z
    friend biped_phys_info* create_biped_phys_info(Entity* owner);
    friend void ApplyPhysics(Entity* hitEnt, const math::Position3& hitp,
                             const math::Dir3& hitd, float force,
                             bool local_hitp, ::hitLocation_t hitLoc);
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

// SaveGameData (sv_stubs.h view; mControllerPort inside StubData @ +0x3B0)
struct SaveGameData {
    uint8_t _pad[0x3B0];
    int     mControllerPort;  // +0x3B0 (StubData.mControllerPort)
    uint8_t _rest[7156 - 0x3B4];
};
extern SaveGameData* gSaveGameData;  // ?gSaveGameData@@3PAUSaveGameData@@A (game.o @ 0xF312F0)
char rumble_test = -1;  // physics.o data @ 0xF916F8 (byte 0xFF)

// controller (core.o; minimal view - full impl in input/controller.cpp)
class controller {
public:
    enum ButtonIndex {
        CIRCLE   = 6,  // B button
        TRIANGLE = 7,  // Y button
    };
    enum RumbleIndex {
        RUMBLE_LEFT  = 0,
        RUMBLE_RIGHT = 1,
    };
    static controller* inst();  // ?inst@controller@@SAPAV1@XZ
    int  button_value(int i_controller_num,
                      ButtonIndex i_button);  // ?button_value@controller@@QAEHHW4ButtonIndex@1@@Z
    void rumble(int i_controller_num, RumbleIndex i_motor,
                float intensity);  // ?rumble@controller@@QAEXHW4RumbleIndex@1@M@Z
};

// ea: 0x704290
void RBAdvanceDebug(float delta_t)
{
    (void)delta_t;
    EntityHandleDb::sInst.GetObject(0);
    if (rumble_test && gSaveGameData[currCl].mControllerPort != -1)
    {
        controller* v0 = controller::inst();
        float intensity;
        if (v0->button_value(gSaveGameData[currCl].mControllerPort,
                             controller::TRIANGLE) != 0)
            intensity = 1.0f;
        else
            intensity = 0.0f;
        controller* v1 = controller::inst();
        v1->rumble(gSaveGameData[currCl].mControllerPort,
                   controller::RUMBLE_LEFT, intensity);
        controller* v2 = controller::inst();
        if (v2->button_value(gSaveGameData[currCl].mControllerPort,
                             controller::CIRCLE) != 0)
        {
            controller* v3 = controller::inst();
            v3->rumble(gSaveGameData[currCl].mControllerPort,
                       controller::RUMBLE_RIGHT, 1.0f);
        }
        else
        {
            controller* v4 = controller::inst();
            v4->rumble(gSaveGameData[currCl].mControllerPort,
                       controller::RUMBLE_RIGHT, 0.0f);
        }
    }
}

// ea: 0x7032B0
void rb_prop_system::frame_advance(float delta_t)
{
    rb_extra_info* m_next = nullptr;
    int count = g_list_rb_extra_info.m_alloc_count;
    for (int i = 0; i < count; ++i)
    {
        rb_extra_info* v3 = g_list_rb_extra_info.m_alloc_list[i];
        if (v3->m_ent == nullptr)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\RBPropSys.cpp";
            AeAssert::gCurrentLine = 193;
            AeAssert::gCurrentExpr = "rb_inf->m_ent";
            if (!AeAssert::IsIgnored() && AeAssert::Assert(defaultFileName))
                __debugbreak();
        }
        Entity* m_ent = v3->m_ent;
        if (m_ent->scr_vehicle != nullptr)
        {
            rigid_body* m_rb = v3->m_rb;
            if ((~(m_rb->m_flags >> 6) & 1) == 0
                && _tlAssert(
                       "c:\\cod\\code\\tl\\physics\\include\\rigid_body.h", 79,
                       "debug_flag_is_not_in_collision()", defaultFileName))
                __debugbreak();
            math::Mat43 v12;
            phys_full_multiply_mat(v12, m_rb->m_mat, v3->m_transform);
            MultiplayerMgr::sInst->ApplyLocalPhysicsToVehicle(
                m_ent, &v12, &m_rb->m_t_vel);
        }
        else if (m_ent->flags >= 0)
        {
            rigid_body* v6 = v3->m_rb;
            if ((~(v6->m_flags >> 6) & 1) == 0
                && _tlAssert(
                       "c:\\cod\\code\\tl\\physics\\include\\rigid_body.h", 79,
                       "debug_flag_is_not_in_collision()", defaultFileName))
                __debugbreak();
            phys_full_multiply_mat(m_ent->r.currentMat, v6->m_mat,
                                   v3->m_transform);
            m_ent->CalcOriginAnglesFromMat();
        }
        if (gPhysicsFinder != 0)
        {
            rigid_body* v7 = v3->m_rb;
            if ((~(v7->m_flags >> 6) & 1) == 0
                && _tlAssert(
                       "c:\\cod\\code\\tl\\physics\\include\\rigid_body.h", 79,
                       "debug_flag_is_not_in_collision()", defaultFileName))
                __debugbreak();
            math::Position3 pt1;
            pt1.v = v7->m_mat.w.v;
            math::Position3 pt2;
            pt2.v = _mm_add_ps(
                pt1.v, _mm_setr_ps(0.0f, 0.0f, 3000.0f, 0.0f));
            float col[4] = {1.0f, 1.0f, 0.0f, 1.0f};
            DebugRender_RenderLine(pt1, pt2, col, 5.0f);
        }
        if (v3->m_priority != 0)
        {
            v3->m_time_since_last_event =
                v3->m_time_since_last_event + delta_t;
            if (v3->m_time_since_last_event > 2.0f)
                v3->evaluate_effect_priority();
        }
        if (m_ent->scr_vehicle == nullptr)
        {
            rigid_body* v10 = v3->m_rb;
            if ((v10->m_flags & 8) != 0
                || _mm_shuffle_ps(v10->m_mat.w.v, v10->m_mat.w.v, 170)
                           .m128_f32[0]
                       < -10000.0f)
            {
                v3->m_next = m_next;
                m_next = v3;
            }
        }
    }
    for (rb_extra_info* v11 = m_next; v11 != nullptr;
         v11 = (rb_extra_info*)v11->m_next)
    {
        v11->m_ent->Notify(hash_const.physicsdone);
        rb_prop_system::remove_entity(v11->m_ent);
    }
}

// ea: 0x7035B0
void SetupConstraints(rb_extra_info* rb_inf)
{
    rigid_body* m_rb = rb_inf->m_rb;
    DObj* mDObj = rb_inf->m_ent->mDObj;
    TPakId mPakId = (TPakId)mDObj->mPhysDataPakId;
    PhysData* phys_data = (PhysData*)mDObj->mPhysDataValue;
    ValidatePakId(mPakId);
    if (phys_data != nullptr)
    {
        ValidatePakId(mPakId);
        InplaceVector<PhysConstraint>* maxAngle =
            (InplaceVector<PhysConstraint>*)&phys_data->mConstraints;
        if (maxAngle->mSize == 2)
        {
            ValidatePakId(mPakId);
            float mDist = maxAngle->mList[0].mDist;
            ValidatePakId(mPakId);
            if (maxAngle->mList[1].mDist > mDist)
                mDist = maxAngle->mList[1].mDist;
            if (mDist > 0.0f)
            {
                rigid_body_constraint_distance* rbc_dist =
                    phys_sys::create_rbc_dist(
                        m_rb, phys_sys::get_environment_rigid_body(), true);
                if (rbc_dist == nullptr)
                    return;
                ValidatePakId(mPakId);
                PhysConstraint* max_con = &maxAngle->mList[0];
                ValidatePakId(mPakId);
                PhysConstraint* v7 = &maxAngle->mList[1];
                if (max_con->mDist != mDist)
                {
                    ValidatePakId(mPakId);
                    max_con = &maxAngle->mList[1];
                    ValidatePakId(mPakId);
                    v7 = &maxAngle->mList[0];
                }
                math::Dir3 origin_dir;
                origin_dir.v = _mm_setr_ps(v7->mOrigin[0], v7->mOrigin[1],
                                           v7->mOrigin[2], 0.0f);
                const math::Mat43& mat = m_rb->get_mat();
                math::Dir3 b1_r_loc;
                b1_r_loc.v = _mm_add_ps(
                    _mm_add_ps(
                        _mm_mul_ps(_mm_shuffle_ps(origin_dir.v,
                                                  origin_dir.v, 0),
                                   mat.x.v),
                        _mm_mul_ps(_mm_shuffle_ps(origin_dir.v,
                                                  origin_dir.v, 85),
                                   mat.y.v)),
                    _mm_add_ps(
                        _mm_mul_ps(_mm_shuffle_ps(origin_dir.v,
                                                  origin_dir.v, 170),
                                   mat.z.v),
                        mat.w.v));
                math::Dir3 b2_r_loc;
                b2_r_loc.v = _mm_setr_ps(max_con->mOrigin[0],
                                         max_con->mOrigin[1],
                                         max_con->mOrigin[2], 0.0f);
                rbc_dist->set(b1_r_loc, b2_r_loc, 0.0f, mDist);
                m_rb->m_flags |= 0x100u;
                return;
            }
            rigid_body_constraint_hinge* rbc_hinge =
                phys_sys::create_rbc_hinge(
                    m_rb, phys_sys::get_environment_rigid_body(), true);
            if (rbc_hinge == nullptr)
                return;
            ValidatePakId(mPakId);
            PhysConstraint* c0 = &maxAngle->mList[0];
            math::Position3 p1;
            p1.v = _mm_setr_ps(c0->mOrigin[0], c0->mOrigin[1],
                               c0->mOrigin[2], 0.0f);
            ValidatePakId(mPakId);
            PhysConstraint* c1 = &maxAngle->mList[1];
            math::Position3 p2;
            p2.v = _mm_setr_ps(c1->mOrigin[0], c1->mOrigin[1],
                               c1->mOrigin[2], 0.0f);
            ValidatePakId(mPakId);
            float theta_min = maxAngle->mList[0].mMinAngle;
            ValidatePakId(mPakId);
            float theta_max = maxAngle->mList[0].mMaxAngle;
            math::Dir3 axis;
            __m128 d = _mm_sub_ps(p2.v, p1.v);
            __m128 d2 = _mm_mul_ps(d, d);
            float len =
                d2.m128_f32[0] + (d2.m128_f32[1] + d2.m128_f32[2]);
            len = sqrtf(len);
            axis.v = _mm_div_ps(d, _mm_set1_ps(len));
            const math::Mat43& mat = m_rb->get_mat();
            __m128 x = mat.x.v, y = mat.y.v, z = mat.z.v, w = mat.w.v;
            __m128 v11 = _mm_shuffle_ps(x, y, 68);
            __m128 v12 = _mm_shuffle_ps(v11, z, 221);
            __m128 v13 = _mm_shuffle_ps(v11, z, 136);
            __m128 v15 =
                _mm_shuffle_ps(_mm_shuffle_ps(x, y, 238), z, 168);
            math::Dir3 b1_r_loc;
            b1_r_loc.v = _mm_add_ps(
                _mm_add_ps(
                    _mm_mul_ps(_mm_shuffle_ps(p1.v, p1.v, 0), v13),
                    _mm_mul_ps(_mm_shuffle_ps(p1.v, p1.v, 85), v12)),
                _mm_add_ps(
                    _mm_mul_ps(_mm_shuffle_ps(p1.v, p1.v, 170), v15),
                    _mm_xor_ps(
                        Float4_SignMask_12,
                        _mm_add_ps(
                            _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(w, w, 0),
                                                  v13),
                                       _mm_mul_ps(_mm_shuffle_ps(w, w, 85),
                                                  v12)),
                            _mm_mul_ps(_mm_shuffle_ps(w, w, 170), v15)))));
            math::Dir3 b1_axis_loc;
            b1_axis_loc.v = _mm_add_ps(
                _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(axis.v, axis.v, 0),
                                      v13),
                           _mm_mul_ps(_mm_shuffle_ps(axis.v, axis.v, 85),
                                      v12)),
                _mm_mul_ps(_mm_shuffle_ps(axis.v, axis.v, 170), v15));
            ValidatePakId(mPakId);
            math::Position3 angles;
            angles.v = _mm_setr_ps(maxAngle->mList[0].mAngles[0],
                                   maxAngle->mList[0].mAngles[1],
                                   maxAngle->mList[0].mAngles[2], 0.0f);
            math::Dir3 fwd;
            AnglesToForward(angles, fwd);
            __m128 v32 = axis.v;
            __m128 v33 = _mm_shuffle_ps(v32, v32, 18);
            __m128 v34 = _mm_shuffle_ps(v32, v32, 9);
            __m128 v36 = _mm_sub_ps(
                _mm_mul_ps(v34, _mm_shuffle_ps(fwd.v, fwd.v, 18)),
                _mm_mul_ps(v33, _mm_shuffle_ps(fwd.v, fwd.v, 9)));
            __m128 v38 = _mm_sub_ps(
                _mm_mul_ps(v34, _mm_shuffle_ps(v36, v36, 18)),
                _mm_mul_ps(v33, _mm_shuffle_ps(v36, v36, 9)));
            __m128 v39 = _mm_shuffle_ps(x, y, 68);
            __m128 v40 = _mm_shuffle_ps(v39, z, 221);
            __m128 v41 = _mm_shuffle_ps(v39, z, 136);
            __m128 v42 = _mm_mul_ps(
                _mm_shuffle_ps(v38, v38, 170),
                _mm_shuffle_ps(_mm_shuffle_ps(x, y, 238), z, 168));
            __m128 v43 = _mm_add_ps(
                _mm_mul_ps(_mm_shuffle_ps(v38, v38, 0), v41),
                _mm_mul_ps(_mm_shuffle_ps(v38, v38, 85), v40));
            math::Dir3 b1_ref_loc;
            b1_ref_loc.v = _mm_add_ps(v43, v42);
            math::Dir3 b2_axis_loc;
            b2_axis_loc.v = axis.v;
            math::Dir3 b2_ref_loc;
            b2_ref_loc.v = v38;
            __m128 inv_inertia = m_rb->m_inv_inertia.v;
            __m128 rcp = _mm_rcp_ps(inv_inertia);
            __m128 v46 = _mm_mul_ps(
                _mm_mul_ps(
                    b1_axis_loc.v,
                    _mm_mul_ps(
                        _mm_sub_ps(_mm_set1_ps(2.0f),
                                   _mm_mul_ps(rcp, inv_inertia)),
                        rcp)),
                b1_axis_loc.v);
            float damp =
                v46.m128_f32[0] + (v46.m128_f32[1] + v46.m128_f32[2]);
            ValidatePakId(mPakId);
            damp = damp * maxAngle->mList[0].mDamp;
            rbc_hinge->set(b1_r_loc, reinterpret_cast<const math::Dir3&>(p1),
                           b1_axis_loc, b2_axis_loc,
                           b1_ref_loc, b2_ref_loc,
                           (theta_min * 3.1415927f) * 0.0055555557f,
                           (theta_max * 3.1415927f) * 0.0055555557f, damp);
            if (theta_max < 179.0f && theta_min > -179.0f)
            {
                rbc_hinge->m_flags |= 8u;
                rb_inf->m_flags.mMask &= ~1u;
                m_rb->m_flags |= 0x100u;
                return;
            }
            rbc_hinge->m_flags |= 4u;
        }
        else
        {
            for (int i = 0; i < (int)maxAngle->mSize; ++i)
            {
                ValidatePakId(mPakId);
                PhysConstraint* c = &maxAngle->mList[i];
                math::Position3 origin;
                origin.v = _mm_setr_ps(c->mOrigin[0], c->mOrigin[1],
                                       c->mOrigin[2], 0.0f);
                const math::Mat43& mat = m_rb->get_mat();
                __m128 v52 = mat.x.v, v51 = mat.y.v, v53 = mat.z.v;
                __m128 v57 = mat.w.v;
                __m128 v55 = _mm_shuffle_ps(v52, v51, 68);
                __m128 v56 = _mm_shuffle_ps(v52, v51, 238);
                __m128 v59 = _mm_shuffle_ps(v55, v53, 221);
                __m128 v60 = _mm_shuffle_ps(v55, v53, 136);
                __m128 v61 = _mm_shuffle_ps(v56, v53, 168);
                math::Dir3 world;
                world.v = _mm_add_ps(
                    _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(origin.v,
                                                         origin.v, 0),
                                          v60),
                               _mm_mul_ps(_mm_shuffle_ps(origin.v,
                                                         origin.v, 85),
                                          v59)),
                    _mm_add_ps(
                        _mm_mul_ps(_mm_shuffle_ps(origin.v, origin.v, 170),
                                   v61),
                        _mm_xor_ps(
                            Float4_SignMask_12,
                            _mm_add_ps(
                                _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(v57,
                                                                     v57, 0),
                                                      v60),
                                           _mm_mul_ps(_mm_shuffle_ps(v57,
                                                                     v57, 85),
                                                      v59)),
                                _mm_mul_ps(_mm_shuffle_ps(v57, v57, 170),
                                           v61)))));
                rigid_body_constraint_point* rbc_point =
                    phys_sys::create_rbc_point(
                        m_rb, phys_sys::get_environment_rigid_body(), true);
                if (rbc_point != nullptr)
                    rbc_point->set(world,
                                   reinterpret_cast<const math::Dir3&>(origin));
            }
        }
    }
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

// ea: 0x70D160
void biped_phys_info::epilog_frame_advance(float delta_t)
{
    biped_system* m_bp_sys = this->m_bp_sys;
    if (m_bp_sys != nullptr)
    {
        m_bp_sys->epilog_frame_advance(m_owner, delta_t);
        if (m_bp_sys->m_is_stable
            || m_owner->r.currentOrigin.v.m128_f32[2] < -60000.0f)
        {
            if (m_owner->actor != nullptr)
            {
                AeAssert::gCurrentAuthor = AeAssert::JRS;
                AeAssert::gCurrentFile = "c:\\cod\\code\\game\\RBRagdoll.cpp";
                AeAssert::gCurrentLine = 205;
                AeAssert::gCurrentExpr = "!m_owner->actor";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert(
                           "Ragdoll physics done but actor still alive."))
                    __debugbreak();
            }
            m_owner->Notify(hash_const.physicsdone);
            destroy_bp_sys(false);
        }
    }
}

// ea: 0x6F23B0
void biped_system::apply_pulse_to_bone(phys_bones bone_id,
                                       const math::Dir3& pulse)
{
    if (!g_in_physics_collision_callback)
    {
        math::Dir3 v4;
        v4.v = _mm_mul_ps(pulse.v, _mm_set1_ps(g_ragdoll_mass_scale));
        apply_pulse((int)bone_id, &v4);
    }
}
// ea: 0x6F24D0
void biped_system::apply_pulse_to_bone(phys_bones bone_id,
                                       const math::Position3& hitp,
                                       const math::Dir3& pulse,
                                       float torque_mult)
{
    if (!g_in_physics_collision_callback)
    {
        math::Dir3 v7;
        v7.v = hitp.v;
        math::Dir3 v8;
        v8.v = _mm_mul_ps(pulse.v, _mm_set1_ps(g_ragdoll_mass_scale));
        apply_pulse((int)bone_id, &v7, &v8, torque_mult);
    }
}

// ea: 0x70D210
void biped_phys_info::epilog_frame_advance_all(float delta_t)
{
    int count = g_list_biped_phys_info.m_alloc_count;
    for (int i = 0; i < count; ++i)
        g_list_biped_phys_info.m_alloc_list[i]->epilog_frame_advance(delta_t);
}

// ea: 0x70B190
void biped_phys_info::create_bp_sys(int flags)
{
    if (m_owner == nullptr
        && _tlAssert("c:\\cod\\code\\game\\RBRagdoll.cpp", 364, "m_owner",
                     defaultFileName))
        __debugbreak();
    biped_system* m_bp_sys = this->m_bp_sys;
    if (m_bp_sys != nullptr)
    {
        m_bp_sys->m_flags = flags;
        m_bp_sys->remove_all_user_rigid_body();
        m_bp_sys->reset_stability();
        m_bp_sys->reset_ballistic_target();
        m_bp_sys->m_is_stable = false;
        m_bp_sys->m_stable_timer = 0.0f;
    }
    else
    {
        biped_system* v5;
        if (g_list_biped_system.m_alloc_count < 16)
            v5 = g_list_biped_system
                     .m_alloc_list[g_list_biped_system.m_alloc_count++];
        else
        {
            tlFatal("phys memory pool add_nc overflow.");
            v5 = nullptr;
        }
        biped_system* v6 = nullptr;
        if (v5 != nullptr)
            v6 = new (v5) biped_system(this);
        this->m_bp_sys = v6;
        if (v6 == nullptr
            && _tlAssert("c:\\cod\\code\\game\\RBRagdoll.cpp", 372, "m_bp_sys",
                         defaultFileName))
            __debugbreak();
        this->m_bp_sys->create_bps(this->m_owner, flags);
        this->m_owner->flags |= 0x410000;
    }
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

// ea: 0x6F76A0
void phys_anim_bone_array::copy_back_bones(Entity* owner)
{
    if ((int)owner->mDObj->numBones
        > m_list_phys_anim_bone.m_alloc_count)
    {
        AeAssert::gCurrentAuthor = AeAssert::JRS;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\RBRagdoll.cpp";
        AeAssert::gCurrentLine = 404;
        AeAssert::gCurrentExpr =
            "owner->GetDObj()->GetNumBones() <= m_list_phys_anim_bone.get_count()";
        if (!AeAssert::IsIgnored() && AeAssert::Assert(defaultFileName))
            __debugbreak();
    }
    int numBones = owner->mDObj->numBones;
    int m_alloc_count = numBones;
    if (numBones > m_list_phys_anim_bone.m_alloc_count)
    {
        m_alloc_count = m_list_phys_anim_bone.m_alloc_count;
        numBones = m_list_phys_anim_bone.m_alloc_count;
    }
    int parentBoneIndex = 0;
    if (numBones > 0)
    {
        for (;;)
        {
            int BoneParent = owner->mDObj->GetBoneParent(parentBoneIndex);
            if ((parentBoneIndex < 0
                 || parentBoneIndex
                        >= m_list_phys_anim_bone.m_alloc_count)
                && _tlAssert(
                       "c:\\cod\\code\\tl\\physics\\include\\phys_array_base.inc",
                       108, "i >= 0 && i < m_alloc_count", defaultFileName))
                __debugbreak();
            phys_anim_bone* bone =
                &m_list_phys_anim_bone.m_slot_array[parentBoneIndex];
            if (bone->m_rb_index == -1)
            {
                const math::Mat43& Mat = owner->mDObj->GetMat(BoneParent);
                math::Mat43 out;
                out.x.v = _mm_add_ps(
                    _mm_add_ps(
                        _mm_mul_ps(
                            _mm_shuffle_ps(bone->mat_loc.x.v,
                                           bone->mat_loc.x.v, 0),
                            Mat.x.v),
                        _mm_mul_ps(
                            _mm_shuffle_ps(bone->mat_loc.x.v,
                                           bone->mat_loc.x.v, 85),
                            Mat.y.v)),
                    _mm_mul_ps(
                        _mm_shuffle_ps(bone->mat_loc.x.v,
                                       bone->mat_loc.x.v, 170),
                        Mat.z.v));
                out.y.v = _mm_add_ps(
                    _mm_add_ps(
                        _mm_mul_ps(
                            _mm_shuffle_ps(bone->mat_loc.y.v,
                                           bone->mat_loc.y.v, 0),
                            Mat.x.v),
                        _mm_mul_ps(
                            _mm_shuffle_ps(bone->mat_loc.y.v,
                                           bone->mat_loc.y.v, 85),
                            Mat.y.v)),
                    _mm_mul_ps(
                        _mm_shuffle_ps(bone->mat_loc.y.v,
                                       bone->mat_loc.y.v, 170),
                        Mat.z.v));
                out.z.v = _mm_add_ps(
                    _mm_add_ps(
                        _mm_mul_ps(
                            _mm_shuffle_ps(bone->mat_loc.z.v,
                                           bone->mat_loc.z.v, 0),
                            Mat.x.v),
                        _mm_mul_ps(
                            _mm_shuffle_ps(bone->mat_loc.z.v,
                                           bone->mat_loc.z.v, 85),
                            Mat.y.v)),
                    _mm_mul_ps(
                        _mm_shuffle_ps(bone->mat_loc.z.v,
                                       bone->mat_loc.z.v, 170),
                        Mat.z.v));
                out.w.v = _mm_add_ps(
                    _mm_add_ps(
                        _mm_mul_ps(
                            _mm_shuffle_ps(bone->mat_loc.w.v,
                                           bone->mat_loc.w.v, 0),
                            Mat.x.v),
                        _mm_mul_ps(
                            _mm_shuffle_ps(bone->mat_loc.w.v,
                                           bone->mat_loc.w.v, 85),
                            Mat.y.v)),
                    _mm_add_ps(
                        _mm_mul_ps(
                            _mm_shuffle_ps(bone->mat_loc.w.v,
                                           bone->mat_loc.w.v, 170),
                            Mat.z.v),
                        Mat.w.v));
                math::Mat43& dst = const_cast<math::Mat43&>(
                    owner->mDObj->GetMat(parentBoneIndex));
                dst = out;
            }
            else if (bone->m_rb_index != 0)
            {
                const math::Mat43& v6 = owner->mDObj->GetMat(BoneParent);
                math::Position3 w;
                w.v = _mm_add_ps(
                    _mm_add_ps(
                        _mm_mul_ps(
                            _mm_shuffle_ps(bone->mat_loc.w.v,
                                           bone->mat_loc.w.v, 0),
                            v6.x.v),
                        _mm_mul_ps(
                            _mm_shuffle_ps(bone->mat_loc.w.v,
                                           bone->mat_loc.w.v, 85),
                            v6.y.v)),
                    _mm_add_ps(
                        _mm_mul_ps(
                            _mm_shuffle_ps(bone->mat_loc.w.v,
                                           bone->mat_loc.w.v, 170),
                            v6.z.v),
                        v6.w.v));
                math::Mat43& dst = const_cast<math::Mat43&>(
                    owner->mDObj->GetMat(parentBoneIndex));
                dst.w.v = w.v;
            }
            ++parentBoneIndex;
            if (parentBoneIndex >= m_alloc_count)
                return;
        }
    }
}

// ea: 0x6F8EF0
void phys_anim_bone_array::remove_rigid_body(int rb_index)
{
    int m_alloc_count = m_list_phys_anim_bone.m_alloc_count;
    int v3 = -1;
    int v4 = 0;
    int bone_array_index = -1;
    if (m_list_phys_anim_bone.m_alloc_count > 0)
    {
        int v5 = 0;
        while (v3 == -1)
        {
            if (v5 < 0 || v4 >= m_alloc_count)
            {
                bool v6 = !_tlAssert(
                    "c:\\cod\\code\\tl\\physics\\include\\phys_array_base.inc",
                    108, "i >= 0 && i < m_alloc_count", defaultFileName);
                m_alloc_count = m_list_phys_anim_bone.m_alloc_count;
                if (!v6)
                    __debugbreak();
            }
            if (m_list_phys_anim_bone.m_slot_array[v5].m_rb_index
                == rb_index)
            {
                bone_array_index = v4;
                v3 = v4;
            }
            ++v4;
            ++v5;
            if (v4 >= m_alloc_count)
            {
                if (v3 == -1)
                    return;
                break;
            }
        }
        if ((v3 < 0 || v3 >= m_alloc_count)
            && _tlAssert(
                   "c:\\cod\\code\\tl\\physics\\include\\phys_array_base.inc",
                   108, "i >= 0 && i < m_alloc_count", defaultFileName))
            __debugbreak();
        int v7 = v3;
        m_list_phys_anim_bone.m_slot_array[v3].m_rb_index = -1;
        int v8 = m_list_phys_anim_bone.m_alloc_count;
        int v9 = 0;
        if (m_list_phys_anim_bone.m_alloc_count > 0)
        {
            int v10 = 0;
            do
            {
                if ((v10 < 0 || v9 >= v8)
                    && _tlAssert(
                           "c:\\cod\\code\\tl\\physics\\include\\phys_array_base.inc",
                           108, "i >= 0 && i < m_alloc_count",
                           defaultFileName))
                    __debugbreak();
                if (m_list_phys_anim_bone.m_slot_array[v10]
                        .m_rb_parent_index
                    == rb_index)
                {
                    if ((bone_array_index < 0
                         || bone_array_index
                                >= m_list_phys_anim_bone.m_alloc_count)
                        && _tlAssert(
                               "c:\\cod\\code\\tl\\physics\\include\\phys_array_base.inc",
                               108, "i >= 0 && i < m_alloc_count",
                               defaultFileName))
                        __debugbreak();
                    if ((v10 < 0
                         || v9 >= m_list_phys_anim_bone.m_alloc_count)
                        && _tlAssert(
                               "c:\\cod\\code\\tl\\physics\\include\\phys_array_base.inc",
                               108, "i >= 0 && i < m_alloc_count",
                               defaultFileName))
                        __debugbreak();
                    m_list_phys_anim_bone.m_slot_array[v10]
                        .m_rb_parent_index =
                        m_list_phys_anim_bone.m_slot_array[v7]
                            .m_rb_parent_index;
                }
                v8 = m_list_phys_anim_bone.m_alloc_count;
                ++v9;
                ++v10;
            } while (v9 < m_list_phys_anim_bone.m_alloc_count);
        }
    }
}

// client_bone_info (physics.o RBRagdoll.cpp; 80 bytes)
struct client_bone_info {
    math::Mat43 m_bone_mat_loc;  // +0x00
    Entity*     m_ent;           // +0x40
    int         m_bone_index;    // +0x44
    int         m_rb_index;      // +0x48

    // set - ea: 0x719EE0 (physics.o inline COMDAT)
    void set(const math::Mat43& bone_mat_loc, Entity* const ent,
             int bone_index, int rb_index)
    {
        m_bone_mat_loc = bone_mat_loc;
        m_ent = ent;
        m_bone_index = bone_index;
        m_rb_index = rb_index;
    }
};
static_assert(sizeof(client_bone_info) == 0x50,
              "client_bone_info size mismatch");

// ea: 0x6FA860
void calc_bone_mat_from_rb(Entity* ent, int boneIndex, rigid_body* rb,
                           const math::Mat43* bone_mat_loc)
{
    if ((~(rb->m_flags >> 6) & 1) == 0
        && _tlAssert(
               "c:\\cod\\code\\tl\\physics\\include\\rigid_body.h", 79,
               "debug_flag_is_not_in_collision()", defaultFileName))
        __debugbreak();
    math::Mat43 root_mat;
    phys_full_multiply_mat(root_mat, rb->m_mat, *bone_mat_loc);
    if (boneIndex == 0)
    {
        math::Mat43 worldBoneMat;
        worldBoneMat.x.v = _mm_setr_ps(1.0f, 0.0f, 0.0f, 0.0f);
        worldBoneMat.y.v = _mm_setr_ps(0.0f, 1.0f, 0.0f, 0.0f);
        worldBoneMat.z.v = _mm_setr_ps(0.0f, 0.0f, 1.0f, 0.0f);
        worldBoneMat.w.v = root_mat.w.v;
        ent->r.currentMat.x.v = worldBoneMat.x.v;
        ent->r.currentMat.y.v = worldBoneMat.y.v;
        ent->r.currentMat.z.v = worldBoneMat.z.v;
        ent->r.currentMat.w.v = worldBoneMat.w.v;
        ent->CalcOriginAnglesFromMat();
    }
    math::Mat43* Mat = (math::Mat43*)&ent->mDObj->GetMat(boneIndex);
    phys_full_inv_multiply_mat(*Mat, ent->r.currentMat, root_mat);
}

// ea: 0x719FA0
void biped_system::rdbi_calc_bone_mat_from_rb()
{
    client_bone_info* list = (client_bone_info*)((char*)this + 0x1D20);
    for (int i = 0; i < 10; ++i)
    {
        if (i < 0
            && _tlAssert("c:/cod/code/tl/physics/include\\phys_mem.h", 374,
                         "i >= 0 && i < size", defaultFileName))
            __debugbreak();
        client_bone_info& cbi = list[i];
        if ((cbi.m_rb_index < 0
             || cbi.m_rb_index >= m_list_rigid_body.m_alloc_count)
            && _tlAssert(
                   "c:\\cod\\code\\tl\\physics\\include\\phys_array_base.inc",
                   108, "i >= 0 && i < m_alloc_count", defaultFileName))
            __debugbreak();
        calc_bone_mat_from_rb(
            cbi.m_ent, cbi.m_bone_index,
            m_list_rigid_body.m_slot_array[cbi.m_rb_index],
            &cbi.m_bone_mat_loc);
    }
}

// ea: 0x6FB610
void biped_system::render_joint(int joint_id,
                                Bitmask<unsigned int> render_flags)
{
    if (joint_id > 10)
    {
        for (int i = 0; i < 10; ++i)
        {
            if ((i < 0 || i >= m_joints.m_alloc_count)
                && _tlAssert(
                       "c:\\cod\\code\\tl\\physics\\include\\phys_array_base.inc",
                       108, "i >= 0 && i < m_alloc_count", defaultFileName))
                __debugbreak();
            render_single_joint(m_joints.m_slot_array[i], render_flags);
        }
    }
    else
    {
        render_single_joint(rb_ragdoll_model::get_joint(joint_id),
                            render_flags);
    }
}

// phys_make_rotate (physics.o inline COMDAT 0x716480). Same math as
// pulse_sum.h make_rotate(Dir3,Dir3) but emitted unguarded in physics.o:
// axis = normalize(v1 x v2), then rotate v1 toward v2 by the axis-angle.
void phys_make_rotate(math::Mat43* mat, const math::Dir3& v1,
                      const math::Dir3& v2)
{
    __m128 cross = _mm_sub_ps(
        _mm_mul_ps(_mm_shuffle_ps(v1.v, v1.v, 9),
                   _mm_shuffle_ps(v2.v, v2.v, 18)),
        _mm_mul_ps(_mm_shuffle_ps(v1.v, v1.v, 18),
                   _mm_shuffle_ps(v2.v, v2.v, 9)));
    __m128 c2 = _mm_mul_ps(cross, cross);
    float len_sq = c2.m128_f32[0] + (c2.m128_f32[1] + c2.m128_f32[2]);
    float len = sqrtf(len_sq);
    __m128 axis = _mm_div_ps(cross, _mm_set1_ps(len));
    __m128 dotv = _mm_mul_ps(v1.v, v2.v);
    float dot = dotv.m128_f32[0] + (dotv.m128_f32[1] + dotv.m128_f32[2]);
    float len2 = sqrtf(dot * dot + len * len);
    float inv_len2 = 1.0f / len2;
    math::Dir3 axis_v;
    axis_v.v = axis;
    make_rotate(*mat, axis_v, inv_len2 * dot, inv_len2 * len);
}

// ea: 0x6FAD30
void biped_system::render_single_joint(
    const rigid_body_constraint_ragdoll* joint,
    Bitmask<unsigned int> render_flags)
{
    if (joint == nullptr)
        return;
    rigid_body* b1 = joint->b1;
    rigid_body* b2 = joint->b2;
    unsigned int m_flags = joint->m_flags;
    Color joint_limit_axis(0.5f, 1.0f, 1.0f, 1.0f);
    Color white(0.5f, 0.0f, 1.0f, 0.0f);
    Color red(0.5f, 0.0f, 0.0f, 1.0f);
    Color cone_axis(1.0f, 1.0f, 1.0f, 0.0f);
    Color joint_axis(0.5f, 0.0f, 0.0f, 1.0f);
    int label_x = 420;
    if ((m_flags & 4) != 0)
    {
        if ((render_flags.mMask & 1) == 0)
            goto LABEL_31;
        DebugRender::RenderText("Hinge", 420, 442, joint_limit_axis, 0.0f,
                                1.0f);
        math::Position3 joint_pos;
        joint_pos.v = _mm_add_ps(
            _mm_add_ps(
                _mm_mul_ps(
                    _mm_shuffle_ps(joint->m_b1_r_loc.v,
                                   joint->m_b1_r_loc.v, 0),
                    b1->get_mat().x.v),
                _mm_mul_ps(
                    _mm_shuffle_ps(joint->m_b1_r_loc.v,
                                   joint->m_b1_r_loc.v, 85),
                    b1->get_mat().y.v)),
            _mm_add_ps(
                _mm_mul_ps(
                    _mm_shuffle_ps(joint->m_b1_r_loc.v,
                                   joint->m_b1_r_loc.v, 170),
                    b1->get_mat().z.v),
                b1->get_mat().w.v));
        math::Position3 end_pt;
        __m128 v8 = joint->m_b2_ref_min_loc.v;
        const math::Mat43& mat = b2->get_mat();
        __m128 v9 = _mm_mul_ps(_mm_shuffle_ps(v8, v8, 170), mat.z.v);
        __m128 v11 = _mm_add_ps(
            _mm_mul_ps(_mm_shuffle_ps(v8, v8, 0), mat.x.v),
            _mm_mul_ps(_mm_shuffle_ps(v8, v8, 85), mat.y.v));
        __m128 len9 = _mm_set1_ps(9.0f);
        end_pt.v =
            _mm_add_ps(joint_pos.v, _mm_mul_ps(_mm_add_ps(v11, v9), len9));
        if ((joint->m_flags & 0x10) == 0)
        {
            DebugRender::RenderLine(joint_pos, end_pt, white, 0.69999999f);
        }
        else
        {
            label_x = 510;
            DebugRender::RenderText("Min", 510, 442, joint_axis, 0.0f, 1.0f);
            DebugRender::RenderLine(joint_pos, end_pt, joint_axis,
                                    0.69999999f);
        }
        const math::Mat43& mat2 = b2->get_mat();
        __m128 v8b = joint->m_b2_ref_max_loc.v;
        end_pt.v = _mm_add_ps(
            joint_pos.v,
            _mm_mul_ps(
                _mm_add_ps(
                    _mm_add_ps(
                        _mm_mul_ps(_mm_shuffle_ps(v8b, v8b, 0), mat2.x.v),
                        _mm_mul_ps(_mm_shuffle_ps(v8b, v8b, 85),
                                   mat2.y.v)),
                    _mm_mul_ps(_mm_shuffle_ps(v8b, v8b, 170), mat2.z.v)),
                len9));
        if ((joint->m_flags & 0x20) == 0)
        {
            DebugRender::RenderLine(joint_pos, end_pt, white, 0.69999999f);
        }
        else
        {
            DebugRender::RenderText("MAX", label_x + 90, 442, joint_axis,
                                    0.0f, 1.0f);
            DebugRender::RenderLine(joint_pos, end_pt, joint_axis,
                                    0.69999999f);
        }
        const math::Mat43& mat3 = b1->get_mat();
        __m128 v8c = joint->m_b1_ref_loc.v;
        end_pt.v = _mm_add_ps(
            joint_pos.v,
            _mm_mul_ps(
                _mm_add_ps(
                    _mm_add_ps(
                        _mm_mul_ps(_mm_shuffle_ps(v8c, v8c, 0), mat3.x.v),
                        _mm_mul_ps(_mm_shuffle_ps(v8c, v8c, 85),
                                   mat3.y.v)),
                    _mm_mul_ps(_mm_shuffle_ps(v8c, v8c, 170), mat3.z.v)),
                len9));
        DebugRender::RenderLine(joint_pos, end_pt, cone_axis, 0.69999999f);
    }
    else if ((m_flags & 8) != 0)
    {
        DebugRender::RenderText("Swivel", 420, 442, joint_limit_axis, 0.0f,
                                1.0f);
        label_x = 510;
        const math::Mat43& mat4 = b1->get_mat();
        math::Dir3 axis1;
        axis1.v = _mm_add_ps(
            _mm_add_ps(
                _mm_mul_ps(
                    _mm_shuffle_ps(joint->m_b1_axis_loc.v,
                                   joint->m_b1_axis_loc.v, 0),
                    mat4.x.v),
                _mm_mul_ps(
                    _mm_shuffle_ps(joint->m_b1_axis_loc.v,
                                   joint->m_b1_axis_loc.v, 85),
                    mat4.y.v)),
            _mm_mul_ps(
                _mm_shuffle_ps(joint->m_b1_axis_loc.v,
                               joint->m_b1_axis_loc.v, 170),
                mat4.z.v));
        const math::Mat43& mat5 = b2->get_mat();
        math::Dir3 axis2;
        axis2.v = _mm_add_ps(
            _mm_add_ps(
                _mm_mul_ps(
                    _mm_shuffle_ps(joint->m_b2_axis_loc.v,
                                   joint->m_b2_axis_loc.v, 0),
                    mat5.x.v),
                _mm_mul_ps(
                    _mm_shuffle_ps(joint->m_b2_axis_loc.v,
                                   joint->m_b2_axis_loc.v, 85),
                    mat5.y.v)),
            _mm_mul_ps(
                _mm_shuffle_ps(joint->m_b2_axis_loc.v,
                               joint->m_b2_axis_loc.v, 170),
                mat5.z.v));
        math::Position3 joint_pos;
        joint_pos.v = _mm_add_ps(
            _mm_add_ps(
                _mm_mul_ps(
                    _mm_shuffle_ps(joint->m_b1_r_loc.v,
                                   joint->m_b1_r_loc.v, 0),
                    b1->get_mat().x.v),
                _mm_mul_ps(
                    _mm_shuffle_ps(joint->m_b1_r_loc.v,
                                   joint->m_b1_r_loc.v, 85),
                    b1->get_mat().y.v)),
            _mm_add_ps(
                _mm_mul_ps(
                    _mm_shuffle_ps(joint->m_b1_r_loc.v,
                                   joint->m_b1_r_loc.v, 170),
                    b1->get_mat().z.v),
                b1->get_mat().w.v));
        if ((render_flags.mMask & 2) != 0)
        {
            if (joint->m_joint_limits_count > 0)
            {
                math::Position3 end_pt;
                end_pt.v = _mm_add_ps(
                    joint_pos.v,
                    _mm_mul_ps(axis2.v, _mm_set1_ps(9.0f)));
                DebugRender::RenderLine(joint_pos, end_pt, joint_limit_axis,
                                        0.69999999f);
            }
            int limit_count = joint->m_joint_limits_count;
            for (int i = 0; i < limit_count; ++i)
            {
                Color limit_col(0.0f, 0.0f, 1.0f, 0.5f);
                if (joint->get_joint_limit_active(i) != 0)
                {
                    limit_col = joint_limit_axis;
                }
                if ((~(b1->m_flags >> 6) & 1) == 0
                    && _tlAssert(
                           "c:\\cod\\code\\tl\\physics\\include\\rigid_body.h",
                           79, "debug_flag_is_not_in_collision()",
                           defaultFileName))
                    __debugbreak();
                math::Dir3 limit_axis;
                limit_axis.v = _mm_add_ps(
                    _mm_add_ps(
                        _mm_mul_ps(
                            _mm_shuffle_ps(joint->m_joint_limits[i]
                                               .m_b1_ud_loc.v,
                                           joint->m_joint_limits[i]
                                               .m_b1_ud_loc.v,
                                           0),
                            b1->get_mat().x.v),
                        _mm_mul_ps(
                            _mm_shuffle_ps(joint->m_joint_limits[i]
                                               .m_b1_ud_loc.v,
                                           joint->m_joint_limits[i]
                                               .m_b1_ud_loc.v,
                                           85),
                            b1->get_mat().y.v)),
                    _mm_mul_ps(
                        _mm_shuffle_ps(joint->m_joint_limits[i].m_b1_ud_loc.v,
                                       joint->m_joint_limits[i].m_b1_ud_loc.v,
                                       170),
                        b1->get_mat().z.v));
                math::Position3 end_pt;
                end_pt.v = _mm_add_ps(
                    joint_pos.v, _mm_mul_ps(limit_axis.v, _mm_set1_ps(9.0f)));
                DebugRender::RenderLine(joint_pos, end_pt, limit_col,
                                        0.69999999f);
                float angle = acosf(
                    joint->m_joint_limits[i].m_b1_ud_limit_co_);
                DebugRender::RenderCone(joint_pos, limit_axis, angle, 9.0f,
                                        limit_col);
                char buf[32];
                sprintf(buf, "%d",
                        (int)(acosf(joint->m_joint_limits[i]
                                        .m_b1_ud_limit_co_)
                              * 180.0f * 0.31830987f));
                DebugRender::RenderText(buf, label_x, 442, limit_col, 0.0f,
                                        1.0f);
                label_x += 40;
            }
        }
        if ((render_flags.mMask & 1) != 0)
        {
            math::Mat43 rot;
            phys_make_rotate(&rot, axis1, axis2);
            const math::Mat43& mat6 = b1->get_mat();
            __m128 v22 = _mm_add_ps(
                _mm_add_ps(
                    _mm_mul_ps(
                        _mm_shuffle_ps(joint->m_b1_ref_loc.v,
                                       joint->m_b1_ref_loc.v, 0),
                        mat6.x.v),
                    _mm_mul_ps(
                        _mm_shuffle_ps(joint->m_b1_ref_loc.v,
                                       joint->m_b1_ref_loc.v, 85),
                        mat6.y.v)),
                _mm_mul_ps(
                    _mm_shuffle_ps(joint->m_b1_ref_loc.v,
                                   joint->m_b1_ref_loc.v, 170),
                    mat6.z.v));
            math::Dir3 ref1;
            ref1.v = _mm_add_ps(
                _mm_add_ps(
                    _mm_mul_ps(_mm_shuffle_ps(v22, v22, 0), rot.x.v),
                    _mm_mul_ps(_mm_shuffle_ps(v22, v22, 85), rot.y.v)),
                _mm_mul_ps(_mm_shuffle_ps(v22, v22, 170), rot.z.v));
            const math::Mat43& mat7 = b2->get_mat();
            math::Dir3 ref_min;
            ref_min.v = _mm_add_ps(
                _mm_add_ps(
                    _mm_mul_ps(
                        _mm_shuffle_ps(joint->m_b2_ref_min_loc.v,
                                       joint->m_b2_ref_min_loc.v, 0),
                        mat7.x.v),
                    _mm_mul_ps(
                        _mm_shuffle_ps(joint->m_b2_ref_min_loc.v,
                                       joint->m_b2_ref_min_loc.v, 85),
                        mat7.y.v)),
                _mm_mul_ps(
                    _mm_shuffle_ps(joint->m_b2_ref_min_loc.v,
                                   joint->m_b2_ref_min_loc.v, 170),
                    mat7.z.v));
            const math::Mat43& mat8 = b2->get_mat();
            math::Dir3 ref_max;
            ref_max.v = _mm_add_ps(
                _mm_add_ps(
                    _mm_mul_ps(
                        _mm_shuffle_ps(joint->m_b2_ref_max_loc.v,
                                       joint->m_b2_ref_max_loc.v, 0),
                        mat8.x.v),
                    _mm_mul_ps(
                        _mm_shuffle_ps(joint->m_b2_ref_max_loc.v,
                                       joint->m_b2_ref_max_loc.v, 85),
                        mat8.y.v)),
                _mm_mul_ps(
                    _mm_shuffle_ps(joint->m_b2_ref_max_loc.v,
                                   joint->m_b2_ref_max_loc.v, 170),
                    mat8.z.v));
            __m128 len9b = _mm_set1_ps(9.0f);
            math::Position3 end_pt;
            end_pt.v = _mm_add_ps(joint_pos.v, _mm_mul_ps(ref1.v, len9b));
            DebugRender::RenderLine(joint_pos, end_pt, cone_axis,
                                    0.69999999f);
            end_pt.v = _mm_add_ps(joint_pos.v, _mm_mul_ps(ref_min.v, len9b));
            if ((joint->m_flags & 0x10) != 0)
            {
                DebugRender::RenderText("MIN", label_x, 442, joint_axis, 0.0f,
                                        1.0f);
                DebugRender::RenderLine(joint_pos, end_pt, joint_axis,
                                        0.69999999f);
                label_x += 40;
            }
            else
            {
                DebugRender::RenderLine(joint_pos, end_pt, white,
                                        0.69999999f);
            }
            end_pt.v = _mm_add_ps(joint_pos.v, _mm_mul_ps(ref_max.v, len9b));
            if ((joint->m_flags & 0x20) != 0)
            {
                DebugRender::RenderText("MAX", label_x, 442, joint_axis, 0.0f,
                                        1.0f);
                DebugRender::RenderLine(joint_pos, end_pt, joint_axis,
                                        0.69999999f);
                goto LABEL_31;
            }
            DebugRender::RenderLine(joint_pos, end_pt, white, 0.69999999f);
        }
    }
LABEL_31:
    if ((render_flags.mMask & 4) != 0)
    {
        render_single_rigid_body(b1);
        render_single_rigid_body(b2);
    }
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

// ?PHYSICS_GRAVITY_SCALE_1@@3MB (physics.o data @ 0xE01F08)
float PHYSICS_GRAVITY_SCALE_1 = 42.5f;

// ea: 0x6FAA60
void biped_system::update_stability(float delta_t)
{
    for (int i = 0; i < 10; ++i)
    {
        if ((i < 0 || i >= m_joints.m_alloc_count)
            && _tlAssert(
                   "c:\\cod\\code\\tl\\physics\\include\\phys_array_base.inc",
                   108, "i >= 0 && i < m_alloc_count", defaultFileName))
            __debugbreak();
        rigid_body_constraint_ragdoll* v4 = m_joints.m_slot_array[i];
        if (v4 != nullptr)
        {
            float damp_k = g_ragdoll_mass_scale * 25.0f;
            switch (i)
            {
            case 0:
            case 2:
                damp_k = damp_k * 0.5f;
                break;
            case 4:
            case 6:
                damp_k = damp_k * 0.80000001f;
                break;
            default:
                break;
            }
            v4->set_damp_k(damp_k);
            v4->m_flags |= 0x200u;
        }
    }
    rb_ragdoll_model::update_stability(delta_t);
    bool m_stable = this->m_stable;
    this->m_is_stable = false;
    if (m_stable)
    {
        float v7 = m_stable_timer + delta_t;
        m_stable_timer = v7;
        if (v7 >= 2.0f)
            m_is_stable = true;
        float turn_off_factor;
        float v8 = 1.0f - ((2.0f - v7) * 0.5f);
        if (v8 >= 0.0f)
        {
            turn_off_factor = 1.0f;
            if (v8 <= 1.0f)
                turn_off_factor = v8;
        }
        else
        {
            turn_off_factor = 0.0f;
        }
        float gravity_multiplier =
            (1.0f - turn_off_factor) * PHYSICS_GRAVITY_SCALE_1;
        for (int j = 0; j < 10; ++j)
        {
            if ((j < 0 || j >= m_list_rigid_body.m_alloc_count)
                && _tlAssert(
                       "c:\\cod\\code\\tl\\physics\\include\\phys_array_base.inc",
                       108, "i >= 0 && i < m_alloc_count", defaultFileName))
                __debugbreak();
            rigid_body* v10 = m_list_rigid_body.m_slot_array[j];
            if ((j < 0 || j >= m_joints.m_alloc_count)
                && _tlAssert(
                       "c:\\cod\\code\\tl\\physics\\include\\phys_array_base.inc",
                       108, "i >= 0 && i < m_alloc_count", defaultFileName))
                __debugbreak();
            rigid_body_constraint_ragdoll* v11 = m_joints.m_slot_array[j];
            if (v10 != nullptr)
                v10->m_gravity_multiplier = gravity_multiplier;
            if (v11 != nullptr)
            {
                float v12 = g_ragdoll_mass_scale * 25.0f;
                switch (j)
                {
                case 0:
                case 2:
                    v12 = v12 * 0.5f;
                    break;
                case 4:
                case 6:
                    v12 = v12 * 0.80000001f;
                    break;
                default:
                    break;
                }
                v11->set_damp_k(
                    ((g_ragdoll_mass_scale * turn_off_factor) * 50.0f)
                    + v12);
            }
        }
    }
    else
    {
        m_stable_timer = 0.0f;
    }
}

// float constants for the copy_back_tween acos/sin approximations
static const __m128 Float4_SinCoefs_12 =
    _mm_setr_ps(-0.16666667f, 0.0083333338f, -0.00019841269f, 0.0f);
static const math::Dir3 Float4_XAxis_12 = {
    _mm_setr_ps(1.0f, 0.0f, 0.0f, 0.0f)
};
static const math::Dir3 Float4_YAxis_12 = {
    _mm_setr_ps(0.0f, 1.0f, 0.0f, 0.0f)
};
static const math::Dir3 Float4_ZAxis_12 = {
    _mm_setr_ps(0.0f, 0.0f, 1.0f, 0.0f)
};

// IS_BAD_NUMBER (physics.o inline 0x716350)
bool IS_BAD_NUMBER(float x)
{
    return ((*(unsigned int*)&x) & 0x7F800000) == 0x7F800000
           || x != x || fabs(x) > 1000000.0f;
}

// ea: 0x6F79B0
void phys_anim_bone_array::copy_back_tween(Entity* owner, float t_)
{
    int numBones = owner->mDObj->numBones;
    int m_alloc_count = m_list_qstart.m_alloc_count;
    int v66 = numBones;
    if (numBones > m_alloc_count)
    {
        v66 = m_alloc_count;
        numBones = m_alloc_count;
    }
    int v6 = 0;
    if (numBones > 0)
    {
        __m128 v47 = _mm_set1_ps(t_);
        __m128 v48 = _mm_set1_ps(1.0f - t_);
        int v69 = 0;
        for (int i = 0;; ++i)
        {
            int BoneParent = owner->mDObj->GetBoneParent(v6);
            if ((i < 0 || v6 >= m_list_phys_anim_bone.m_alloc_count)
                && _tlAssert(
                       "c:\\cod\\code\\tl\\physics\\include\\phys_array_base.inc",
                       108, "i >= 0 && i < m_alloc_count", defaultFileName))
                __debugbreak();
            phys_anim_bone* bone_count =
                &m_list_phys_anim_bone.m_slot_array[i];
            if ((i < 0 || v6 >= m_list_qstart.m_alloc_count)
                && _tlAssert(
                       "c:\\cod\\code\\tl\\physics\\include\\phys_array_base.inc",
                       108, "i >= 0 && i < m_alloc_count", defaultFileName))
                __debugbreak();
            if ((i < 0 || v6 >= m_list_pstart.m_alloc_count)
                && _tlAssert(
                       "c:\\cod\\code\\tl\\physics\\include\\phys_array_base.inc",
                       108, "i >= 0 && i < m_alloc_count", defaultFileName))
                __debugbreak();
            int m_rb_index = bone_count->m_rb_index;
            __m128 v52 = _mm_add_ps(
                _mm_mul_ps(m_list_pstart.m_slot_array[v69].v, v48),
                _mm_mul_ps(bone_count->mat_loc.w.v, v47));
            if (m_rb_index == -1)
                goto slerp;
            if (m_rb_index != 0)
            {
                const math::Mat43& Mat = owner->mDObj->GetMat(BoneParent);
                math::Position3 plerp;
                plerp.v = _mm_add_ps(
                    _mm_add_ps(
                        _mm_mul_ps(_mm_shuffle_ps(v52, v52, 0), Mat.x.v),
                        _mm_mul_ps(_mm_shuffle_ps(v52, v52, 85), Mat.y.v)),
                    _mm_add_ps(
                        _mm_mul_ps(_mm_shuffle_ps(v52, v52, 170), Mat.z.v),
                        Mat.w.v));
                math::Mat43& dst = const_cast<math::Mat43&>(
                    owner->mDObj->GetMat(v6));
                dst.w.v = plerp.v;
            }
            ++v6;
            ++v69;
            if (v6 >= v66)
                return;
            continue;

        slerp:
            const math::Quaternion& qstart_q = m_list_qstart.m_slot_array[v69];
            __m128 qstart = _mm_setr_ps(qstart_q.x, qstart_q.y, qstart_q.z,
                                        qstart_q.w);
            __m128 qend = _mm_setr_ps(bone_count->qend.x, bone_count->qend.y,
                                      bone_count->qend.z, bone_count->qend.w);
            __m128 v15 = _mm_mul_ps(qstart, qend);
            float dot = v15.m128_f32[0]
                + (v15.m128_f32[1] + (v15.m128_f32[2] + v15.m128_f32[3]));
            __m128 v18;
            float d = dot;
            if (dot >= 0.0f)
            {
                v18 = _mm_setr_ps(1.0f - t_, t_, 1.0f, 0.0f);
            }
            else
            {
                v18 = _mm_setr_ps(1.0f - t_, 0.0f - t_, 1.0f, 0.0f);
                d = 0.0f - dot;
            }
            float theta;
            if (d < 0.99999899f)
            {
                if (d >= 0.5f)
                {
                    float s = sqrtf((1.0f - d) * 0.5f);
                    float s2 = s * s;
                    float s3 = s2 * s;
                    float s5 = s3 * s2;
                    theta = ((((s5 * s2) * 0.1079625f)
                              + (s5 * 0.15000001f))
                             + (s3 * 0.33333331f))
                        + (s * 2.0f);
                }
                else
                {
                    float d2 = d * d;
                    float d3 = d2 * d;
                    float d4 = d2 * d2;
                    theta = ((((d4 * d2) * -0.053981241f)
                              - (d4 * 0.075000003f))
                             - (d3 * 0.1666667f))
                        - d
                        + 1.570796f;
                }
                __m128 v27 = _mm_mul_ps(v18, _mm_set1_ps(theta));
                __m128 v28 = _mm_mul_ps(v27, v27);
                __m128 v29 = _mm_mul_ps(v28, v27);
                __m128 v30 = _mm_mul_ps(v28, v29);
                __m128 v31 = _mm_add_ps(
                    _mm_add_ps(
                        v27,
                        _mm_mul_ps(
                            _mm_mul_ps(v28, v30),
                            _mm_shuffle_ps(Float4_SinCoefs_12,
                                           Float4_SinCoefs_12, 170))),
                    _mm_mul_ps(v30,
                               _mm_shuffle_ps(Float4_SinCoefs_12,
                                              Float4_SinCoefs_12, 85)));
                qstart = _mm_setr_ps(qstart_q.x, qstart_q.y, qstart_q.z,
                                     qstart_q.w);
                __m128 v32 = _mm_add_ps(
                    v31,
                    _mm_mul_ps(v29,
                               _mm_shuffle_ps(Float4_SinCoefs_12,
                                              Float4_SinCoefs_12, 0)));
                float v64 = v32.m128_f32[2];
                v18 = _mm_div_ps(v32, _mm_set1_ps(v64));
            }
            __m128 v33 = _mm_add_ps(
                _mm_mul_ps(qstart, _mm_shuffle_ps(v18, v18, 0)),
                _mm_mul_ps(qend, _mm_shuffle_ps(v18, v18, 85)));
            float v63 = v33.m128_f32[3];
            __m128 v34 = _mm_mul_ps(v33, v33);
            float len = sqrtf(v34.m128_f32[0]
                              + (v34.m128_f32[1] + v34.m128_f32[2]));
            __m128 v36;
            if (len == 0.0f)
                v36 = Float4_XAxis_12.v;
            else
                v36 = _mm_div_ps(v33, _mm_set1_ps(0.0f - len));
            __m128 v37 = _mm_xor_ps(Float4_SignMask_12, v36);
            float ql = len;
            float w2_w = (v63 * v63) - (ql * ql);
            __m128 v39 = _mm_mul_ps(v37, _mm_set1_ps(w2_w));
            __m128 v40 = _mm_sub_ps(v37, v39);
            float wq2 = (v63 * 2.0f) * ql;
            __m128 v41 = _mm_mul_ps(v37, _mm_set1_ps(wq2));
            __m128 v42 = _mm_xor_ps(Float4_SignMask_12, v41);
            __m128 r0 = _mm_setr_ps(
                (v63 * v63) - (ql * ql), v41.m128_f32[2], v42.m128_f32[1],
                v42.m128_f32[1]);
            __m128 r1 = _mm_setr_ps(v41.m128_f32[1], w2_w,
                                    v41.m128_f32[0], v41.m128_f32[0]);
            __m128 r2 = _mm_setr_ps(v42.m128_f32[0], v41.m128_f32[0],
                                    w2_w, v42.m128_f32[0]);
            __m128 v49 = _mm_add_ps(
                r0, _mm_mul_ps(v40, _mm_shuffle_ps(v37, v37, 0)));
            __m128 v50 = _mm_add_ps(
                r1, _mm_mul_ps(v40, _mm_shuffle_ps(v37, v37, 85)));
            __m128 v51 = _mm_add_ps(
                r2, _mm_mul_ps(v40, _mm_shuffle_ps(v37, v37, 170)));
            const math::Mat43& v43 = owner->mDObj->GetMat(BoneParent);
            __m128 y = v43.y.v;
            __m128 z = v43.z.v;
            __m128 v54 = _mm_add_ps(
                _mm_add_ps(
                    _mm_mul_ps(_mm_shuffle_ps(v49, v49, 0), v43.x.v),
                    _mm_mul_ps(_mm_shuffle_ps(v49, v49, 85), y)),
                _mm_mul_ps(_mm_shuffle_ps(v49, v49, 170), z));
            math::Dir3 v55;
            v55.v = _mm_add_ps(
                _mm_add_ps(
                    _mm_mul_ps(_mm_shuffle_ps(v50, v50, 0), v43.x.v),
                    _mm_mul_ps(_mm_shuffle_ps(v50, v50, 85), y)),
                _mm_mul_ps(_mm_shuffle_ps(v50, v50, 170), z));
            math::Dir3 v56;
            v56.v = _mm_add_ps(
                _mm_add_ps(
                    _mm_mul_ps(_mm_shuffle_ps(v51, v51, 0), v43.x.v),
                    _mm_mul_ps(_mm_shuffle_ps(v51, v51, 85), y)),
                _mm_mul_ps(_mm_shuffle_ps(v51, v51, 170), z));
            __m128 v57 = _mm_add_ps(
                _mm_add_ps(
                    _mm_mul_ps(_mm_shuffle_ps(v52, v52, 0), v43.x.v),
                    _mm_mul_ps(_mm_shuffle_ps(v52, v52, 85), y)),
                _mm_add_ps(
                    _mm_mul_ps(_mm_shuffle_ps(v52, v52, 170), z),
                    v43.w.v));
            math::Mat43& v46 = const_cast<math::Mat43&>(
                owner->mDObj->GetMat(v6));
            v46.x.v = v54;
            v46.y.v = v55.v;
            v46.z.v = v56.v;
            v46.w.v = v57;
            ++v6;
            ++v69;
            if (v6 >= v66)
                return;
        }
    }
}

// ea: 0x6F8E20
int phys_anim_bone_array::get_bone(int rb_index)
{
    int m_alloc_count = m_list_phys_anim_bone.m_alloc_count;
    for (int i = 0; i < m_alloc_count; ++i)
    {
        if ((i < 0 || i >= m_alloc_count)
            && _tlAssert(
                   "c:\\cod\\code\\tl\\physics\\include\\phys_array_base.inc",
                   108, "i >= 0 && i < m_alloc_count", defaultFileName))
            __debugbreak();
        if (m_list_phys_anim_bone.m_slot_array[i].m_rb_index == rb_index)
        {
            if ((i < 0 || i >= m_alloc_count)
                && _tlAssert(
                       "c:\\cod\\code\\tl\\physics\\include\\phys_array_base.inc",
                       108, "i >= 0 && i < m_alloc_count", defaultFileName))
                __debugbreak();
            return m_list_phys_anim_bone.m_slot_array[i].mBoneIndex;
        }
    }
    if (_tlAssert(
            "c:\\cod\\code\\tl\\physics\\include\\phys_array_base.inc", 108,
            "i >= 0 && i < m_alloc_count", defaultFileName))
        __debugbreak();
    return m_list_phys_anim_bone.m_slot_array[0].mBoneIndex;
}

// ea: 0x6F8CB0
int phys_anim_bone_array::get_phys_bone(int bone_id)
{
    int m_alloc_count = m_list_phys_anim_bone.m_alloc_count;
    int v3 = 0;
    if (m_alloc_count <= 0)
        return -1;
    for (int i = 0;; ++i)
    {
        if ((i < 0 || v3 >= m_alloc_count)
            && _tlAssert(
                   "c:\\cod\\code\\tl\\physics\\include\\phys_array_base.inc",
                   108, "i >= 0 && i < m_alloc_count", defaultFileName))
            __debugbreak();
        if (m_list_phys_anim_bone.m_slot_array[i].mBoneIndex == bone_id)
            break;
        m_alloc_count = m_list_phys_anim_bone.m_alloc_count;
        if (++v3 >= m_alloc_count)
            return -1;
    }
    if ((v3 < 0 || v3 >= m_alloc_count)
        && _tlAssert(
               "c:\\cod\\code\\tl\\physics\\include\\phys_array_base.inc", 108,
               "i >= 0 && i < m_alloc_count", defaultFileName))
        __debugbreak();
    int v6 = v3;
    if (m_list_phys_anim_bone.m_slot_array[v3].m_rb_index == -1)
    {
        if ((v3 < 0 || v3 >= m_alloc_count)
            && _tlAssert(
                   "c:\\cod\\code\\tl\\physics\\include\\phys_array_base.inc",
                   108, "i >= 0 && i < m_alloc_count", defaultFileName))
            __debugbreak();
        if (m_list_phys_anim_bone.m_slot_array[v6].m_rb_parent_index < 0
            && _tlAssert("c:\\cod\\code\\game\\RBRagdoll.cpp", 618,
                         "m_list_phys_anim_bone[i].m_rb_parent_index >= 0",
                         defaultFileName))
            __debugbreak();
        if ((v3 < 0 || v3 >= m_alloc_count)
            && _tlAssert(
                   "c:\\cod\\code\\tl\\physics\\include\\phys_array_base.inc",
                   108, "i >= 0 && i < m_alloc_count", defaultFileName))
            __debugbreak();
        return m_list_phys_anim_bone.m_slot_array[v6].m_rb_parent_index;
    }
    else
    {
        if ((v3 < 0 || v3 >= m_alloc_count)
            && _tlAssert(
                   "c:\\cod\\code\\tl\\physics\\include\\phys_array_base.inc",
                   108, "i >= 0 && i < m_alloc_count", defaultFileName))
            __debugbreak();
        return m_list_phys_anim_bone.m_slot_array[v6].m_rb_index;
    }
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

// ea: 0x6FA800
void biped_system::remove_system()
{
    rb_ragdoll_model::remove_all_user_rigid_body();
    int count = m_list_rigid_body.m_alloc_count;
    for (int i = 0; i < 10; ++i)
    {
        if ((i < 0 || i >= count)
            && _tlAssert(
                   "c:\\cod\\code\\tl\\physics\\include\\phys_array_base.inc",
                   108, "i >= 0 && i < m_alloc_count", defaultFileName))
            __debugbreak();
        if (m_list_rigid_body.m_slot_array[i] != nullptr)
            phys_sys::destroy(m_list_rigid_body.m_slot_array[i]);
    }
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

// ?m_bone_mass_info@@3PAVbone_mass_info@@A (physics.o data @ 0xF8C160)
bone_mass_info m_bone_mass_info[10];

// Ragdoll local-frame rotate used by setup_bone_mass_info's scaling tail:
// x*-X + y*Z + z*Y => (-x, z, y) (constants 0xBF800000 / v38=(0,0,1,0) /
// 0x3F80000000000000uLL from 0x6FA62E..0x6FA6D4).
static __m128 RagdollRotate(__m128 v, __m128 y_axis)
{
    return _mm_add_ps(
        _mm_add_ps(
            _mm_mul_ps(_mm_shuffle_ps(v, v, 0),
                       _mm_setr_ps(-1.0f, 0.0f, 0.0f, 0.0f)),
            _mm_mul_ps(_mm_shuffle_ps(v, v, 85), y_axis)),
        _mm_mul_ps(_mm_shuffle_ps(v, v, 170),
                   _mm_setr_ps(0.0f, 1.0f, 0.0f, 0.0f)));
}

// ea: 0x6F94D0
void setup_bone_mass_info(Entity* owner)
{
    setup_user_bone_ids(owner);
    memset(m_bone_mass_info, 0, sizeof(m_bone_mass_info));
    math::Dir3 v46, v47, v48, v49;
    math::Position3 v50;
    v49.v = _mm_setzero_ps();
    v48.v = _mm_setzero_ps();
    v47.v = _mm_setzero_ps();
    v50.v = _mm_setzero_ps();
    v46.v = _mm_setzero_ps();
    int v2 = USER_BONE_ID_HEAD;
    m_bone_mass_info[0].set(
        USER_BONE_ID_PELVIS, USER_BONE_ID_HEAD, 0.2f, 0.22499999f, 0.5f,
        v50, 2, 300.0f, 0.30000001f, 1.0f, 0, -1,
        bone_mass_info::JOINT_TYPE_NONE, 0.0f, 0.0f, v46, v47, v48, v49,
        0.0f);
    float v3 = dampValue;
    m_bone_mass_info[0].m_damp_k = dampValue;
    m_bone_mass_info[0].m_capsule_b1_adjust_p2_loc.v = _mm_setzero_ps();
    m_bone_mass_info[0].m_capsule_radius = 0.18000001f;
    m_bone_mass_info[0].m_capsule_b1_adjust_p2_loc.v.m128_f32[0] =
        -0.15000001f;
    v46.v = _mm_setr_ps(1.0f, 0.0f, 0.0f, 0.0f);
    v47.v = _mm_setr_ps(1.0f, 0.0f, 0.0f, 0.0f);
    v48.v = _mm_setr_ps(-1.0f, 0.0f, 0.0f, 0.0f);
    v49.v = _mm_setr_ps(-1.0f, 0.0f, 0.0f, 0.0f);
    v50.v = _mm_setr_ps(-0.34999999f, 0.0f, 0.0f, 0.0f);
    m_bone_mass_info[1].set(
        v2, v2, 0.15000001f, 0.15000001f, 0.5f, v50, 1, 50.0f,
        0.30000001f, 0.25f, 1, 0, bone_mass_info::JOINT_TYPE_SWIVEL,
        -0.75f, 0.75f, v49, v48, v47, v46, 1.0f);
    v50.v.m128_f32[2] = 0.0f;
    v50.v.m128_f32[3] = 0.0f;
    math::Dir3* v5 = &m_bone_mass_info[1].m_joint_limit_axis[
        m_bone_mass_info[1].m_joint_limit_count];
    v5->v = _mm_setr_ps(-1.0f, 0.0f, v50.v.m128_f32[2],
                        v50.v.m128_f32[3]);
    m_bone_mass_info[1].m_joint_limit_angle[
        m_bone_mass_info[1].m_joint_limit_count++] = 0.87266463f;
    v50.v = _mm_setzero_ps();
    m_bone_mass_info[1].m_capsule_b1_adjust_p2_loc.v = _mm_setzero_ps();
    v46.v = _mm_setr_ps(1.0f, 0.0f, 0.0f, 0.0f);
    v47.v = _mm_setr_ps(1.0f, 0.0f, 0.0f, 0.0f);
    v48.v = _mm_setr_ps(-1.0f, 0.0f, 0.0f, 0.0f);
    v49.v = _mm_setr_ps(-1.0f, 0.0f, 0.0f, 0.0f);
    m_bone_mass_info[1].m_capsule_radius = 0.0f;
    int v8 = USER_BONE_ID_LEFT_FOREARM;
    m_bone_mass_info[2].set(
        USER_BONE_ID_LEFT_UPPERARM, USER_BONE_ID_LEFT_FOREARM, 0.1f, 0.1f,
        0.5f, v50, 2, 50.0f, 0.30000001f, 1.0f, 2, 0,
        bone_mass_info::JOINT_TYPE_SWIVEL, -1.6f, -0.2f, v49, v48, v47, v46,
        1.0f);
    math::Dir3* v9p = &m_bone_mass_info[2].m_joint_limit_axis[
        m_bone_mass_info[2].m_joint_limit_count];
    m_bone_mass_info[2].m_damp_k = v3;
    v50.v.m128_f32[2] = 0.0f;
    v50.v.m128_f32[3] = 0.0f;
    v9p->v = _mm_setr_ps(-1.0f, 0.0f, v50.v.m128_f32[2],
                         v50.v.m128_f32[3]);
    m_bone_mass_info[2].m_joint_limit_angle[
        m_bone_mass_info[2].m_joint_limit_count++] = 1.4835299f;
    v50.v.m128_f32[2] = -1.0f;
    v50.v.m128_f32[3] = 0.0f;
    math::Dir3* v12 = &m_bone_mass_info[2].m_joint_limit_axis[
        m_bone_mass_info[2].m_joint_limit_count];
    v12->v = _mm_setr_ps(0.0f, 0.0f, v50.v.m128_f32[2],
                         v50.v.m128_f32[3]);
    m_bone_mass_info[2].m_joint_limit_angle[
        m_bone_mass_info[2].m_joint_limit_count++] = 1.9198623f;
    m_bone_mass_info[2].m_capsule_radius = 0.0f;
    m_bone_mass_info[2].m_capsule_b1_adjust_p2_loc.v = _mm_setzero_ps();
    v46.v = _mm_setr_ps(1.0f, 0.0f, 0.0f, 0.0f);
    v47.v = _mm_setr_ps(1.0f, 0.0f, 0.0f, 0.0f);
    v48.v = _mm_setr_ps(1.0f, 0.0f, 0.0f, 0.0f);
    v50.v = _mm_setr_ps(-0.15000001f, 0.0f, 0.0f, 0.0f);
    v49.v = _mm_setr_ps(1.0f, 0.0f, 0.0f, 0.0f);
    m_bone_mass_info[3].set(
        v8, USER_BONE_ID_LEFT_HAND, 0.1f, 0.1f, 0.5f, v50, 2, 50.0f,
        0.30000001f, 0.5f, 3, 2, bone_mass_info::JOINT_TYPE_HINGE,
        -2.0943952f, -0.087266468f, v49, v48, v47, v46, 1.0f);
    m_bone_mass_info[3].m_damp_k = v3;
    m_bone_mass_info[3].m_capsule_radius = 0.075000003f;
    m_bone_mass_info[3].m_capsule_b1_adjust_p2_loc.v = _mm_setzero_ps();
    m_bone_mass_info[3].m_capsule_b1_adjust_p2_loc.v.m128_f32[0] =
        -0.050000001f;
    v46.v = _mm_setr_ps(1.0f, 0.0f, 0.0f, 0.0f);
    v47.v = _mm_setr_ps(1.0f, 0.0f, 0.0f, 0.0f);
    v48.v = _mm_setr_ps(-1.0f, 0.0f, 0.0f, 0.0f);
    v49.v = _mm_setr_ps(-1.0f, 0.0f, 0.0f, 0.0f);
    int v15 = USER_BONE_ID_RIGHT_FOREARM;
    m_bone_mass_info[4].set(
        USER_BONE_ID_RIGHT_UPPERARM, USER_BONE_ID_RIGHT_FOREARM, 0.1f, 0.1f,
        0.5f, v50, 2, 50.0f, 0.30000001f, 1.0f, 4, 0,
        bone_mass_info::JOINT_TYPE_SWIVEL, 0.2f, 1.6f, v49, v48, v47, v46,
        1.0f);
    v50.v.m128_f32[2] = 0.0f;
    v50.v.m128_f32[3] = 0.0f;
    m_bone_mass_info[4].m_damp_k = v3;
    math::Dir3* v16 = &m_bone_mass_info[4].m_joint_limit_axis[
        m_bone_mass_info[4].m_joint_limit_count];
    v16->v = _mm_setr_ps(-1.0f, 0.0f, v50.v.m128_f32[2],
                         v50.v.m128_f32[3]);
    m_bone_mass_info[4].m_joint_limit_angle[
        m_bone_mass_info[4].m_joint_limit_count++] = 1.4835299f;
    v50.v.m128_f32[2] = -1.0f;
    v50.v.m128_f32[3] = 0.0f;
    math::Dir3* v19 = &m_bone_mass_info[4].m_joint_limit_axis[
        m_bone_mass_info[4].m_joint_limit_count];
    v19->v = _mm_setr_ps(0.0f, 0.0f, v50.v.m128_f32[2],
                         v50.v.m128_f32[3]);
    m_bone_mass_info[4].m_joint_limit_angle[
        m_bone_mass_info[4].m_joint_limit_count++] = 1.9198623f;
    m_bone_mass_info[4].m_capsule_b1_adjust_p2_loc.v = _mm_setzero_ps();
    v46.v = _mm_setr_ps(1.0f, 0.0f, 0.0f, 0.0f);
    v47.v = _mm_setr_ps(1.0f, 0.0f, 0.0f, 0.0f);
    v48.v = _mm_setr_ps(1.0f, 0.0f, 0.0f, 0.0f);
    m_bone_mass_info[4].m_capsule_radius = 0.0f;
    v49.v = _mm_setr_ps(1.0f, 0.0f, 0.0f, 0.0f);
    v50.v = _mm_setr_ps(-0.15000001f, 0.0f, 0.0f, 0.0f);
    m_bone_mass_info[5].set(
        v15, USER_BONE_ID_RIGHT_HAND, 0.1f, 0.1f, 0.5f, v50, 2, 50.0f,
        0.30000001f, 0.5f, 5, 4, bone_mass_info::JOINT_TYPE_HINGE,
        -2.0943952f, -0.087266468f, v49, v48, v47, v46, 1.0f);
    v50.v = _mm_setr_ps(0.0f, 0.0f, 1.0f, 0.0f);
    m_bone_mass_info[5].m_capsule_radius = 0.075000003f;
    m_bone_mass_info[5].m_capsule_b1_adjust_p2_loc.v = _mm_setzero_ps();
    m_bone_mass_info[5].m_capsule_b1_adjust_p2_loc.v.m128_f32[0] =
        -0.050000001f;
    __m128 v = v50.v;
    v50.v = _mm_setzero_ps();
    v46.v = v;
    v47.v = v50.v;
    v48.v = _mm_setr_ps(-1.0f, 0.0f, 0.0f, 0.0f);
    m_bone_mass_info[1].m_damp_k = v3;
    v49.v = _mm_setr_ps(1.0f, 0.0f, 0.0f, 0.0f);
    v50.v.m128_f32[2] = 0.0f;
    v50.v.m128_f32[3] = 0.0f;
    int v23 = USER_BONE_ID_LEFT_CALF;
    m_bone_mass_info[6].set(
        USER_BONE_ID_LEFT_THIGH, USER_BONE_ID_LEFT_CALF, 0.15000001f,
        0.15000001f, 0.5f, v50, 2, 50.0f, 0.30000001f, 1.0f, 6, 0,
        bone_mass_info::JOINT_TYPE_SWIVEL, -0.60000002f, 0.60000002f, v49,
        v48, v47, v46, 1.0f);
    math::Dir3* v24 = &m_bone_mass_info[6].m_joint_limit_axis[
        m_bone_mass_info[6].m_joint_limit_count];
    m_bone_mass_info[6].m_damp_k = v3;
    v50.v.m128_f32[2] = 0.0f;
    v50.v.m128_f32[3] = 0.0f;
    v24->v = _mm_setr_ps(1.0f, 0.0f, v50.v.m128_f32[2],
                         v50.v.m128_f32[3]);
    m_bone_mass_info[6].m_joint_limit_angle[
        m_bone_mass_info[6].m_joint_limit_count++] = 0.69813174f;
    v50.v.m128_f32[2] = 0.0f;
    v50.v.m128_f32[3] = 0.0f;
    math::Dir3* v27 = &m_bone_mass_info[6].m_joint_limit_axis[
        m_bone_mass_info[6].m_joint_limit_count];
    v27->v = _mm_setr_ps(0.0f, 1.0f, v50.v.m128_f32[2],
                         v50.v.m128_f32[3]);
    m_bone_mass_info[6].m_joint_limit_angle[
        m_bone_mass_info[6].m_joint_limit_count++] = 1.5707964f;
    m_bone_mass_info[6].m_capsule_b1_adjust_p2_loc.v = _mm_setzero_ps();
    m_bone_mass_info[6].m_capsule_radius = 0.11f;
    v46.v = _mm_setr_ps(1.0f, 0.0f, 0.0f, 0.0f);
    v47.v = _mm_setr_ps(1.0f, 0.0f, 0.0f, 0.0f);
    v48.v = _mm_setr_ps(1.0f, 0.0f, 0.0f, 0.0f);
    v50.v = _mm_setr_ps(-0.15000001f, 0.0f, 0.0f, 0.0f);
    v49.v = _mm_setr_ps(1.0f, 0.0f, 0.0f, 0.0f);
    m_bone_mass_info[7].set(
        v23, USER_BONE_ID_LEFT_FOOT, 0.15000001f, 0.15000001f, 0.5f, v50, 2,
        50.0f, 0.30000001f, 0.25f, 7, 6, bone_mass_info::JOINT_TYPE_HINGE,
        -1.7453293f, -0.087266468f, v49, v48, v47, v46, 1.0f);
    v50.v = _mm_setr_ps(0.0f, 0.0f, 1.0f, 0.0f);
    m_bone_mass_info[7].m_capsule_b1_adjust_p2_loc.v = _mm_setzero_ps();
    m_bone_mass_info[7].m_capsule_b1_adjust_p2_loc.v.m128_f32[0] =
        -0.050000001f;
    m_bone_mass_info[7].m_capsule_radius = 0.1f;
    __m128 v30 = v50.v;
    v50.v = _mm_setzero_ps();
    v46.v = v30;
    v47.v = v50.v;
    v48.v = _mm_setr_ps(-1.0f, 0.0f, 0.0f, 0.0f);
    v50.v.m128_f32[2] = 0.0f;
    v50.v.m128_f32[3] = 0.0f;
    v49.v = _mm_setr_ps(1.0f, 0.0f, 0.0f, 0.0f);
    int v31 = USER_BONE_ID_RIGHT_CALF;
    m_bone_mass_info[7].m_damp_k = v3;
    m_bone_mass_info[8].set(
        USER_BONE_ID_RIGHT_THIGH, USER_BONE_ID_RIGHT_CALF, 0.15000001f,
        0.15000001f, 0.5f, v50, 2, 50.0f, 0.30000001f, 1.0f, 8, 0,
        bone_mass_info::JOINT_TYPE_SWIVEL, -0.60000002f, 0.60000002f, v49,
        v48, v47, v46, 1.0f);
    math::Dir3* v32 = &m_bone_mass_info[8].m_joint_limit_axis[
        m_bone_mass_info[8].m_joint_limit_count];
    m_bone_mass_info[8].m_damp_k = v3;
    v50.v.m128_f32[2] = 0.0f;
    v50.v.m128_f32[3] = 0.0f;
    v32->v = _mm_setr_ps(1.0f, 0.0f, v50.v.m128_f32[2],
                         v50.v.m128_f32[3]);
    m_bone_mass_info[8].m_joint_limit_angle[
        m_bone_mass_info[8].m_joint_limit_count++] = 0.69813174f;
    v50.v.m128_f32[2] = 0.0f;
    v50.v.m128_f32[3] = 0.0f;
    math::Dir3* v35 = &m_bone_mass_info[8].m_joint_limit_axis[
        m_bone_mass_info[8].m_joint_limit_count];
    v35->v = _mm_setr_ps(0.0f, -1.0f, v50.v.m128_f32[2],
                         v50.v.m128_f32[3]);
    m_bone_mass_info[8].m_joint_limit_angle[
        m_bone_mass_info[8].m_joint_limit_count++] = 1.5707964f;
    m_bone_mass_info[8].m_capsule_b1_adjust_p2_loc.v = _mm_setzero_ps();
    m_bone_mass_info[8].m_capsule_radius = 0.11f;
    v46.v = _mm_setr_ps(1.0f, 0.0f, 0.0f, 0.0f);
    v47.v = _mm_setr_ps(1.0f, 0.0f, 0.0f, 0.0f);
    v48.v = _mm_setr_ps(1.0f, 0.0f, 0.0f, 0.0f);
    v50.v = _mm_setr_ps(-0.15000001f, 0.0f, 0.0f, 0.0f);
    v49.v = _mm_setr_ps(1.0f, 0.0f, 0.0f, 0.0f);
    m_bone_mass_info[9].set(
        v31, USER_BONE_ID_RIGHT_FOOT, 0.15000001f, 0.15000001f, 0.5f, v50, 2,
        50.0f, 0.30000001f, 0.25f, 9, 8, bone_mass_info::JOINT_TYPE_HINGE,
        -1.7453293f, -0.087266468f, v49, v48, v47, v46, 1.0f);
    m_bone_mass_info[9].m_capsule_b1_adjust_p2_loc.v = _mm_setzero_ps();
    m_bone_mass_info[9].m_capsule_b1_adjust_p2_loc.v.m128_f32[0] =
        -0.050000001f;
    v50.v = _mm_setr_ps(0.0f, 0.0f, 1.0f, 0.0f);
    m_bone_mass_info[9].m_damp_k = v3;
    __m128 v38 = v50.v;
    m_bone_mass_info[9].m_capsule_radius = 0.1f;

    __m128 scale34 = _mm_set1_ps(34.0f);
    __m128 y_axis = v38;  // (0,0,1,0) - RagdollRotate y-axis
    for (int i = 0; i < 10; ++i)
    {
        bone_mass_info* b = &m_bone_mass_info[i];
        b->m_mass = (g_ragdoll_mass_scale * b->m_mass) * 0.001f;
        b->m_inertia_sphere_radius *= 34.0f;
        b->m_p1_radius *= 34.0f;
        b->m_p2_radius *= 34.0f;
        b->m_b1_adjust_p2_loc.v =
            _mm_mul_ps(RagdollRotate(b->m_b1_adjust_p2_loc.v, y_axis),
                       scale34);
        b->m_capsule_b1_adjust_p2_loc.v = _mm_mul_ps(
            RagdollRotate(b->m_capsule_b1_adjust_p2_loc.v, y_axis), scale34);
        b->m_capsule_radius *= 34.0f;
        b->m_rb_parent_axis_loc.v =
            RagdollRotate(b->m_rb_parent_axis_loc.v, y_axis);
        b->m_rb_axis_loc.v = RagdollRotate(b->m_rb_axis_loc.v, y_axis);
        b->m_rb_parent_ref_loc.v =
            RagdollRotate(b->m_rb_parent_ref_loc.v, y_axis);
        float old_theta_min = b->m_theta_min;
        b->m_rb_ref_loc.v = RagdollRotate(b->m_rb_ref_loc.v, y_axis);
        b->m_theta_min = 0.0f - b->m_theta_max;
        b->m_theta_max = 0.0f - old_theta_min;
        for (int j = 0; j < b->m_joint_limit_count; ++j)
        {
            b->m_joint_limit_axis[j].v = _mm_add_ps(
                _mm_add_ps(
                    _mm_mul_ps(
                        _mm_shuffle_ps(b->m_joint_limit_axis[j].v,
                                       b->m_joint_limit_axis[j].v, 0),
                        _mm_setr_ps(-1.0f, 0.0f, 0.0f, 0.0f)),
                    _mm_mul_ps(
                        _mm_shuffle_ps(b->m_joint_limit_axis[j].v,
                                       b->m_joint_limit_axis[j].v, 85),
                        v46.v)),
                _mm_mul_ps(
                    _mm_shuffle_ps(b->m_joint_limit_axis[j].v,
                                   b->m_joint_limit_axis[j].v, 170),
                    v46.v));
        }
    }
    for (int i = 0; i < 10; ++i)
        m_bone_mass_info[i].calc_stuff(owner);
}

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

// PathNodeMgr (mp_actors.o / path.o view; class tag V matches the binary
// manglings ?sInst@PathNodeMgr@@2PAV1@A / ?SetCoverNodeStatus@PathNodeMgr@@
// QAEXABVstring@Broc@@H@Z; sInst defined in sv_globals.cpp)
class PathNodeMgr {
public:
    static PathNodeMgr* sInst;
    void SetCoverNodeStatus(const Broc::string& name, int inValid);
};

// Destructible (physics.o RBDestructible.cpp; 196 bytes, IDA ordinal).
// mVisibleStatic/mVisibleSwapOut/mVisiblePiece offsets verified from IDA.
class Destructible {
public:
    uint8_t     _pad0[0x3C];
    InplaceVector<unsigned int> mVisibleStatic;      // +0x3C (DbLinkedHandle<...> mVal)
    uint8_t     _pad44[0x4C - 0x44];
    InplaceVector<unsigned int> mVisibleSwapOut;     // +0x4C
    uint8_t     _pad54[0x5C - 0x54];
    InplaceVector<unsigned int> mVisiblePiece;       // +0x5C

    void DeletePiece(Entity* ent);  // ?DeletePiece@Destructible@@QAEXPAVEntity@@@Z
    void ThrowPiece(Entity* entPiece, float force,
                    const math::Position3& hitp,
                    const math::Position3& trajectory,
                    bool useRealPhysics);  // ?ThrowPiece@Destructible@@QAEXPAVEntity@@MABVPosition3@math@@1_N@Z
    static void InvalidateCoverNode(Entity* ent);  // ?InvalidateCoverNode@Destructible@@SAXPAVEntity@@@Z
    void AddPiece(Entity* ent, const char* exploderType);  // ?AddPiece@Destructible@@QAEXPAVEntity@@PBD@Z
};

// ea: 0x6F64B0
void Destructible::InvalidateCoverNode(Entity* ent)
{
    if (ent->mTarget.mBlock != nullptr)
        PathNodeMgr::sInst->SetCoverNodeStatus(ent->mTarget, 1);
}

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

// ea: 0x70D870
void Destructible::ThrowPiece(Entity* entPiece, float force,
                              const math::Position3& hitp,
                              const math::Position3& trajectory,
                              bool useRealPhysics)
{
    if (entPiece != nullptr)
    {
        if (useRealPhysics)
        {
            math::Dir3 v7;
            v7.v = trajectory.v;
            ApplyPhysics(entPiece, hitp, v7, force, false,
                         (hitLocation_t)4);  // HITLOC_TORSO_UPR
        }
        else
        {
            math::Dir3 v7;
            v7.v = _mm_mul_ps(trajectory.v, _mm_set1_ps(force));
            MoveGravity(entPiece, &v7.v.m128_f32[0], 10.0f);
            math::Dir3 rot;
            rot.v = _mm_mul_ps(trajectory.v, _mm_set1_ps(0.5f));
            math::Position3 rotpos;
            rotpos.v = rot.v;
            BrocSys::Mover_RotateSpeed(entPiece, rotpos, 10.0f, 0.0f, 0.0f);
        }
    }
}

// AddPiece helper (shared by the exploder_stay / exploder_swap_out /
// exploder_piece_visible branches; identical scan-and-fill logic per branch)
static void AddPiece_ToVector(InplaceVector<unsigned int>* p,
                              unsigned int ent_handle)
{
    unsigned int v4 = 0;
    if (p->mSize != 0)
    {
        while (1)
        {
            unsigned int mVal = p->mList[v4];
            unsigned int v6 = mVal & 0xFFF;
            if (v6 >= 0x540
                || mVal >> 12 != EntityHandleDb::sInst.mElements[v6].mKey
                || EntityHandleDb::sInst.mElements[v6].mObject == nullptr)
                break;
            if (++v4 >= p->mSize)
                goto LABEL_9;
        }
        p->mList[v4] = ent_handle;
        return;
    }
LABEL_9:
    for (unsigned int i = 0; i < p->mSize; ++i)
    {
        unsigned int v8 = i;
        if (i >= p->mSize)
        {
            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
            AeAssert::gCurrentFile = "../ae\\inplace/InplaceVector.h";
            AeAssert::gCurrentLine = 81;
            AeAssert::gCurrentExpr = "index < mSize";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("Bounds check"))
                __debugbreak();
            if (i >= p->mSize)
                v8 = 0;
        }
        p->mList[v8] = 0;
    }
    unsigned int v9 = 0;
    if (p->mSize != 0)
    {
        while (1)
        {
            unsigned int v10 = v9;
            if (v9 >= p->mSize)
            {
                AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
                AeAssert::gCurrentFile = "../ae\\inplace/InplaceVector.h";
                AeAssert::gCurrentLine = 81;
                AeAssert::gCurrentExpr = "index < mSize";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert("Bounds check"))
                    __debugbreak();
                if (v9 >= p->mSize)
                    v10 = 0;
            }
            unsigned int v11 = p->mList[v10] & 0xFFF;
            if (v11 >= 0x540
                || p->mList[v10] >> 12
                       != EntityHandleDb::sInst.mElements[v11].mKey
                || EntityHandleDb::sInst.mElements[v11].mObject == nullptr)
                break;
            if (++v9 >= p->mSize)
                return;
        }
        if (v9 < p->mSize)
            goto LABEL_36;
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "../ae\\inplace/InplaceVector.h";
        AeAssert::gCurrentLine = 81;
        AeAssert::gCurrentExpr = "index < mSize";
        if (AeAssert::IsIgnored() || !AeAssert::Assert("Bounds check"))
        {
        LABEL_34:
            if (v9 >= p->mSize)
                v9 = 0;
        LABEL_36:
            p->mList[v9] = ent_handle;
            return;
        }
        __debugbreak();
        goto LABEL_34;
    }
}

// ea: 0x7057A0
void Destructible::AddPiece(Entity* ent, const char* exploderType)
{
    if (exploderType != nullptr && ent != nullptr)
    {
        if (strcmp(exploderType, "exploder_stay") == 0)
        {
            AddPiece_ToVector(&mVisibleStatic, ent->mHandle);
        }
        else if (strcmp(exploderType, "exploder_swap_out") == 0)
        {
            AddPiece_ToVector(&mVisibleSwapOut, ent->mHandle);
        }
        else if (strcmp(exploderType, "exploder_piece_visible") == 0)
        {
            AddPiece_ToVector(&mVisiblePiece, ent->mHandle);
        }
    }
}

// ============================================================================
// PhysDataBankManager / DestructibleBankManager DecodeBank family (physics.o
// RBPhysData.cpp / RBDestructible.cpp). The InplaceAssetBank<> Fixup and
// InplaceAssetBankSet<> AddBank calls are template instantiations from
// inplace.o/streamer.o; local C-name helpers stand in until those templates
// are ported (same pattern as ConfigStringManager::DecodeBank).
// ============================================================================
class PhysDataBank;
class DestructibleBank;
class DestructibleLocal;
class PhysDataBankManager {
public:
    static PhysDataBankManager* sInst;  // ?sInst@PhysDataBankManager@@2PAV1@A (g_globals.cpp)
    void DecodeBank(const char* name, unsigned char* data, int size,
                    TPakId pak_id);  // ?DecodeBank@PhysDataBankManager@@QAEXPBDPAEHW4TPakId@@@Z
    IVPointer<PhysData> GetPhysData(
        TPakId pak_id, const char* name);  // ?GetPhysData@PhysDataBankManager@@QAE?AV?$IVPointer@VPhysData@@@@W4TPakId@@PBD@Z
};
class DestructibleBankManager {
public:
    static DestructibleBankManager* sInst;  // ?sInst@DestructibleBankManager@@2PAV1@A (g_globals.cpp)
    void DecodeBank(const char* name, unsigned char* data, int size,
                    TPakId pak_id);  // ?DecodeBank@DestructibleBankManager@@QAEXPBDPAEHW4TPakId@@@Z
    IVPointer<Destructible> GetDestructible(
        TPakId pak_id, const char* name);  // ?GetDestructible@DestructibleBankManager@@QAE?AV?$IVPointer@VDestructible@@@@W4TPakId@@PBD@Z
};

// InplaceAssetBank<PhysData,InplaceTree<InplaceString,unsigned int>>::Fixup
// @ 0x410B14 / InplaceAssetBankSet<PhysDataBank>::AddBank @ 0x42E4F2 (stubs)
void InplaceAssetBank_Fixup_PhysData(void* data) { (void)data; }
void InplaceAssetBankSet_AddBank_PhysDataBank(void* self, TPakId pakId,
                                              void* data)
{
    (void)self; (void)pakId; (void)data;
}
// InplaceAssetBank<Destructible,InplaceTree<InplaceString,unsigned int>>::
// Fixup @ 0x41FF92 / InplaceAssetBankSet<DestructibleBank>::AddBank @ 0x42F447
void InplaceAssetBank_Fixup_Destructible(void* data) { (void)data; }
void InplaceAssetBankSet_AddBank_DestructibleBank(void* self, TPakId pakId,
                                                  void* data)
{
    (void)self; (void)pakId; (void)data;
}

// ea: 0x6FE870
void PhysDataBankManager::DecodeBank(const char* name, unsigned char* data,
                                     int size, TPakId pak_id)
{
    (void)name; (void)size;
    InplaceAssetBank_Fixup_PhysData(data);
    InplaceAssetBankSet_AddBank_PhysDataBank(this, pak_id, data);
}

// ea: 0x702C80
void DestructibleBankManager::DecodeBank(const char* name, unsigned char* data,
                                         int size, TPakId pak_id)
{
    (void)name; (void)size;
    InplaceAssetBank_Fixup_Destructible(data);
    InplaceAssetBankSet_AddBank_DestructibleBank(this, pak_id, data);
}

// ea: 0x7031B0
void DecodePhysData(const char* name, unsigned char* data, int size,
                    TPakId pakId, PakFile* pakFile)
{
    (void)name; (void)size; (void)pakFile;
    PhysDataBankManager* v4 = PhysDataBankManager::sInst;
    InplaceAssetBank_Fixup_PhysData(data);
    InplaceAssetBankSet_AddBank_PhysDataBank(v4, pakId, data);
}

// ea: 0x705CA0
void DecodeDestructible(const char* name, unsigned char* data, int size,
                        TPakId pakId, PakFile* pakFile)
{
    (void)name; (void)size; (void)pakFile;
    DestructibleBankManager* v4 = DestructibleBankManager::sInst;
    InplaceAssetBank_Fixup_Destructible(data);
    InplaceAssetBankSet_AddBank_DestructibleBank(v4, pakId, data);
}

// ea: 0x709720
Entity* SpawnBrokenPiece(Entity* owner, const char* classname,
                         const char* modelName, const math::Position3* origin,
                         bool makeDestructible)
{
    BrocAPILocal* api = (BrocAPILocal*)gpBrocAPI;
    TPakInfoLocal* v6 =
        api->mGetPakVector(origin->v.m128_f32[0], origin->v.m128_f32[1],
                           origin->v.m128_f32[2]);
    TPakId v7;
    if (v6 != nullptr)
        v7 = v6->mPakId;
    else
        v7 = CurPakId();
    if (owner != nullptr)
    {
        TPakId mPakId = (TPakId)owner->mPakId;
        if (mPakId == PAK_ID_INVALID)
            mPakId = CurPakId();
        if (mPakId != CurPakId())
        {
            TPakId v9 = (TPakId)owner->mPakId;
            if (v9 == PAK_ID_INVALID)
                v9 = CurPakId();
            v7 = v9;
        }
    }
    void* result = G_GetModel(modelName, v7);
    if (result != nullptr)
    {
        TPakId v11 = v6 != nullptr ? v6->mPakId : PAK_ID_INVALID;
        Entity* v12 = G_Spawn(v11);
        v12->mClassName = classname;
        HashString hs(v12->mClassName);
        v12->mClassNameHash.mHash = hs.mHash;
        v12->r.currentOrigin.v = origin->v;
        if ((_fpclass(v12->r.currentOrigin.v.m128_f32[0]) & 0x297) != 0
            || (_fpclass(v12->r.currentOrigin.v.m128_f32[1]) & 0x297) != 0
            || (_fpclass(v12->r.currentOrigin.v.m128_f32[2]) & 0x297) != 0)
        {
            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\Destructible.cpp";
            AeAssert::gCurrentLine = 49;
            AeAssert::gCurrentExpr =
                "!IS_NAN((ent->r.currentOrigin)[0]) && !IS_NAN((ent->r.currentOrigin)[1]) && !IS_NAN((ent->r.currentOrigin)[2])";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("Invalid vector"))
                __debugbreak();
        }
        v12->spawnflags = 0;
        SetModel(v12, modelName);
        if (G_CallSpawnEntity(v12) != 0)
        {
            if (makeDestructible)
            {
                TPakId v13 = (TPakId)v12->mPakId;
                if (v13 == PAK_ID_INVALID)
                    v13 = CurPakId();
                IVPointer<Destructible> Destructible =
                    DestructibleBankManager::sInst->GetDestructible(
                        v13, "global");
                v12->mDestructibleValue = Destructible.mValue;
                v12->mDestructiblePakId = Destructible.mPakId;
                v12->takedamage = 1;
            }
            SV_SetBrushModel(v12);
            ValidatePakId(v12->mModelPakId);
            TPakId v17 = (TPakId)v12->mPakId;
            const char* mStr =
                v12->mModelValue != nullptr
                    ? ((XModelLocal*)v12->mModelValue)->mStr
                    : nullptr;
            if (v17 == PAK_ID_INVALID)
                v17 = CurPakId();
            IVPointer<PhysData> p =
                PhysDataBankManager::sInst->GetPhysData(v17, mStr);
            int v19 = p.mPakId;
            ValidatePakId(p.mPakId);
            if (p.mValue != nullptr)
            {
                DObj* mDObj = v12->mDObj;
                mDObj->mPhysDataValue = p.mValue;
                mDObj->mPhysDataPakId = v19;
                IVPointer<PhysData> v21;
                v21.mValue = p.mValue;
                v21.mPakId = v19;
                CalculatePhysData(v12, v21);
            }
            v12->r.contents = 0x200001;
            if (owner != nullptr)
                v12->r.svFlags = owner->r.svFlags;
            else
                v12->r.svFlags |= 0x10u;
            v12->r.contents = 0x202081;
            G_SetOrigin(v12, origin);
            g_LinkEntity(v12);
        }
        return v12;
    }
    return (Entity*)result;
}

// ea: 0x702EA0
void CalculatePhysData(Entity* ent, float& mass, float& friction,
                       float& bounce)
{
    if (ent != nullptr)
    {
        DObj* mDObj = ent->mDObj;
        if (mDObj != nullptr)
        {
            void** model0 = (void**)mDObj->models;
            TPakId mPakId = (TPakId)(uintptr_t)model0[1];
            XModelLocal* mValue = (XModelLocal*)model0[0];
            ValidatePakId(mPakId);
            if (mValue != nullptr && ent->r.bmodel != nullptr)
            {
                ValidatePakId(mPakId);
                __m128 mins = ((math::Position3*)mValue)->v;
                ValidatePakId(mPakId);
                __m128 maxs =
                    ((math::Position3*)((char*)mValue + 0x10))->v;
                __m128 v7 = _mm_sub_ps(maxs, mins);
                float v17 =
                    ((v7.m128_f32[2] + v7.m128_f32[1]) + v7.m128_f32[0])
                    * 0.333f;
                ValidatePakId(mPakId);
                unsigned int mSize = *(unsigned int*)((char*)mValue + 0x38);
                void** p_collSurfs =
                    *(void***)((char*)mValue + 0x3C);
                int surfFlags;
                if (mSize != 0)
                {
                    ValidatePakId(mPakId);
                    surfFlags = *(int*)p_collSurfs[0];
                }
                else
                {
                    DCGSet* bmodel = (DCGSet*)ent->r.bmodel;
                    unsigned int m_count =
                        *(unsigned int*)((char*)bmodel + 0x40);
                    void* p_objects = *(void**)((char*)bmodel + 0x44);
                    if (m_count == 0
                        && _tlAssert(
                               "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h",
                               91, "index >= 0 && index < size()",
                               "invalid index"))
                        __debugbreak();
                    surfFlags = ((cdl_object_t*)p_objects)->sflags;
                }
                mass = 0.050000001f;
                friction = 0.2f;
                bounce = 0.25f;
                if (surfFlags != 0)
                {
                    int v14 = ((surfFlags >> 20) & 0x1F) - 1;
                    if (v14 > -1)
                    {
                        mass = (infoSurfaceProperties[v14].density
                                * 0.0099999998f)
                            * v17;
                        friction = infoSurfaceProperties[v14].friction;
                        bounce = 0.25f;
                    }
                }
            }
            else
            {
                mass = 0.1f;
                friction = 0.2f;
                bounce = 0.25f;
            }
        }
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

// ea: 0x6FE160
unsigned int get_gjk_geom_id(Entity* ent)
{
    if (ent == nullptr
        && _tlAssert("c:\\cod\\code\\game\\RBSimpleAPI.cpp", 36, "ent",
                     defaultFileName))
        __debugbreak();
    DCGSet* bmodel = (DCGSet*)ent->r.bmodel;
    if (bmodel == nullptr
        && _tlAssert("c:\\cod\\code\\game\\RBSimpleAPI.cpp", 38, "dcg",
                     defaultFileName))
        __debugbreak();
    unsigned int v2 = 0;
    unsigned int nobjects = bmodel->objects_m_count;
    if (nobjects == 0)
        return 0;
    while (1)
    {
        if (v2 >= (unsigned int)bmodel->objects_m_count)
        {
            AeAssert::gCurrentAuthor = AeAssert::JSV;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\cgbank.h";
            AeAssert::gCurrentLine = 77;
            AeAssert::gCurrentExpr = "index < size()";
            if (!AeAssert::IsIgnored() && AeAssert::Assert(defaultFileName))
                __debugbreak();
            if (v2 >= (unsigned int)bmodel->objects_m_count
                && _tlAssert(
                       "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h", 89,
                       "index >= 0 && index < size()", "invalid index"))
                __debugbreak();
        }
        cdl_object_t* obj =
            &((cdl_object_t*)bmodel->objects_m_elements)[v2];
        if ((obj->cflags & 0x241) != 0)
            break;
        if (++v2 >= nobjects)
            return 0;
    }
    if (v2 >= 0x470
        && _tlAssert("c:\\cod\\code\\game\\RBCollision.h", 283,
                     "object_id < sizeof(Entity)", defaultFileName))
        __debugbreak();
    // Binary: ent + v2 (Entity stride 0x470); object id encoded as address.
    return (unsigned int)((char*)ent + v2 * 0x470);
}

// ea: 0x70D380
void ApplyPhysics(Entity* hitEnt, const math::Position3& hitp,
                  const math::Dir3& hitd, float force, bool local_hitp,
                  hitLocation_t hitLoc)
{
    if ((hitp.v.m128_f32[0] != hitp.v.m128_f32[0])
        || (hitp.v.m128_f32[1] != hitp.v.m128_f32[1])
        || (hitp.v.m128_f32[2] != hitp.v.m128_f32[2]))
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\RBSimpleAPI.cpp";
        AeAssert::gCurrentLine = 51;
        AeAssert::gCurrentExpr =
            "!IS_NAN((hitp)[0]) && !IS_NAN((hitp)[1]) && !IS_NAN((hitp)[2])";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid vector"))
            __debugbreak();
    }
    if ((hitd.v.m128_f32[0] != hitd.v.m128_f32[0])
        || (hitd.v.m128_f32[1] != hitd.v.m128_f32[1])
        || (hitd.v.m128_f32[2] != hitd.v.m128_f32[2]))
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\RBSimpleAPI.cpp";
        AeAssert::gCurrentLine = 52;
        AeAssert::gCurrentExpr =
            "!IS_NAN((hitd)[0]) && !IS_NAN((hitd)[1]) && !IS_NAN((hitd)[2])";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid vector"))
            __debugbreak();
    }
    if (force != force)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\RBSimpleAPI.cpp";
        AeAssert::gCurrentLine = 53;
        AeAssert::gCurrentExpr = "!IS_NAN(force)";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid number!"))
            __debugbreak();
    }
    if (hitEnt == nullptr || hitEnt == EntityManager::sInst->mWorld
        || hitEnt->mDObj == nullptr)
        return;
    if ((hitEnt->flags & 0x2000000) != 0
        || (hitEnt->r.contents & 0x8000) != 0)
    {
        biped_phys_info* bp = create_biped_phys_info(hitEnt);
        hitEnt->set_bp_info(bp);
    }
    if (hitEnt->mBPInfo != nullptr)
    {
        phys_bones PhysBoneID = (phys_bones)GetPhysBoneID(hitLoc);
        if (hitEnt->mBPInfo->m_bp_sys != nullptr)
        {
            math::Dir3 pulse;
            pulse.v = _mm_mul_ps(hitd.v, _mm_set1_ps(force));
            if (fabs(hitp.v.m128_f32[0]) >= 0.000099999997f
                || fabs(hitp.v.m128_f32[1]) >= 0.000099999997f
                || fabs(hitp.v.m128_f32[2]) >= 0.000099999997f)
                hitEnt->mBPInfo->m_bp_sys->apply_pulse_to_bone(
                    PhysBoneID, hitp, pulse, 1.0f);
            else
                hitEnt->mBPInfo->m_bp_sys->apply_pulse_to_bone(PhysBoneID,
                                                               pulse);
            return;
        }
        if (g_list_biped_system.m_alloc_count < 16
            && phys_sys::available_slots_rigid_body() >= 10)
        {
            hitEnt->mBPInfo->create_bp_sys(0);
            if (hitEnt->tagInfo != nullptr
                && ((tagInfoLocal*)hitEnt->tagInfo)->parent != nullptr)
                G_EntUnlink(hitEnt);
            if (hitEnt->actor != nullptr || (hitEnt->flags & 0x2000000) != 0)
                KillEntity(hitEnt);
            math::Dir3 pulse;
            pulse.v = _mm_mul_ps(hitd.v, _mm_set1_ps(force));
            if (fabs(hitp.v.m128_f32[0]) >= 0.000099999997f
                || fabs(hitp.v.m128_f32[1]) >= 0.000099999997f
                || fabs(hitp.v.m128_f32[2]) >= 0.000099999997f)
                hitEnt->mBPInfo->m_bp_sys->apply_pulse_to_bone(
                    PhysBoneID, hitp, pulse, 1.0f);
            else
                hitEnt->mBPInfo->m_bp_sys->apply_pulse_to_bone(PhysBoneID,
                                                               pulse);
            return;
        }
        AeAssert::gCurrentAuthor = AeAssert::JRS;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\RBSimpleAPI.cpp";
        AeAssert::gCurrentLine = 99;
        AeAssert::gCurrentExpr = nullptr;
        if (!AeAssert::IsIgnored()
            && AeAssert::Warning(
                   "Failed to create ragdoll.  Already at max ragdolls %d.",
                   16))
            __debugbreak();
        if ((hitEnt->flags & 0x2000000) != 0)
            KillEntity(hitEnt);
        return;
    }
    if (hitEnt->scr_vehicle != nullptr)
    {
        rb_vehicle* mRBVeh =
            (rb_vehicle*)((scr_vehicle_t*)hitEnt->scr_vehicle)->mRBVeh;
        if (mRBVeh != nullptr)
            mRBVeh->unpause_physics();
    }
    if ((hitEnt->flags & 0x400000) != 0)
    {
        if (fabs(hitp.v.m128_f32[0]) >= 0.000099999997f
            || fabs(hitp.v.m128_f32[1]) >= 0.000099999997f
            || fabs(hitp.v.m128_f32[2]) >= 0.000099999997f)
        {
            if (local_hitp)
            {
                rigid_body* entity_rb =
                    (rigid_body*)rb_prop_system::get_entity_rb(hitEnt);
                math::Mat43 mat = entity_rb->m_mat;
                math::Position3 world_hitp;
                world_hitp.v = _mm_add_ps(
                    _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(hitp.v, hitp.v, 0),
                                          mat.x.v),
                               _mm_mul_ps(_mm_shuffle_ps(hitp.v, hitp.v, 85),
                                          mat.y.v)),
                    _mm_add_ps(
                        _mm_mul_ps(_mm_shuffle_ps(hitp.v, hitp.v, 170),
                                   mat.z.v),
                        mat.w.v));
                rb_prop_system::hit_entity(hitEnt, world_hitp, hitd, force,
                                           1.0f);
            }
            else
            {
                rb_prop_system::hit_entity(hitEnt, hitp, hitd, force, 1.0f);
            }
        }
        else
        {
            rigid_body* v15 =
                (rigid_body*)rb_prop_system::get_entity_rb(hitEnt);
            math::Position3 v22;
            v22.v = v15->m_mat.w.v;
            rb_prop_system::hit_entity(hitEnt, v22, hitd, force, 1.0f);
        }
        return;
    }
    if (hitEnt->scr_vehicle == nullptr)
        rb_prop_system::add_entity(hitEnt, -1.0f, -1.0f);
    if ((hitEnt->flags & 0x400000) != 0)
    {
        if (fabs(hitp.v.m128_f32[0]) >= 0.000099999997f
            || fabs(hitp.v.m128_f32[1]) >= 0.000099999997f
            || fabs(hitp.v.m128_f32[2]) >= 0.000099999997f)
        {
            if (local_hitp)
            {
                rigid_body* entity_rb =
                    (rigid_body*)rb_prop_system::get_entity_rb(hitEnt);
                math::Mat43 mat = entity_rb->m_mat;
                math::Position3 world_hitp;
                world_hitp.v = _mm_add_ps(
                    _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(hitp.v, hitp.v, 0),
                                          mat.x.v),
                               _mm_mul_ps(_mm_shuffle_ps(hitp.v, hitp.v, 85),
                                          mat.y.v)),
                    _mm_add_ps(
                        _mm_mul_ps(_mm_shuffle_ps(hitp.v, hitp.v, 170),
                                   mat.z.v),
                        mat.w.v));
                rb_prop_system::hit_entity(hitEnt, world_hitp, hitd, force,
                                           1.0f);
            }
            else
            {
                rb_prop_system::hit_entity(hitEnt, hitp, hitd, force, 1.0f);
            }
        }
        else
        {
            rigid_body* v15 =
                (rigid_body*)rb_prop_system::get_entity_rb(hitEnt);
            math::Position3 v22;
            v22.v = v15->m_mat.w.v;
            rb_prop_system::hit_entity(hitEnt, v22, hitd, force, 1.0f);
        }
    }
}

// ?_Return_MF_UnderCrossHair@@YAPAVEntity@@XZ (game2.o 0x503E30)
extern Entity* _Return_MF_UnderCrossHair();

// ea: 0x70EF20
void FN_HangEntity()
{
    Entity* v1 = _Return_MF_UnderCrossHair();
    if (v1 != nullptr)
    {
        math::Position3 worldPt;
        math::Dir3 hitd;
        hitd.v = _mm_setzero_ps();
        worldPt.v = v1->r.currentMat.w.v;
        ApplyPhysics(v1, worldPt, hitd, 1.0f, false, HITLOC_TORSO_UPR);
        biped_system* m_bp_sys = v1->mBPInfo->m_bp_sys;
        if (m_bp_sys != nullptr)
        {
            if (m_bp_sys->m_list_rigid_body.m_alloc_count <= 1
                && _tlAssert(
                       "c:\\cod\\code\\tl\\physics\\include\\phys_array_base.inc",
                       108, "i >= 0 && i < m_alloc_count", defaultFileName))
                __debugbreak();
            rigid_body* v5 = m_bp_sys->m_list_rigid_body.m_slot_array[1];
            if (v5 != nullptr)
            {
                math::Dir3 zero_loc;
                zero_loc.v = _mm_setzero_ps();
                rigid_body_constraint_point* rbc_point =
                    phys_sys::create_rbc_point(
                        v5, phys_sys::get_environment_rigid_body(), false);
                rbc_point->set(zero_loc,
                               reinterpret_cast<const math::Dir3&>(worldPt));
            }
        }
    }
}

// ea: 0x705DA0
bool FindInitialVel(Entity* e, math::Dir3& initial_t_vel,
                    math::Dir3& initial_a_vel)
{
    Entity* v3 = e;
    if ((e->flags & 0x400000) != 0)
        goto LABEL_6;
    while (true)
    {
        if (v3->scr_vehicle != nullptr)
        {
            VEH_Backup(v3);
            VEH_UpdatePath(v3, 50);
            scr_vehicle_t* scr_vehicle = (scr_vehicle_t*)v3->scr_vehicle;
            math::Dir3 v9;
            v9.v.m128_f32[0] = scr_vehicle->phys.vel.v.m128_f32[0];
            v9.v.m128_f32[1] = scr_vehicle->phys.vel.v.m128_f32[1];
            v9.v.m128_f32[2] = scr_vehicle->phys.vel.v.m128_f32[2];
            v9.v.m128_f32[3] = 0.0f;
            initial_t_vel.v = v9.v;
            math::Dir3 va;
            va.v.m128_f32[0] =
                scr_vehicle->phys.rotVel.v.m128_f32[0] * 3.1415927f
                * 0.0055555557f;
            va.v.m128_f32[1] =
                scr_vehicle->phys.rotVel.v.m128_f32[1] * 3.1415927f
                * 0.0055555557f;
            va.v.m128_f32[2] =
                scr_vehicle->phys.rotVel.v.m128_f32[2] * 3.1415927f
                * 0.0055555557f;
            va.v.m128_f32[3] = 0.0f;
            initial_a_vel.v = _mm_mul_ps(va.v, _mm_set1_ps(0.3f));
            return true;
        }
        if (v3->tagInfo == nullptr)
            break;
        v3 = (Entity*)((tagInfoLocal*)v3->tagInfo)->parent;
        if (v3 == nullptr)
            break;
        if ((v3->flags & 0x400000) != 0)
            goto LABEL_6;
    }
    initial_t_vel.v = _mm_setzero_ps();
    initial_a_vel.v = _mm_setzero_ps();
    return false;
LABEL_6:
    const rigid_body* entity_rb = rb_prop_system::get_entity_rb(v3);
    if (entity_rb != nullptr)
    {
        initial_t_vel.v = entity_rb->m_t_vel.v;
        initial_a_vel.v = entity_rb->m_a_vel.v;
    }
    else
    {
        AeAssert::gCurrentAuthor = AeAssert::JRS;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\RBPropSys.cpp";
        AeAssert::gCurrentLine = 25;
        AeAssert::gCurrentExpr = "rb";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Bad flag on physics entity."))
        {
            __debugbreak();
            return true;
        }
    }
    return true;
}

// ea: 0x7096E0
void SetAVel(Entity* e)
{
    if ((e->flags & 0x400000) == 0)
        rb_prop_system::add_entity(e, -1.0f, -1.0f);
}

// minimal BitSet<1344> iteration over EntityHandleDb::sInst.mFreeIndices
// (the binary's ~mFreeIndices set; 1344 bits = 42 words at +0x00)
static int bitset_next_free(int& word_idx, unsigned int& cur_word,
                            int& cur_val)
{
    for (;;)
    {
        if (cur_word != 0)
        {
            unsigned long bit;
            _BitScanForward(&bit, cur_word);
            cur_word &= ~(1u << bit);
            cur_val = word_idx * 32 + (int)bit;
            return cur_val;
        }
        if (++word_idx >= 42)
        {
            word_idx = -1;
            return -1;
        }
        cur_word = ((unsigned int*)&EntityHandleDb::sInst)[word_idx];
    }
}

// ea: 0x7041B0
Entity* nuge_get_actor()
{
    int word_idx = 0;
    unsigned int cur_word = ((unsigned int*)&EntityHandleDb::sInst)[0];
    int cur_val = -1;
    // ~mFreeIndices: invert the stored free bits
    cur_word = ~cur_word;
    bitset_next_free(word_idx, cur_word, cur_val);
    for (;;)
    {
        if (cur_val != -1)
        {
            if (cur_val >= 0x540)
            {
                AeAssert::gCurrentAuthor = AeAssert::COD3;
                AeAssert::gCurrentFile = "c:\\cod\\code\\game\\HandleDb.h";
                AeAssert::gCurrentLine = 78;
                AeAssert::gCurrentExpr =
                    "idx >= 0 && idx < _MaxEltements";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert("index out of bounds"))
                    __debugbreak();
            }
            Entity* result = EntityHandleDb::sInst.mElements[cur_val].mObject;
            if (result != nullptr && result->actor != nullptr)
            {
                biped_phys_info* mBPInfo = result->mBPInfo;
                if (mBPInfo != nullptr && mBPInfo->m_bp_sys == nullptr)
                    return result;
            }
            bitset_next_free(word_idx, cur_word, cur_val);
        }
        else
        {
            if (word_idx == -1)
                return nullptr;
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\HandleDb.h";
            AeAssert::gCurrentLine = 78;
            AeAssert::gCurrentExpr = "idx >= 0 && idx < _MaxEltements";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("index out of bounds"))
                __debugbreak();
        }
    }
}

// ea: 0x7040A0
Entity* nuge_get_entity(const Broc::string& target_name)
{
    int word_idx = 0;
    unsigned int cur_word = ~((unsigned int*)&EntityHandleDb::sInst)[0];
    int cur_val = -1;
    bitset_next_free(word_idx, cur_word, cur_val);
    for (;;)
    {
        if (cur_val != -1)
        {
            if (cur_val >= 0x540)
            {
                AeAssert::gCurrentAuthor = AeAssert::COD3;
                AeAssert::gCurrentFile = "c:\\cod\\code\\game\\HandleDb.h";
                AeAssert::gCurrentLine = 78;
                AeAssert::gCurrentExpr =
                    "idx >= 0 && idx < _MaxEltements";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert("index out of bounds"))
                    __debugbreak();
            }
            Entity* mObject =
                EntityHandleDb::sInst.mElements[cur_val].mObject;
            if (mObject != nullptr)
            {
                // mObject->targetname (Broc::string at +0x284 in full Entity;
                // local view: targetname mBlock at +0x28C per g_local.h)
                void* mBlock = *(void**)((char*)mObject + 0x28C);
                if (mBlock != nullptr)
                {
                    const char* ent_name = (const char*)mBlock + 4;
                    const char* want = (const char*)target_name.mBlock + 4;
                    if (_stricmp(ent_name, want) == 0)
                        return mObject;
                }
            }
            bitset_next_free(word_idx, cur_word, cur_val);
        }
        else
        {
            if (word_idx == -1)
                return nullptr;
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\HandleDb.h";
            AeAssert::gCurrentLine = 78;
            AeAssert::gCurrentExpr = "idx >= 0 && idx < _MaxEltements";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("index out of bounds"))
                __debugbreak();
        }
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
    uint8_t         _pad14[0x20 - 0x14];  // 32-byte stride (IDA ordinal)
};
// phys_gjk_geom_ragdoll_1 (physics.o; 128 bytes; derives from phys_gjk_geom
// which has vtable +0x00, m_geom_id +0x30, m_next_geom +0x34)
class phys_gjk_geom_ragdoll_1 {
public:
    void*          __vftable;   // +0x00
    uint8_t        _pad4[0x30 - 0x04];
    void*          m_geom_id;   // +0x30
    void*          m_next_geom; // +0x34
    uint8_t        _pad38[0x40 - 0x38];
    math::Dir3     m_list_center[3];  // +0x40
    math::Position3 m_center;         // +0x60
    float          m_list_radius[2];  // +0x70
    float          m_geom_radius;     // +0x78
    int            m_count;           // +0x7C
    math::Dir3     m_aabb_mn;         // +0x80
    math::Dir3     m_aabb_mx;         // +0x90

    void init(rigid_body_sphere_list* cg);  // ?init@phys_gjk_geom_ragdoll_1@@QAEXPAVrigid_body_sphere_list@@@Z
    virtual void comp_aabb(const math::Mat43& cg_to_world_xform);  // ?comp_aabb@phys_gjk_geom_ragdoll_1@@UAEXABVMat43@math@@@Z
};

class rigid_body_sphere_list {
public:
    uint8_t        _pad0[0x40];
    rb_collision_sphere* m_slot_array;  // +0x40
    int            m_alloc_count;       // +0x44
    uint8_t        _pad48[0x50 - 0x48];
    math::Position3 m_bounding_sphere_center_loc;  // +0x50
    float          m_bounding_sphere_radius;       // +0x60
    uint8_t        _pad64[0x70 - 0x64];
    rb_collision_capsule m_capsule;  // +0x70
    math::Position3 m_tunnel_test_last_pos;  // +0xC0
    math::Position3 m_tunnel_hit_pos;  // +0xD0
    float          m_tunnel_test_radius;  // +0xE0
    int            m_tunnel_test_active_counter;  // +0xE4
    rigid_body*    m_owner;  // +0xE8
    int            m_rb_id;  // +0xEC
    phys_gjk_geom_ragdoll_1 m_gjk_geom;  // +0xF0
    math::Dir3     m_total_aabb_mn;  // +0x170
    math::Dir3     m_total_aabb_mx;  // +0x180

private:
    void calc_bounding_sphere();  // ?calc_bounding_sphere@rigid_body_sphere_list@@AAEXXZ
public:
    void calc_total_aabb();  // ?calc_total_aabb@rigid_body_sphere_list@@QAEXXZ (physics.o inline 0xB09930)
    void tunnel_test_prolog(physics_colgeom_visitor* visitor);  // ?tunnel_test_prolog@rigid_body_sphere_list@@QAEXPAUphysics_colgeom_visitor@@@Z (inline 0xB0D0F0)
    void update_colgeom();  // ?update_colgeom@rigid_body_sphere_list@@QAEXXZ (inline 0xB0B950)
    void init_tunnel_test(math::Position3& center_pos);  // ?init_tunnel_test@rigid_body_sphere_list@@QAEXAAVPosition3@math@@@Z
    void set(rigid_body* const owner);  // ?set@rigid_body_sphere_list@@QAEXQAVrigid_body@@@Z
};

// ea: 0x708C80
void biped_system::setup_initial_gjk_cache(
    unsigned int geom_id, const math::Dir3& separation_direction)
{
    for (int i = 0; i < 10; ++i)
    {
        rigid_body_sphere_list** m_alloc_list =
            m_collision_callback.m_rb_colgeom_alloc_list;
        rigid_body_sphere_list** v6 =
            &m_alloc_list[m_collision_callback.m_rb_colgeom_count];
        rigid_body_sphere_list* v7;
        if (v6 == m_alloc_list)
        {
            v7 = nullptr;
            goto LABEL_7;
        }
        while ((*m_alloc_list)->m_rb_id != i)
        {
            if (v6 == ++m_alloc_list)
            {
                v7 = nullptr;
                goto LABEL_7;
            }
        }
        v7 = *m_alloc_list;
        if (*m_alloc_list == nullptr)
            goto LABEL_7;
        goto LABEL_9;
    LABEL_7:
        if (_tlAssert("c:\\cod\\code\\game\\RBRagdoll.cpp", 1301, "rbsl",
                      defaultFileName))
            __debugbreak();
    LABEL_9:
        phys_gjk_cache_info* gjk_cache_info =
            g_phys_gjk_cache_system.get_gjk_cache_info(
                (unsigned int)v7->m_gjk_geom.m_geom_id, geom_id, true);
        if (gjk_cache_info != nullptr)
        {
            int v9 = gjk_cache_info->m_flags | 4;
            gjk_cache_info->m_flags = v9;
            gjk_cache_info->m_support_dir.v = separation_direction.v;
            gjk_cache_info->m_flags = v9 & 0xFFFFFFF7;
        }
    }
}

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

// ea: 0x700730
void rigid_body_sphere_list::init_tunnel_test(math::Position3& center_pos)
{
    m_tunnel_test_last_pos.v = center_pos.v;
    m_tunnel_test_radius = m_bounding_sphere_radius;
    rb_collision_sphere* p = m_slot_array;
    rb_collision_sphere* end = &m_slot_array[m_alloc_count];
    for (; p != end; ++p)
    {
        if (m_tunnel_test_radius > p->m_radius)
            m_tunnel_test_radius = p->m_radius;
    }
}

// ea: 0x7007A0
void phys_gjk_geom_ragdoll_1::init(rigid_body_sphere_list* cg)
{
    m_center.v = cg->m_bounding_sphere_center_loc.v;
    m_count = 0;
    rb_collision_sphere* p = cg->m_slot_array;
    rb_collision_sphere* end = &cg->m_slot_array[cg->m_alloc_count];
    for (; p != end; ++m_count)
    {
        if (m_count > 2)
        {
            if (!_tlAssert(
                    "c:\\cod\\code\\game\\RBRagdollCollision.cpp", 588,
                    "m_count <= 2", defaultFileName))
                __debugbreak();
        }
        m_list_center[m_count].v = p->m_center_loc.v;
        m_list_radius[m_count] = p->m_radius;
        ++p;
    }
    if (m_count == 2)
    {
        if (m_list_radius[0] > m_list_radius[1])
        {
            math::Dir3 tmp_center = m_list_center[0];
            m_list_center[0] = m_list_center[1];
            m_list_center[1] = tmp_center;
            float tmp_r = m_list_radius[0];
            m_list_radius[0] = m_list_radius[1];
            m_list_radius[1] = tmp_r;
        }
    }
    m_geom_radius = m_list_radius[0];
    m_list_radius[0] = 0.0f;
    if (m_count == 2)
    {
        if (m_list_radius[1] < 0.0f
            && _tlAssert("c:\\cod\\code\\game\\RBRagdollCollision.cpp", 609,
                         "m_list_radius[0] <= m_list_radius[1]",
                         defaultFileName))
            __debugbreak();
        float v9 = m_list_radius[1] - m_geom_radius;
        if (v9 < 0.0f)
            v9 = 0.0f;
        m_list_radius[1] = v9;
    }
}

// ea: 0x6FBE40
void phys_gjk_geom_ragdoll_1::comp_aabb(
    const math::Mat43& cg_to_world_xform)
{
    for (int i = 0; i < m_count; ++i)
    {
        math::Dir3 c;
        c.v = m_list_center[i].v;
        math::Dir3 r;
        r.v = _mm_set1_ps(m_list_radius[i] + m_geom_radius);
        math::Mat43 m = cg_to_world_xform;
        __m128 w = _mm_add_ps(
            _mm_add_ps(
                _mm_mul_ps(_mm_shuffle_ps(c.v, c.v, 0), m.x.v),
                _mm_mul_ps(_mm_shuffle_ps(c.v, c.v, 85), m.y.v)),
            _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(c.v, c.v, 170), m.z.v),
                       m.w.v));
        math::Dir3 mn, mx;
        mn.v = _mm_sub_ps(w, r.v);
        mx.v = _mm_add_ps(w, r.v);
        if (i != 0)
        {
            m_aabb_mn.v = _mm_min_ps(m_aabb_mn.v, mn.v);
            m_aabb_mx.v = _mm_max_ps(m_aabb_mx.v, mx.v);
        }
        else
        {
            m_aabb_mn = mn;
            m_aabb_mx = mx;
        }
    }
}

// ea: 0x704A50
void rigid_body_sphere_list::set(rigid_body* const owner)
{
    m_owner = owner;
    calc_bounding_sphere();
    m_gjk_geom.init(this);
    m_gjk_geom.m_geom_id = this;
    m_gjk_geom.m_next_geom = nullptr;
    m_tunnel_test_active_counter = 0;
}

// ea: 0x71A430
void rigid_body_sphere_list::calc_total_aabb()
{
    rigid_body* m_owner = this->m_owner;
    if ((m_owner->m_flags & 0x50) == 0
        && _tlAssert(
               "c:\\cod\\code\\tl\\physics\\include\\rigid_body.h", 109,
               "debug_flag_is_in_collision()", defaultFileName))
        __debugbreak();
    // world center = bounding sphere center transformed by owner's col mat
    __m128 c = m_bounding_sphere_center_loc.v;
    __m128 world = _mm_add_ps(
        _mm_add_ps(
            _mm_mul_ps(_mm_shuffle_ps(c, c, 0), m_owner->m_col_mat.x.v),
            _mm_mul_ps(_mm_shuffle_ps(c, c, 85), m_owner->m_col_mat.y.v)),
        _mm_add_ps(
            _mm_mul_ps(_mm_shuffle_ps(c, c, 170), m_owner->m_col_mat.z.v),
            m_owner->m_col_mat.w.v));
    __m128 radius = _mm_setr_ps(m_bounding_sphere_radius,
                                m_bounding_sphere_radius,
                                m_bounding_sphere_radius, 0.0f);
    m_total_aabb_mn.v =
        _mm_sub_ps(_mm_min_ps(world, m_tunnel_test_last_pos.v), radius);
    m_total_aabb_mx.v =
        _mm_add_ps(_mm_max_ps(world, m_tunnel_test_last_pos.v), radius);
}

// ea: 0x71C450
void rigid_body_sphere_list::update_colgeom()
{
    rigid_body* m_owner = this->m_owner;
    if ((m_owner->m_flags & 0x50) == 0
        && _tlAssert(
               "c:\\cod\\code\\tl\\physics\\include\\rigid_body.h", 109,
               "debug_flag_is_in_collision()", defaultFileName))
        __debugbreak();
    m_capsule.xform(m_owner->m_col_mat);
    rb_collision_sphere* end = m_slot_array + m_alloc_count;
    for (rb_collision_sphere* s = m_slot_array; s != end; ++s)
    {
        if ((m_owner->m_flags & 0x50) == 0
            && _tlAssert(
                   "c:\\cod\\code\\tl\\physics\\include\\rigid_body.h", 109,
                   "debug_flag_is_in_collision()", defaultFileName))
            __debugbreak();
    }
}

// ea: 0x71DBF0
void rigid_body_sphere_list::tunnel_test_prolog(
    physics_colgeom_visitor* visitor)
{
    rigid_body* m_owner = this->m_owner;
    if ((m_owner->m_flags & 0x50) == 0
        && _tlAssert(
               "c:\\cod\\code\\tl\\physics\\include\\rigid_body.h", 109,
               "debug_flag_is_in_collision()", defaultFileName))
        __debugbreak();
    __m128 c = m_bounding_sphere_center_loc.v;
    __m128 world = _mm_add_ps(
        _mm_add_ps(
            _mm_mul_ps(_mm_shuffle_ps(c, c, 0), m_owner->m_col_mat.x.v),
            _mm_mul_ps(_mm_shuffle_ps(c, c, 85), m_owner->m_col_mat.y.v)),
        _mm_add_ps(
            _mm_mul_ps(_mm_shuffle_ps(c, c, 170), m_owner->m_col_mat.z.v),
            m_owner->m_col_mat.w.v));
    __m128 delta = _mm_sub_ps(world, m_tunnel_test_last_pos.v);
    __m128 d2 = _mm_mul_ps(delta, delta);
    float len_sq = d2.m128_f32[0] + (d2.m128_f32[1] + d2.m128_f32[2]);
    float radius = m_tunnel_test_radius * 0.5f;
    if (len_sq > radius * radius)
    {
        float dist = sqrtf(len_sq);
        float t_ = (dist + radius) / dist;
        math::Position3 p0;
        p0.v = m_tunnel_test_last_pos.v;
        math::Position3 p1;
        p1.v = _mm_add_ps(m_tunnel_test_last_pos.v,
                          _mm_mul_ps(delta, _mm_set1_ps(t_)));
        if (collide_ray_object_list(
                visitor, m_total_aabb_mn, m_total_aabb_mx,
                reinterpret_cast<const math::Dir3&>(p0),
                reinterpret_cast<const math::Dir3&>(p1), &t_))
        {
            m_tunnel_hit_pos.v =
                _mm_add_ps(m_tunnel_test_last_pos.v,
                           _mm_mul_ps(delta, _mm_set1_ps(t_ - 1.0f)));
            if (m_tunnel_test_active_counter < 0x2D)
                ++m_tunnel_test_active_counter;
        }
        else
        {
            m_tunnel_hit_pos.v = _mm_setzero_ps();
            m_tunnel_test_active_counter = 0;
        }
    }
    else
    {
        m_tunnel_hit_pos.v = _mm_setzero_ps();
        m_tunnel_test_active_counter = 0;
    }
    m_owner->translate_col_mat(
        reinterpret_cast<const math::Dir3&>(m_tunnel_hit_pos));
    m_tunnel_test_last_pos.v =
        _mm_add_ps(world, m_tunnel_hit_pos.v);
}

// ?add_active_eps_aabb@@YAXPAVDir3@math@@0@Z (physics.o inline 0x6F2080).
// Expands the AABB by the active-eps pad (3.4 units each axis; local static
// active_eps_vec {3.4,3.4,3.4,0}).
void add_active_eps_aabb(math::Dir3* aabb_mn, math::Dir3* aabb_mx)
{
    if (aabb_mn == aabb_mx
        && _tlAssert("c:\\cod\\code\\game\\RBCollision.h", 0x112,
                     "aabb_mn != aabb_mx", defaultFileName))
        __debugbreak();
    static const __m128 active_eps_vec =
        _mm_setr_ps(3.4f, 3.4f, 3.4f, 0.0f);
    aabb_mn->v = _mm_sub_ps(aabb_mn->v, active_eps_vec);
    aabb_mx->v = _mm_add_ps(aabb_mx->v, active_eps_vec);
}

// rb_capsule_pair (physics.o; m_b1_cg/m_b2_cg)
class rb_capsule_pair {
public:
    rigid_body_sphere_list* m_b1_cg;  // +0x00
    rigid_body_sphere_list* m_b2_cg;  // +0x04

    void do_test(void* const col_resp_group);  // ?do_test@rb_capsule_pair@@QAEXQAX@Z
};

// ea: 0x718AF0
void* phys_memory_heap::allocate(int size, int alignment, bool no_error,
                                 const char* error_msg)
{
    if (size <= 0
        && _tlAssert("c:/cod/code/tl/physics/include\\phys_mem.h", 73,
                     "size > 0", defaultFileName))
        __debugbreak();
    char* result =
        (char*)((~(alignment - 1))
                & (intptr_t)&m_buffer_cur[alignment - 1]);
    if (&result[size] > m_buffer_end)
    {
        if (!no_error
            && _tlAssert("c:/cod/code/tl/physics/include\\phys_mem.h", 82,
                         "0", error_msg))
            __debugbreak();
        return nullptr;
    }
    m_buffer_cur = &result[size];
    return result;
}

// ea: 0x6F1180
void contact_point_info::set(float fric_coef, float bounce_coef,
                             float max_restitution_vel,
                             bool no_overflow_error)
{
    m_bounce_coef = bounce_coef;
    m_flags = 0;
    m_fric_coef = fric_coef;
    m_max_restitution_vel = max_restitution_vel;
    m_flags = no_overflow_error ? 2 : 0;
    if (fric_coef < 0.0
        && _tlAssert(
               "c:\\cod\\code\\tl\\physics\\include\\rbc_defs\\rbc_def_contact.h",
               100, "m_fric_coef >= 0.0f", defaultFileName))
        __debugbreak();
    if ((m_bounce_coef > 1.0 || m_bounce_coef < 0.0)
        && _tlAssert(
               "c:\\cod\\code\\tl\\physics\\include\\rbc_defs\\rbc_def_contact.h",
               101, "m_bounce_coef <= 1.0f && m_bounce_coef >= 0.0f",
               defaultFileName))
        __debugbreak();
}

// ea: 0x718F20
contact_point_info* contact_point_info::create_cpi(
    int point_pair_count, bool no_error, phys_memory_heap* allocater)
{
    if (point_pair_count <= 0
        && _tlAssert(
               "c:\\cod\\code\\tl\\physics\\include\\rbc_defs\\rbc_def_contact.h",
               117, "point_pair_count > 0", defaultFileName))
        __debugbreak();
    contact_point_info* v3 = (contact_point_info*)allocater->allocate(
        (point_pair_count << 6) + 80, 16, no_error,
        "contact_point_info buffer overflow");
    if (v3 == nullptr)
        return v3;
    v3->m_list_b1_r_loc = (math::Dir3*)(v3 + 1);
    v3->m_list_b2_r_loc = v3->m_list_b1_r_loc + point_pair_count;
    v3->m_list_pulse_sum_cache_info =
        (pulse_sum_cache_info*)(v3->m_list_b2_r_loc + point_pair_count);
    v3->m_point_pair_count = point_pair_count;
    if (allocater->m_buffer_cur
        == (char*)(v3->m_list_pulse_sum_cache_info
                   + 2 * point_pair_count))
        return v3;
    if (_tlAssert(
            "c:\\cod\\code\\tl\\physics\\include\\rbc_defs\\rbc_def_contact.h",
            127,
            "allocater->get_buffer_cur() == (char *)(cpi->m_list_pulse_sum_cache_info + point_pair_count)",
            defaultFileName))
        __debugbreak();
    return v3;
}

// ea: 0x718D90
void contact_point_info::get_closest_psc(
    const math::Dir3& normal, const math::Dir3& b1_r_loc,
    const math::Dir3& b2_r_loc, float* closest_error,
    const pulse_sum_cache_info** closest_psc) const
{
    __m128 v7 = _mm_sub_ps(normal.v, m_normal.v);
    __m128 v8 = _mm_mul_ps(v7, v7);
    float v17 =
        v8.m128_f32[0]
        + (_mm_shuffle_ps(v8, v8, 85).m128_f32[0]
           + _mm_shuffle_ps(v8, v8, 170).m128_f32[0]);
    math::Dir3* m_list_b1_r_loc = this->m_list_b1_r_loc;
    math::Dir3* m_list_b2_r_loc = this->m_list_b2_r_loc;
    int v18 = 0;
    if (m_point_pair_count > 0)
    {
        int v19 = 0;
        do
        {
            __m128 v11 = _mm_sub_ps(m_list_b1_r_loc->v, b1_r_loc.v);
            __m128 v12 = _mm_mul_ps(v11, v11);
            float v16 =
                v12.m128_f32[0]
                + (_mm_shuffle_ps(v12, v12, 85).m128_f32[0]
                   + _mm_shuffle_ps(v12, v12, 170).m128_f32[0]);
            __m128 v13 = _mm_sub_ps(m_list_b2_r_loc->v, b2_r_loc.v);
            __m128 v14 = _mm_mul_ps(v13, v13);
            float v15 =
                (v14.m128_f32[0]
                 + (_mm_shuffle_ps(v14, v14, 85).m128_f32[0]
                    + _mm_shuffle_ps(v14, v14, 170).m128_f32[0]))
                + v16 + v17;
            if (*closest_error > v15)
            {
                *closest_error = v15;
                *closest_psc = &m_list_pulse_sum_cache_info[v19];
            }
            ++v18;
            v19 += 2;
            ++m_list_b1_r_loc;
            ++m_list_b2_r_loc;
        } while (v18 < m_point_pair_count);
    }
}

// ea: 0x71BA70
void contact_point_info::set_closest_cached_psc(
    const contact_point_info* cached_cpi, const math::Dir3& normal,
    const math::Dir3& b1_r_loc, const math::Dir3& b2_r_loc,
    pulse_sum_cache_info* psc)
{
    const contact_point_info* v5 = cached_cpi;
    const pulse_sum_cache_info* closest_psc = nullptr;
    float closest_error = 10000000.0f;
    if (cached_cpi != nullptr)
    {
        do
        {
            v5->get_closest_psc(normal, b1_r_loc, b2_r_loc, &closest_error,
                                &closest_psc);
            v5 = v5->m_next_link;
        } while (v5 != nullptr);
    }
    if (closest_psc != nullptr)
    {
        memcpy(psc, closest_psc, 0x20u);
    }
    else
    {
        if (psc != nullptr)
        {
            psc->m_ps_cache_list[0].m_visit_key = -1;
            psc->m_ps_cache_list[1].m_visit_key = -1;
            psc[1].m_ps_cache_list[0].m_visit_key = -1;
        }
    }
}

// ea: 0x718A10
void PHYS_ASSERT_UNIT(const math::Dir3& v)
{
    __m128 v1 = _mm_mul_ps(v.v, v.v);
    if (fabs(sqrtf(v1.m128_f32[0]
                   + (_mm_shuffle_ps(v1, v1, 85).m128_f32[0]
                      + _mm_shuffle_ps(v1, v1, 170).m128_f32[0]))
             - 1.0)
            >= 0.001
        && _tlAssert("c:\\cod\\code\\tl\\physics\\include\\phys_math.h", 341,
                     "fabsf(nv - 1.0f) < 0.001f", defaultFileName))
        __debugbreak();
}

// ea: 0x71BB10
void rigid_body_constraint_contact::add_point_list(
    rigid_body* b1_, rigid_body* b2_, const math::Dir3* list_b1_r_loc,
    const math::Dir3* list_b2_r_loc, int num_points,
    const math::Dir3& normal_, float fric_coef, float bounce_coef,
    float max_restitution_vel, bool no_overflow_error)
{
    if (((unsigned int)list_b1_r_loc & 0xF) != 0
        && _tlAssert(
               "c:\\cod\\code\\tl\\physics\\include\\phys_math.h", 365,
               "uint(v) % PHYS_ALIGNOF(phys_vec3) == 0", defaultFileName))
        __debugbreak();
    if (((unsigned int)list_b2_r_loc & 0xF) != 0
        && _tlAssert(
               "c:\\cod\\code\\tl\\physics\\include\\phys_math.h", 365,
               "uint(v) % PHYS_ALIGNOF(phys_vec3) == 0", defaultFileName))
        __debugbreak();
    verify_constraint(b1_, b2_);
    phys_memory_heap* cpi_allocater = contact_point_info::get_cpi_allocater();
    contact_point_info* cpi = contact_point_info::create_cpi(
        num_points, no_overflow_error, cpi_allocater);
    if (cpi != nullptr)
    {
        cpi->m_next_link = m_list_contact_point_info_buffer_1.m_first;
        m_list_contact_point_info_buffer_1.m_first = cpi;
        cpi->set(fric_coef, bounce_coef, max_restitution_vel,
                 no_overflow_error);
        if (b1 == b1_)
        {
            cpi->m_normal.v = normal_.v;
            PHYS_ASSERT_UNIT(cpi->m_normal);
        }
        else
        {
            cpi->m_normal.v = _mm_xor_ps(Float4_SignMask_12, normal_.v);
            PHYS_ASSERT_UNIT(cpi->m_normal);
            const math::Dir3* v14 = list_b1_r_loc;
            list_b1_r_loc = list_b2_r_loc;
            list_b2_r_loc = v14;
        }
        if (cpi->m_point_pair_count != num_points
            && _tlAssert(
                   "c:\\cod\\code\\tl\\physics\\include\\rbc_defs\\rbc_def_contact.h",
                   241, "cpi->m_point_pair_count == num_points",
                   defaultFileName))
            __debugbreak();
        contact_point_info::pulse_sum_cache_info* m_list_pulse_sum_cache_info =
            cpi->m_list_pulse_sum_cache_info;
        math::Dir3* last_psci_i = cpi->m_list_b1_r_loc;
        math::Dir3* m_list_b2_r_loc = cpi->m_list_b2_r_loc;
        for (contact_point_info::pulse_sum_cache_info* i =
                 &m_list_pulse_sum_cache_info[2 * cpi->m_point_pair_count];
             m_list_pulse_sum_cache_info != i; ++list_b2_r_loc)
        {
            last_psci_i->v = list_b1_r_loc->v;
            m_list_b2_r_loc->v = list_b2_r_loc->v;
            contact_point_info::set_closest_cached_psc(
                m_list_contact_point_info_buffer_2.m_first, cpi->m_normal,
                *last_psci_i++, *m_list_b2_r_loc++,
                m_list_pulse_sum_cache_info);
            m_list_pulse_sum_cache_info += 2;
            ++list_b1_r_loc;
        }
    }
}

// world -> body-local transform (transpose rows of m_col_mat)
static inline math::Dir3 world_to_local(const rigid_body* rb,
                                        const math::Position3& p)
{
    math::Mat43 m = rb->m_col_mat;
    math::Dir3 r;
    for (int i = 0; i < 3; ++i)
    {
        float row[4];
        row[0] = m.x.v.m128_f32[i];
        row[1] = m.y.v.m128_f32[i];
        row[2] = m.z.v.m128_f32[i];
        row[3] = m.w.v.m128_f32[i];
        r.v.m128_f32[i] = row[0] * p.v.m128_f32[0]
                          + row[1] * p.v.m128_f32[1]
                          + row[2] * p.v.m128_f32[2] + row[3];
    }
    r.v.m128_f32[3] = 0.0f;
    return r;
}

bool collide_crapsule_crapsule(const math::Position3& a1,
                               const math::Position3& a2, float r1,
                               const math::Position3& b1,
                               const math::Position3& b2, float r2,
                               math::Position3* p1, math::Position3* p2,
                               math::Dir3* normal);  // 0x6FB740

// ea: 0x7004D0
void rb_capsule_pair::do_test(void* const col_resp_group)
{
    (void)col_resp_group;
    math::Position3 p1_world, p2_world;
    math::Dir3 normal;
    if (collide_crapsule_crapsule(
            m_b1_cg->m_capsule.m_p1, m_b1_cg->m_capsule.m_p2,
            m_b1_cg->m_capsule.m_r, m_b2_cg->m_capsule.m_p1,
            m_b2_cg->m_capsule.m_p2, m_b2_cg->m_capsule.m_r, &p1_world,
            &p2_world, &normal))
    {
        rigid_body_constraint_contact* rbc_contact =
            phys_sys::create_rbc_contact(m_b1_cg->m_owner,
                                         m_b2_cg->m_owner, false);
        if (rbc_contact != nullptr)
        {
            rigid_body* b1 = m_b1_cg->m_owner;
            rigid_body* b2 = m_b2_cg->m_owner;
            if ((b1->m_flags & 0x50) == 0
                && _tlAssert(
                       "c:\\cod\\code\\tl\\physics\\include\\rigid_body.h",
                       109, "debug_flag_is_in_collision()", defaultFileName))
                __debugbreak();
            if ((b2->m_flags & 0x50) == 0
                && _tlAssert(
                       "c:\\cod\\code\\tl\\physics\\include\\rigid_body.h",
                       109, "debug_flag_is_in_collision()", defaultFileName))
                __debugbreak();
            math::Dir3 b1_r = world_to_local(b1, p1_world);
            math::Dir3 b2_r = world_to_local(b2, p2_world);
            rbc_contact->add_point_list(b1, b2, &b1_r, &b2_r, 1, normal,
                                        0.0f, 0.0f, 3400.0f, false);
        }
    }
}

void mem_break();  // mem_heap.cpp

// ea: 0x6FB740 - segment-segment closest points + capsule overlap test.
// Reconstructed from the SIMD disassembly (Ericson segment-distance form).
static inline float dot3(const float* a, const float* b)
{
    return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

bool collide_crapsule_crapsule(const math::Position3& a1,
                               const math::Position3& a2, float r1,
                               const math::Position3& b1,
                               const math::Position3& b2, float r2,
                               math::Position3* p1, math::Position3* p2,
                               math::Dir3* normal)
{
    const float* d1 = &a2.v.m128_f32[0];
    const float* d2 = &b1.v.m128_f32[0];
    float da[3], db[3], dr[3];
    da[0] = a2.v.m128_f32[0] - a1.v.m128_f32[0];
    da[1] = a2.v.m128_f32[1] - a1.v.m128_f32[1];
    da[2] = a2.v.m128_f32[2] - a1.v.m128_f32[2];
    db[0] = b1.v.m128_f32[0] - b2.v.m128_f32[0];
    db[1] = b1.v.m128_f32[1] - b2.v.m128_f32[1];
    db[2] = b1.v.m128_f32[2] - b2.v.m128_f32[2];
    dr[0] = a1.v.m128_f32[0] - b1.v.m128_f32[0];
    dr[1] = a1.v.m128_f32[1] - b1.v.m128_f32[1];
    dr[2] = a1.v.m128_f32[2] - b1.v.m128_f32[2];

    float a = dot3(da, da);          // det
    float b = dot3(da, db);          // v68
    float e = dot3(db, db);          // v64
    float c = dot3(da, dr);          // s_
    float f = dot3(db, dr);          // v19
    float denom = e * a - b * b;     // v61

    float dist_sq = 0.0f;
    bool interior = false;
    if (fabs(denom) > 0.0000099999997f)
    {
        float s = (b * f - c * e) / denom;
        float t = (a * f - c * b) / denom;
        if (s >= 0.0f && s <= 1.0f && t >= 0.0f && t <= 1.0f)
        {
            for (int i = 0; i < 3; ++i)
            {
                p1->v.m128_f32[i] = a1.v.m128_f32[i] + da[i] * s;
                p2->v.m128_f32[i] = b1.v.m128_f32[i] - db[i] * t;
            }
            for (int i = 0; i < 3; ++i)
                normal->v.m128_f32[i] =
                    p2->v.m128_f32[i] - p1->v.m128_f32[i];
            dist_sq = dot3(&normal->v.m128_f32[0], &normal->v.m128_f32[0]);
            interior = true;
        }
    }

    if (!interior)
    {
        // clamp on segment A
        float sa = c / a;
        if (sa < 0.0f)
            sa = 0.0f;
        else if (sa > 1.0f)
            sa = 1.0f;
        for (int i = 0; i < 3; ++i)
        {
            p1->v.m128_f32[i] = a1.v.m128_f32[i] + da[i] * sa;
            p2->v.m128_f32[i] = b1.v.m128_f32[i];
        }
        for (int i = 0; i < 3; ++i)
            normal->v.m128_f32[i] =
                p2->v.m128_f32[i] - p1->v.m128_f32[i];
        float d1_sq = dot3(&normal->v.m128_f32[0], &normal->v.m128_f32[0]);
        dist_sq = d1_sq;

        // clamp on segment B
        float tb = f / e;
        if (tb < 0.0f)
            tb = 0.0f;
        else if (tb > 1.0f)
            tb = 1.0f;
        float n4[3], an4[3];
        for (int i = 0; i < 3; ++i)
        {
            n4[i] = b1.v.m128_f32[i] - db[i] * tb;
            an4[i] = n4[i] - a1.v.m128_f32[i];
        }
        float d2_sq = dot3(an4, an4);
        if (d1_sq > d2_sq)
        {
            for (int i = 0; i < 3; ++i)
            {
                p1->v.m128_f32[i] = a1.v.m128_f32[i];
                p2->v.m128_f32[i] = n4[i];
                normal->v.m128_f32[i] = an4[i];
            }
            dist_sq = d2_sq;
        }

        // a2 endpoint vs segment B
        float v50[3];
        v50[0] = a2.v.m128_f32[0] - b2.v.m128_f32[0];
        v50[1] = a2.v.m128_f32[1] - b2.v.m128_f32[1];
        v50[2] = a2.v.m128_f32[2] - b2.v.m128_f32[2];
        float s2 = dot3(v50, db) / e;
        if (s2 < 0.0f)
            s2 = 0.0f;
        else if (s2 > 1.0f)
            s2 = 1.0f;
        float n4a[3], an4a[3];
        for (int i = 0; i < 3; ++i)
        {
            n4a[i] = b2.v.m128_f32[i] + db[i] * s2;
            an4a[i] = n4a[i] - a2.v.m128_f32[i];
        }
        float d3_sq = dot3(an4a, an4a);
        if (dist_sq > d3_sq)
        {
            for (int i = 0; i < 3; ++i)
            {
                p1->v.m128_f32[i] = a2.v.m128_f32[i];
                p2->v.m128_f32[i] = n4a[i];
                normal->v.m128_f32[i] = an4a[i];
            }
            dist_sq = d3_sq;
        }

        // b2 endpoint vs segment A
        float s3 = dot3(v50, da) / a;
        if (s3 < 0.0f)
            s3 = 0.0f;
        else if (s3 > 1.0f)
            s3 = 1.0f;
        float p1b[3], an4b[3];
        for (int i = 0; i < 3; ++i)
        {
            p1b[i] = a2.v.m128_f32[i] - da[i] * s3;
            an4b[i] = b2.v.m128_f32[i] - p1b[i];
        }
        float d4_sq = dot3(an4b, an4b);
        if (dist_sq > d4_sq)
        {
            for (int i = 0; i < 3; ++i)
            {
                p1->v.m128_f32[i] = p1b[i];
                p2->v.m128_f32[i] = b2.v.m128_f32[i];
                normal->v.m128_f32[i] = an4b[i];
            }
            dist_sq = d4_sq;
        }
    }

    if (dist_sq <= 0.0000099999997f
        || ((r1 + r2) * (r1 + r2)) < dist_sq)
        return false;
    float len = sqrtf(dist_sq);
    for (int i = 0; i < 3; ++i)
        normal->v.m128_f32[i] /= len;
    for (int i = 0; i < 3; ++i)
        p1->v.m128_f32[i] += normal->v.m128_f32[i] * r1;
    for (int i = 0; i < 3; ++i)
        p2->v.m128_f32[i] -= normal->v.m128_f32[i] * r2;
    return true;
}

// ea: 0x700D10
rigid_body_sphere_list* ragdoll_collision_callback::get_colgeom(int rb_id)
{
    for (int i = 0; i < m_rb_colgeom_count; ++i)
    {
        rigid_body_sphere_list* cg = m_rb_colgeom_alloc_list[i];
        if (cg->m_rb_id == rb_id)
            return cg;
    }
    return nullptr;
}

// ea: 0x704EA0
rigid_body_sphere_list* ragdoll_collision_callback::add_colgeom(int rb_id)
{
    if (get_colgeom(rb_id) != nullptr
        && _tlAssert("c:\\cod\\code\\game\\RBRagdollCollision.cpp", 1051,
                     "get_colgeom(rb_id) == NULL", defaultFileName))
        __debugbreak();
    // phys_static_memory_pool<rigid_body_sphere_list,10>::add
    int count = m_rb_colgeom_count;
    rigid_body_sphere_list* v3 = nullptr;
    if (count < 10)
    {
        v3 = m_rb_colgeom_alloc_list[count];
        m_rb_colgeom_count = count + 1;
        if (v3 != nullptr)
            new (v3) rigid_body_sphere_list;
    }
    if (v3 == nullptr)
        mem_break();
    v3->m_rb_id = rb_id;
    return v3;
}

// ea: 0x708DC0
void ragdoll_collision_callback::remove_colgeom(int rb_id)
{
    rigid_body_sphere_list* colgeom = get_colgeom(rb_id);
    rigid_body_sphere_list* v4 = colgeom;
    if (colgeom != nullptr)
    {
        // phys_static_memory_pool<rigid_body_sphere_list,10>::remove
        int count = m_rb_colgeom_count;
        int idx = -1;
        for (int i = 0; i < count; ++i)
        {
            if (m_rb_colgeom_alloc_list[i] == v4)
            {
                idx = i;
                break;
            }
        }
        if (idx >= 0)
        {
            if (count > 1)
            {
                int last = count - 1;
                m_rb_colgeom_count = last;
                m_rb_colgeom_alloc_list[idx] =
                    m_rb_colgeom_alloc_list[last];
            }
            else
            {
                m_rb_colgeom_count = 0;
            }
        }
        // remove rb_capsule_pair entries referencing this colgeom
        int cp_count = m_rb_cp_count;
        int i = 0;
        while (i < cp_count)
        {
            rb_capsule_pair* pair = m_rb_cp_alloc_list[i];
            if (pair->m_b1_cg == v4 || pair->m_b2_cg == v4)
            {
                if (cp_count > 1)
                {
                    int last = cp_count - 1;
                    m_rb_cp_count = last;
                    m_rb_cp_alloc_list[i] =
                        m_rb_cp_alloc_list[last];
                }
                else
                {
                    m_rb_cp_count = 0;
                }
                cp_count = m_rb_cp_count;
            }
            else
            {
                ++i;
            }
        }
    }
}

// ea: 0x6F46A0
void ragdoll_collision_callback::set(Entity* const owner)
{
    m_owner = owner;
}

// ?m_current_owner@@3PAVEntity@@A (physics.o data @ 0x13689AC)
Entity* m_current_owner = nullptr;

// ea: 0x70BD00
void ragdoll_collision_callback::get_all_collisions()
{
    m_current_owner = m_owner;
    collide_terrain();

    math::Dir3 aabb_mn, aabb_mx;
    bool first = true;
    for (int i = 0; i < m_rb_colgeom_count; ++i)
    {
        rigid_body_sphere_list* rbsl = m_rb_colgeom_alloc_list[i];
        if (first)
        {
            aabb_mn.v = rbsl->m_gjk_geom.m_aabb_mn.v;
            aabb_mx.v = rbsl->m_gjk_geom.m_aabb_mx.v;
            first = false;
        }
        else
        {
            aabb_mn.v = _mm_min_ps(aabb_mn.v, rbsl->m_gjk_geom.m_aabb_mn.v);
            aabb_mx.v = _mm_max_ps(aabb_mx.v, rbsl->m_gjk_geom.m_aabb_mx.v);
        }
    }
    for (int i = 0; i < m_rb_cp_count; ++i)
        m_rb_cp_alloc_list[i]->do_test(m_owner);

    environment_rigid_body* env = phys_sys::get_environment_rigid_body();
    if ((env->m_flags & 0x50) == 0
        && _tlAssert(
               "c:\\cod\\code\\tl\\physics\\include\\rigid_body.h", 109,
               "debug_flag_is_in_collision()", defaultFileName))
        __debugbreak();
    const math::Mat43* env_col_mat = &env->m_col_mat;

    int count = g_list_rb_extra_info.m_alloc_count;
    for (int ri = 0; ri < count; ++ri)
    {
        rb_extra_info* rb_inf = g_list_rb_extra_info.m_alloc_list[ri];
        phys_gjk_geom_list* geom_list =
            (phys_gjk_geom_list*)rb_inf->m_gjk_geom_list;
        if (geom_list == nullptr)
            continue;
        if (!are_potentially_colliding(
                &aabb_mn, &aabb_mx, &geom_list->m_aabb_mn,
                &geom_list->m_aabb_mx))
            continue;
        for (int ci = 0; ci < m_rb_colgeom_count; ++ci)
        {
            rigid_body_sphere_list* rbsl = m_rb_colgeom_alloc_list[ci];
            rigid_body* rbsl_rb = rbsl->m_owner;
            for (void* g = geom_list->m_first_geom; g != nullptr;
                 g = *(void**)((char*)g + 0x34))
            {
                if (!are_potentially_colliding(
                        &rbsl->m_gjk_geom.m_aabb_mn,
                        &rbsl->m_gjk_geom.m_aabb_mx,
                        (math::Dir3*)((char*)g + 0x10),
                        (math::Dir3*)((char*)g + 0x20)))
                    continue;
                float fric_coef = rbsl_rb->m_fric_coef;
                rigid_body* rb2 = rb_inf->m_rb;
                if ((rb2->m_flags & 0x50) == 0
                    && _tlAssert(
                           "c:\\cod\\code\\tl\\physics\\include\\rigid_body.h",
                           109, "debug_flag_is_in_collision()",
                           defaultFileName))
                    __debugbreak();
                if ((rbsl_rb->m_flags & 0x50) == 0
                    && _tlAssert(
                           "c:\\cod\\code\\tl\\physics\\include\\rigid_body.h",
                           109, "debug_flag_is_in_collision()",
                           defaultFileName))
                    __debugbreak();
                phys_collide_data* v23 = g_list_phys_collide_data;
                v23->gjk_cg1 = (const phys_gjk_geom*)&rbsl->m_gjk_geom;
                v23->gjk_cg2 = (const phys_gjk_geom*)g;
                v23->cg1_to_world_xform = &rbsl_rb->m_col_mat;
                v23->cg2_to_world_xform = rb_inf->m_cg_mesh_mat;
                v23->cg1_to_rb1_xform = env_col_mat;
                v23->rb2_to_world_xform = &rb2->m_col_mat;
                v23->rb1 = rbsl_rb;
                v23->rb2 = rb2;
                v23->gjk_ci = nullptr;
                v23->pcd_callback = nullptr;
                v23->gjk_info = g_gjk_info;
                v23->cman_process = g_cman_process;
                v23->id1 = (unsigned int)rbsl->m_gjk_geom.m_geom_id;
                v23->fric_coef = fric_coef;
                v23->id2 = *(unsigned int*)((char*)g + 0x30);
                v23->bounce_coef = 0.5f;
                v23->no_overflow_error = false;
                v23->solver_priority = 0;
                v23->gjk_ci = g_phys_gjk_cache_system.get_gjk_cache_info(
                    v23->id1, v23->id2, true);
                phys_collide_do_gjk_collide_and_contact_manifold(v23);
            }
        }
    }
    m_current_owner = nullptr;
}

// ragdoll_collision_callback::collide_terrain - ea: 0x70B300
//
// Gathers the ragdoll sphere-list AABBs, expands by the active-eps pad,
// generates the local primitive (terrain) list and runs GJK collide requests
// for each terrain object against the first sphere-list geometry.
void ragdoll_collision_callback::collide_terrain()
{
    math::Dir3 aabb_mn, aabb_mx;
    bool first = true;
    for (int i = 0; i < m_rb_colgeom_count; ++i)
    {
        rigid_body_sphere_list* rbsl = m_rb_colgeom_alloc_list[i];
        rbsl->calc_total_aabb();
        if (first)
        {
            aabb_mn.v = rbsl->m_total_aabb_mn.v;
            aabb_mx.v = rbsl->m_total_aabb_mx.v;
            first = false;
        }
        else
        {
            aabb_mn.v = _mm_min_ps(aabb_mn.v, rbsl->m_total_aabb_mn.v);
            aabb_mx.v = _mm_max_ps(aabb_mx.v, rbsl->m_total_aabb_mx.v);
        }
    }
    add_active_eps_aabb(&aabb_mn, &aabb_mx);
    physics_colgeom_visitor* g1 =
        generate_local_primitive_list(aabb_mn, aabb_mx, 65);
    for (int i = 0; i < m_rb_colgeom_count; ++i)
    {
        rigid_body_sphere_list* rbsl = m_rb_colgeom_alloc_list[i];
        rbsl->tunnel_test_prolog(g1);
        rbsl->update_colgeom();
        if ((rbsl->m_owner->m_flags & 0x50) == 0
            && _tlAssert(
                   "c:\\cod\\code\\tl\\physics\\include\\rigid_body.h", 109,
                   "debug_flag_is_in_collision()", defaultFileName))
            __debugbreak();
        rbsl->m_gjk_geom.comp_aabb(rbsl->m_owner->m_col_mat);
    }
    environment_rigid_body* ent_i = phys_sys::get_environment_rigid_body();
    if ((ent_i->m_flags & 0x50) == 0
        && _tlAssert("c:\\cod\\code\\tl\\physics\\include\\rigid_body.h", 109,
                     "debug_flag_is_in_collision()", defaultFileName))
        __debugbreak();

    // The ragdoll-side geometry/body (first sphere-list) for the GJK pair.
    rigid_body* b2w = nullptr;
    phys_gjk_geom_cod_base* ragdoll_geom = nullptr;
    if (m_rb_colgeom_count > 0)
    {
        b2w = m_rb_colgeom_alloc_list[0]->m_owner;
        ragdoll_geom =
            (phys_gjk_geom_cod_base*)&m_rb_colgeom_alloc_list[0]->m_gjk_geom;
    }
    int object_count = g1->m_object_list_count;
    for (int obj_inf_i = 0; obj_inf_i < object_count; ++obj_inf_i)
    {
        CGBank* bank = g1->m_object_list[obj_inf_i].m_bank;
        int index = g1->m_object_list[obj_inf_i].m_index;
        if ((unsigned int)index >= (unsigned int)bank->objects.m_count)
        {
            AeAssert::gCurrentAuthor = AeAssert::JSV;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\cgbank.h";
            AeAssert::gCurrentLine = 233;
            AeAssert::gCurrentExpr = "index < size()";
            if (!AeAssert::IsIgnored() && AeAssert::Assert(defaultFileName))
                __debugbreak();
        }
        cdl_object_t* obj =
            &((cdl_object_t*)bank->objects.m_elements)[index];
        bank->get_type(index);
        math::Dir3 b2_mn, b2_mx;
        b2_mn.v = _mm_setr_ps(obj->center[0] - obj->box_radius[0],
                              obj->center[1] - obj->box_radius[1],
                              obj->center[2] - obj->box_radius[2], 0.0f);
        b2_mx.v = _mm_setr_ps(obj->center[0] + obj->box_radius[0],
                              obj->center[1] + obj->box_radius[1],
                              obj->center[2] + obj->box_radius[2], 0.0f);
        if (!are_potentially_colliding(
                &ragdoll_geom->m_aabb_mn, &ragdoll_geom->m_aabb_mx, &b2_mn,
                &b2_mx))
            continue;
        phys_gjk_geom_cod_base* m_gjk_geom = nullptr;
        if (ragdoll_geom == nullptr || b2w == nullptr)
            continue;
        m_gjk_geom = g_gjk_geom_database->get_gjk_geom(obj, index, bank);
        if (m_gjk_geom == nullptr)
            continue;
        if ((ent_i->m_flags & 0x50) == 0
            && _tlAssert(
                   "c:\\cod\\code\\tl\\physics\\include\\rigid_body.h", 109,
                   "debug_flag_is_in_collision()", defaultFileName))
            __debugbreak();
        phys_collide_data* v40 = g_list_phys_collide_data;
        float fric_coef = b2w->m_fric_coef;
        v40->gjk_cg1 = (const phys_gjk_geom*)ragdoll_geom;
        v40->gjk_cg2 = (const phys_gjk_geom*)m_gjk_geom;
        v40->cg1_to_world_xform = &b2w->m_col_mat;
        v40->cg2_to_world_xform = &ent_i->m_col_mat;
        v40->cg1_to_rb1_xform = &b2w->m_col_mat;
        v40->rb2_to_world_xform = &ent_i->m_col_mat;
        v40->rb1 = b2w;
        v40->rb2 = ent_i;
        v40->gjk_ci = nullptr;
        v40->pcd_callback = nullptr;
        v40->gjk_info = g_gjk_info;
        v40->id1 = (unsigned int)ragdoll_geom->m_geom_id;
        v40->fric_coef = fric_coef;
        v40->id2 = (unsigned int)m_gjk_geom->m_geom_id;
        v40->bounce_coef = 0.5f;
        v40->cman_process = g_cman_process;
        v40->no_overflow_error = false;
        v40->solver_priority = 1;
        v40->gjk_ci = g_phys_gjk_cache_system.get_gjk_cache_info(
            v40->id1, v40->id2, true);
        phys_collide_do_gjk_collide_and_contact_manifold(v40);
    }
}

// ea: 0x700910
void ragdoll_collision_callback::process_environment_collision_events()
{
    if (m_rb_colgeom_count <= 0
        && _tlAssert(
               "c:\\cod\\code\\tl\\physics\\include\\phys_memory_pool_base.inc",
               178, "i >= 0 && i < m_alloc_count", defaultFileName))
        __debugbreak();
    if (m_rb_colgeom_alloc_list[0]->m_rb_id != 0
        && _tlAssert(
               "c:\\cod\\code\\game\\RBRagdollCollision.cpp", 888,
               "m_rb_colgeom[rb_torso].m_rb_id == rb_torso",
               defaultFileName))
        __debugbreak();
    if (m_rb_colgeom_count <= 0
        && _tlAssert(
               "c:\\cod\\code\\tl\\physics\\include\\phys_memory_pool_base.inc",
               178, "i >= 0 && i < m_alloc_count", defaultFileName))
        __debugbreak();
    rigid_body* torso_rb = m_rb_colgeom_alloc_list[0]->m_owner;
    rigid_body_constraint_contact* rbc_contact = phys_sys::get_rbc_contact(
        torso_rb, phys_sys::get_environment_rigid_body());
    if (rbc_contact != nullptr)
    {
        if (torso_rb != rbc_contact->b1
            && _tlAssert(
                   "c:\\cod\\code\\game\\RBRagdollCollision.cpp", 893,
                   "rigid_body_torso == rbc_contact_torso_environment->get_b1()",
                   defaultFileName))
            __debugbreak();
        const contact_point_info* max_intensity_cpi = nullptr;
        float max_intensity = 0.0f;
        for (const contact_point_info* cpi =
                 rbc_contact->m_list_contact_point_info_buffer_1.m_first;
             cpi != nullptr; cpi = cpi->m_next_link)
        {
            if (cpi->m_point_pair_count <= 0
                && _tlAssert(
                       "c:\\cod\\code\\game\\RBRagdollCollision.cpp", 900,
                       "cpi.m_point_pair_count > 0", defaultFileName))
                __debugbreak();
            __m128 v9 = _mm_mul_ps(cpi->m_normal.v, torso_rb->m_t_vel.v);
            float intensity =
                v9.m128_f32[0] + (v9.m128_f32[1] + v9.m128_f32[2]);
            if (intensity > max_intensity)
            {
                max_intensity = intensity;
                max_intensity_cpi = cpi;
            }
        }
        if (max_intensity > 50.0f)
        {
            if (max_intensity_cpi == nullptr
                && _tlAssert(
                       "c:\\cod\\code\\game\\RBRagdollCollision.cpp", 910,
                       "max_intensity_cpi", defaultFileName))
                __debugbreak();
            CollisionDesc col_desc;
            col_desc.simple.coord.v = max_intensity_cpi->m_list_b2_r_loc->v;
            col_desc.simple.normal.v =
                _mm_xor_ps(Float4_SignMask_12, max_intensity_cpi->m_normal.v);
            col_desc.material = (ECollisionMaterial)kCollisionMaterialNONE;
            PostEffectEventPhysicsImpact(m_owner, kCollisionMaterialFLESH,
                                         &col_desc, max_intensity);
        }
    }

    if ((m_owner->mFlags & 0x10) == 0)
    {
        if (m_rb_colgeom_count <= 1
            && _tlAssert(
                   "c:\\cod\\code\\tl\\physics\\include\\phys_memory_pool_base.inc",
                   178, "i >= 0 && i < m_alloc_count", defaultFileName))
            __debugbreak();
        if (m_rb_colgeom_alloc_list[1]->m_rb_id != 1
            && _tlAssert(
                   "c:\\cod\\code\\game\\RBRagdollCollision.cpp", 917,
                   "m_rb_colgeom[rb_head].m_rb_id == rb_head",
                   defaultFileName))
            __debugbreak();
        if (m_rb_colgeom_count <= 1
            && _tlAssert(
                   "c:\\cod\\code\\tl\\physics\\include\\phys_memory_pool_base.inc",
                   178, "i >= 0 && i < m_alloc_count", defaultFileName))
            __debugbreak();
        rigid_body* v12 = m_rb_colgeom_alloc_list[1]->m_owner;
        rigid_body_constraint_contact* v14 = phys_sys::get_rbc_contact(
            v12, phys_sys::get_environment_rigid_body());
        if (v14 != nullptr)
        {
            if (v12 != v14->b1
                && _tlAssert(
                       "c:\\cod\\code\\game\\RBRagdollCollision.cpp", 922,
                       "rigid_body_head == rbc_contact_head_environment->get_b1()",
                       defaultFileName))
                __debugbreak();
            const contact_point_info* max_intensity_cpi = nullptr;
            float max_intensity = 0.0f;
            for (const contact_point_info* cpi =
                     v14->m_list_contact_point_info_buffer_1.m_first;
                 cpi != nullptr; cpi = cpi->m_next_link)
            {
                if (cpi->m_point_pair_count <= 0
                    && _tlAssert(
                           "c:\\cod\\code\\game\\RBRagdollCollision.cpp",
                           929, "cpi.m_point_pair_count > 0",
                           defaultFileName))
                    __debugbreak();
                __m128 v18 =
                    _mm_mul_ps(cpi->m_normal.v, torso_rb->m_t_vel.v);
                float intensity =
                    v18.m128_f32[0] + (v18.m128_f32[1] + v18.m128_f32[2]);
                if (intensity > max_intensity)
                {
                    max_intensity = intensity;
                    max_intensity_cpi = cpi;
                }
            }
            if (max_intensity > 50.0f)
            {
                if (max_intensity_cpi == nullptr
                    && _tlAssert(
                           "c:\\cod\\code\\game\\RBRagdollCollision.cpp",
                           939, "max_intensity_cpi", defaultFileName))
                    __debugbreak();
                if (rand() % 100 <= 10)
                {
                    if (SpawnHelmet(
                            m_owner,
                            max_intensity_cpi->m_list_b2_r_loc->v.m128_f32,
                            _mm_xor_ps(Float4_SignMask_12,
                                       max_intensity_cpi->m_normal.v)
                                .m128_f32,
                            25.0f) != nullptr)
                        m_owner->mFlags |= 0x10u;
                }
            }
        }
    }
}

// ea: 0x6F42C0
void bone_mass_info::set(
    int b1, int b2, float p1_radius, float p2_radius, float percent,
    const math::Position3& b1_adjust_p2_loc, int collision_sphere_count,
    float mass, float inertia_sphere_radius, float friction_k, int rb_id,
    int rb_parent_id, joint_type_e joint_type, float theta_min,
    float theta_max, const math::Dir3& rb_parent_axis_loc,
    const math::Dir3& rb_axis_loc, const math::Dir3& rb_parent_ref_loc,
    const math::Dir3& rb_ref_loc, float power)
{
    (void)power;
    m_b1 = b1;
    m_p1_radius = p1_radius;
    m_b2 = b2;
    m_p2_radius = p2_radius;
    m_percent = percent;
    m_b1_adjust_p2_loc.v = b1_adjust_p2_loc.v;
    m_collision_sphere_count = collision_sphere_count;
    m_rb_parent_id = rb_parent_id;
    m_mass = mass;
    m_inertia_sphere_radius = inertia_sphere_radius;
    m_rb_id = rb_id;
    m_friction_k = friction_k;
    m_joint_type = joint_type;
    m_theta_min = theta_min;
    m_theta_max = theta_max;
    m_rb_parent_axis_loc.v = rb_parent_axis_loc.v;
    m_rb_axis_loc.v = rb_axis_loc.v;
    m_rb_parent_ref_loc.v = rb_parent_ref_loc.v;
    m_rb_ref_loc.v = rb_ref_loc.v;
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

// ea: 0x709B10
void prop_phys_collision::collide_terrain(rb_extra_info* rb_inf)
{
    if ((rb_inf->m_flags.mMask & 1) == 0)
        return;
    if (rb_inf->m_ent == nullptr
        && _tlAssert("c:\\cod\\code\\game\\RBCollision.cpp", 387, "ent",
                     defaultFileName))
        __debugbreak();
    const math::Mat43* m_cg_mesh_mat = rb_inf->m_cg_mesh_mat;
    rigid_body* rb = rb_inf->m_rb;
    rb_vehicle* m_rb_vehicle = rb_inf->m_rb_vehicle;
    bool vehicle =
        m_rb_vehicle != nullptr && (m_rb_vehicle->m_flags.mMask & 0x80) != 0;

    phys_gjk_geom_list* gjk_geom_list =
        (phys_gjk_geom_list*)rb_inf->m_gjk_geom_list;
    math::Dir3 aabb_mn = gjk_geom_list->m_aabb_mn;
    math::Dir3 aabb_mx = gjk_geom_list->m_aabb_mx;
    add_active_eps_aabb(&aabb_mn, &aabb_mx);
    environment_rigid_body* erb = phys_sys::get_environment_rigid_body();
    physics_colgeom_visitor* local_primitive_list =
        generate_local_primitive_list(aabb_mn, aabb_mx, 577);
    int m_object_list_count = local_primitive_list->m_object_list_count;
    for (int obj_i = 0; obj_i < m_object_list_count; ++obj_i)
    {
        CGBank* bank =
            local_primitive_list->m_object_list[obj_i].m_bank;
        unsigned int index =
            (unsigned short)local_primitive_list->m_object_list[obj_i].m_index;
        int type = bank->get_type(index);
        if (index >= (unsigned int)bank->objects.m_count)
        {
            AeAssert::gCurrentAuthor = AeAssert::JSV;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\cgbank.h";
            AeAssert::gCurrentLine = 233;
            AeAssert::gCurrentExpr = "index < size()";
            if (!AeAssert::IsIgnored() && AeAssert::Assert(defaultFileName))
                __debugbreak();
            if (index >= (unsigned int)bank->objects.m_count
                && _tlAssert(
                       "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h", 89,
                       "index >= 0 && index < size()", "invalid index"))
                __debugbreak();
        }
        cdl_object_t* obj =
            &((cdl_object_t*)bank->objects.m_elements)[index];
        math::Dir3 obj_mn, obj_mx;
        obj_mn.v = _mm_sub_ps(
            _mm_setr_ps(obj->center[0], obj->center[1], obj->center[2], 0.0f),
            _mm_setr_ps(obj->box_radius[0], obj->box_radius[1],
                        obj->box_radius[2], 0.0f));
        obj_mx.v = _mm_add_ps(
            _mm_setr_ps(obj->center[0], obj->center[1], obj->center[2], 0.0f),
            _mm_setr_ps(obj->box_radius[0], obj->box_radius[1],
                        obj->box_radius[2], 0.0f));
        // CONTENTS marker 0x10200 ("D3DXX" header-string address in the
        // original build): skipped for non-vehicle rb_inf.
        if (obj->cflags == 0x10200 && !vehicle)
            continue;
        phys_gjk_geom_cod_base* gjk_geom =
            g_gjk_geom_database->get_gjk_geom(obj, (int)index, bank);
        if (gjk_geom == nullptr)
            continue;

        for (phys_gjk_geom_cod_base* i =
                 (phys_gjk_geom_cod_base*)
                     gjk_geom_list->m_first_geom;
             i != nullptr; i = i->m_next_geom)
        {
            if (m_rb_vehicle != nullptr
                && (m_rb_vehicle->m_flags.mMask & 0x100) != 0)
                break;
            if (!are_potentially_colliding(&i->m_aabb_mn, &i->m_aabb_mx,
                                           &obj_mn, &obj_mx))
                continue;
            float fric_coef = rb->m_fric_coef;
            if (m_rb_vehicle != nullptr)
            {
                if ((rb->m_flags & 0x50) == 0
                    && _tlAssert(
                           "c:\\cod\\code\\tl\\physics\\include\\rigid_body.h",
                           109, "debug_flag_is_in_collision()",
                           defaultFileName))
                    __debugbreak();
                if (rb->m_col_mat.z.v.m128_f32[2] > 0.2f)
                    fric_coef = 0.0f;
            }
            DObj* mDObj = rb_inf->m_ent->mDObj;
            void* mValue = mDObj->mPhysDataValue;
            TPakId mPakId = (TPakId)mDObj->mPhysDataPakId;
            float M_BOUNCE_COEF = 0.25f;
            ValidatePakId(mPakId);
            if (mValue != nullptr)
            {
                ValidatePakId(mPakId);
                M_BOUNCE_COEF = ((PhysData*)mValue)->mBounce;
            }
            if (rb_inf->can_have_event())
                create_prop_collide_callback(rb_inf, obj->sflags, rb, erb);
            phys_collide_data* v26 = g_list_phys_collide_data;
            if ((erb->m_flags & 0x50) == 0
                && _tlAssert(
                       "c:\\cod\\code\\tl\\physics\\include\\rigid_body.h",
                       109, "debug_flag_is_in_collision()", defaultFileName))
                __debugbreak();
            if ((erb->m_flags & 0x50) == 0
                && _tlAssert(
                       "c:\\cod\\code\\tl\\physics\\include\\rigid_body.h",
                       109, "debug_flag_is_in_collision()", defaultFileName))
                __debugbreak();
            int m_priority = rb_inf->m_priority;
            unsigned int id1 = (unsigned int)i->m_geom_id;
            unsigned int id2 = (unsigned int)gjk_geom->m_geom_id;
            v26->gjk_cg1 = (const phys_gjk_geom*)i;
            v26->gjk_cg2 = (const phys_gjk_geom*)gjk_geom;
            v26->cg1_to_world_xform = m_cg_mesh_mat;
            v26->cg2_to_world_xform = &erb->m_col_mat;
            v26->cg1_to_rb1_xform = &rb_inf->m_transform;
            v26->rb2_to_world_xform = &erb->m_col_mat;
            v26->rb1 = rb;
            v26->rb2 = erb;
            v26->gjk_ci = nullptr;
            v26->pcd_callback = nullptr;
            v26->gjk_info = g_gjk_info;
            v26->cman_process = g_cman_process;
            v26->id1 = id1;
            v26->id2 = id2;
            v26->fric_coef = fric_coef;
            v26->bounce_coef = M_BOUNCE_COEF;
            v26->no_overflow_error = true;
            v26->solver_priority = m_priority;
            v26->gjk_ci =
                g_phys_gjk_cache_system.get_gjk_cache_info(id1, id2, true);
            phys_collide_do_gjk_collide_and_contact_manifold(v26);
        }

        if (m_rb_vehicle != nullptr)
        {
            vehicle_collision_info* m_vci =
                (vehicle_collision_info*)m_rb_vehicle->m_vci;
            if (m_vci == nullptr
                && _tlAssert("c:\\cod\\code\\game\\RBCollision.cpp", 470,
                             "vci", defaultFileName))
                __debugbreak();
            int wheel_count = m_vci->m_list_wheel_collision_info_count;
            for (int wheel_i = 0; wheel_i < wheel_count; ++wheel_i)
            {
                wheel_collision_info* wci =
                    &m_vci->m_list_wheel_collision_info[wheel_i];
                int overlap = _mm_movemask_ps(
                    _mm_cmplt_ps(
                        _mm_max_ps(
                            _mm_sub_ps(obj_mn.v, wci->m_aabb_mx.v),
                            _mm_sub_ps(wci->m_aabb_mn.v, obj_mx.v)),
                        Float4_Zero_12));
                if ((overlap & 7) != 7)
                    continue;
                bool hit = false;
                if (type == 1)  // CDL_BRUSH
                {
                    unsigned int brush_index =
                        index - (unsigned int)bank->nboxes;
                    if (brush_index >= (unsigned int)bank->brushes.m_count
                        && _tlAssert(
                               "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h",
                               91, "index >= 0 && index < size()",
                               "invalid index"))
                        __debugbreak();
                    cdl_brush_rec* brush_rec =
                        &((cdl_brush_rec*)bank->brushes.m_elements)
                              [brush_index];
                    unsigned int first_side = brush_rec->first_side;
                    if (first_side
                            >= (unsigned int)bank->brush_sides.m_count
                        && _tlAssert(
                               "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h",
                               91, "index >= 0 && index < size()",
                               "invalid index"))
                        __debugbreak();
                    hit = collide_brush_segment(
                        reinterpret_cast<const math::Position3&>(wci->m_p0),
                        reinterpret_cast<const math::Position3&>(wci->m_p1),
                        reinterpret_cast<const math::Position3&>(obj_mn),
                        reinterpret_cast<const math::Position3&>(obj_mx),
                        (const cdlPlane*)bank->brush_sides.m_elements
                            + first_side,
                        brush_rec->nsides, wci->m_t,
                        reinterpret_cast<math::Position3*>(&wci->m_normal));
                }
                else if (type == 2)  // CDL_PATCH
                {
                    unsigned int patch_index =
                        index - (unsigned int)bank->nbrushes
                        - (unsigned int)bank->nboxes;
                    if (patch_index >= (unsigned int)bank->patches.m_count
                        && _tlAssert(
                               "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h",
                               91, "index >= 0 && index < size()",
                               "invalid index"))
                        __debugbreak();
                    cdl_patch_rec* patch_rec =
                        &((cdl_patch_rec*)bank->patches.m_elements)
                              [patch_index];
                    if (patch_index
                            >= (unsigned int)bank->gjk_patches.m_count
                        && _tlAssert(
                               "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h",
                               91, "index >= 0 && index < size()",
                               "invalid index"))
                        __debugbreak();
                    unsigned int num_inds = patch_rec->num_inds;
                    const unsigned char* index_list =
                        (const unsigned char*)bank->patch_inds.m_elements
                        + patch_rec->first_index;
                    math::Dir3* vert_list =
                        ((phys_gjk_geom_vert_list*)gjk_geom)->m_vert_list;
                    hit = collide_segment(
                        *obj, vert_list, index_list, 0, (int)num_inds,
                        reinterpret_cast<const math::Position3&>(wci->m_p0),
                        reinterpret_cast<const math::Position3&>(wci->m_p1),
                        wci->m_t,
                        reinterpret_cast<math::Position3&>(wci->m_normal),
                        nullptr);
                }
                else  // CDL_BOX
                {
                    hit = collide_box_segment(
                        reinterpret_cast<const math::Position3&>(wci->m_p0),
                        reinterpret_cast<const math::Position3&>(wci->m_p1),
                        reinterpret_cast<const math::Position3&>(obj_mn),
                        reinterpret_cast<const math::Position3&>(obj_mx),
                        wci->m_t,
                        reinterpret_cast<math::Position3*>(&wci->m_normal));
                }
                if (hit)
                {
                    wci->m_did_hit = true;
                    wci->m_surface_flags = obj->sflags;
                    wci->m_hit_rb = erb;
                }
            }
        }
    }
}

// ea: 0x70A330
void prop_phys_collision::collide_entities(rb_extra_info* rb_inf)
{
    Entity* m_ent = rb_inf->m_ent;
    if (m_ent == nullptr
        && _tlAssert("c:\\cod\\code\\game\\RBCollision.cpp", 676, "ent1",
                     defaultFileName))
        __debugbreak();
    rigid_body* m_rb = rb_inf->m_rb;
    if (rb_inf->m_gjk_geom_list == nullptr
        && _tlAssert(
               "c:\\cod\\code\\game\\RBCollision.cpp", 678,
               "rb_inf->m_gjk_geom_list", defaultFileName))
        __debugbreak();
    rb_vehicle* m_rb_vehicle = rb_inf->m_rb_vehicle;
    const math::Mat43* m_cg_mesh_mat = rb_inf->m_cg_mesh_mat;
    bool vehicle =
        m_rb_vehicle != nullptr && (m_rb_vehicle->m_flags.mMask & 0x80) != 0;
    phys_gjk_geom_list* gjk_geom_list =
        (phys_gjk_geom_list*)rb_inf->m_gjk_geom_list;
    math::Dir3 aabb_mn, aabb_mx;
    // get_active_eps_aabb<phys_gjk_geom_list> (physics.o inline 0x718220)
    aabb_mn.v = gjk_geom_list->m_aabb_mn.v;
    aabb_mx.v = gjk_geom_list->m_aabb_mx.v;
    add_active_eps_aabb(&aabb_mn, &aabb_mx);
    g_phys_touch_entity_data->mins.v = aabb_mn.v;
    g_phys_touch_entity_data->maxs.v = aabb_mx.v;
    g_phys_touch_entity_data->num = CM_AreaEntities(
        g_phys_touch_entity_data->mins, g_phys_touch_entity_data->maxs,
        g_phys_touch_entity_data->touch, 128, 42009169);
    int num = g_phys_touch_entity_data->num;
    if (num <= 0)
        return;
    for (int ent_i = 0; ent_i < num; ++ent_i)
    {
        unsigned int handle =
            g_phys_touch_entity_data->touch[ent_i].mHandle.mVal;
        unsigned int idx = handle & 0xFFF;
        if (idx >= 0x540
            || handle >> 12 != EntityHandleDb::sInst.mElements[idx].mKey)
            continue;
        Entity* mObject = EntityHandleDb::sInst.mElements[idx].mObject;
        if (mObject == nullptr || m_ent == mObject)
            continue;
        int flags = mObject->flags;
        if (((flags & 0x2000000) == 0 && mObject->r.bmodel == nullptr
             && mObject->actor == nullptr && mObject->client == nullptr)
            || (0x400000 & flags) != 0)
            continue;
        DCGSet* bmodel = (DCGSet*)mObject->r.bmodel;
        if (bmodel != nullptr && bmodel->objects_m_count > 0x14)
        {
            AeAssert::gCurrentAuthor = AeAssert::JRS;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\RBCollision.cpp";
            AeAssert::gCurrentLine = 702;
            AeAssert::gCurrentExpr = nullptr;
            if (!AeAssert::IsIgnored())
            {
                const char* mStr;
                void* model_value = *(void**)((char*)mObject + 0x270);
                ValidatePakId(*(int*)((char*)mObject + 0x274));
                if (model_value != nullptr)
                    mStr = *(const char**)((char*)model_value + 0x48);
                else
                    mStr = "unknown";
                if (AeAssert::Warning(
                        "Model %s has too many brushes (%d). Will not collide.",
                        mStr, bmodel->objects_m_count))
                    __debugbreak();
            }
            continue;
        }

        void* actor = mObject->actor;
        bool is_actor = (flags & 0x2000000) != 0;
        if (actor != nullptr || mObject->client != nullptr
            || (flags & 0x2000000) != 0)
        {
            // Actor / client path: GJK collide + direct impulse damage.
            if (m_ent->scr_vehicle == nullptr)
                goto LABEL_94;
            {
                unsigned int owner_handle =
                    *(unsigned int*)((char*)mObject + 0x1B0);
                unsigned int owner_idx = owner_handle & 0xFFF;
                Entity* owner = nullptr;
                if (owner_idx < 0x540
                    && owner_handle >> 12
                           == EntityHandleDb::sInst.mElements[owner_idx].mKey)
                    owner =
                        EntityHandleDb::sInst.mElements[owner_idx].mObject;
                if (owner == m_ent)
                    continue;
            }
        LABEL_94:
            if (mObject->tagInfo != nullptr || flags < 0
                || ((actor != nullptr || mObject->client != nullptr)
                    && rb_inf->m_priority == 0))
                continue;
            {
                phys_gjk_geom_cod_base* actor_gjk_geom =
                    g_gjk_geom_database->get_actor_gjk_geom(mObject, nullptr);
                __m128 v45 = _mm_mul_ps(m_rb->m_t_vel.v, m_rb->m_t_vel.v);
                float v79 =
                    v45.m128_f32[0]
                    + (_mm_shuffle_ps(v45, v45, 85).m128_f32[0]
                       + _mm_shuffle_ps(v45, v45, 170).m128_f32[0]);
                bool process_actor = true;
                if (min_speed2 <= v79 || is_actor)
                {
                    for (phys_gjk_geom_cod_base* m_first_geom =
                             (phys_gjk_geom_cod_base*)
                                 gjk_geom_list->m_first_geom;
                         m_first_geom != nullptr;
                         m_first_geom = m_first_geom->m_next_geom)
                    {
                        if (!process_actor)
                            break;
                        environment_rigid_body* erb =
                            phys_sys::get_environment_rigid_body();
                        phys_collide_data* v49 = g_list_phys_collide_data;
                        phys_gjk_info* v50 = g_gjk_info;
                        unsigned int id2 =
                            (unsigned int)*(void**)((char*)actor_gjk_geom
                                                    + 0x30);
                        unsigned int id1 =
                            (unsigned int)m_first_geom->m_geom_id;
                        v49->gjk_cg1 = (const phys_gjk_geom*)m_first_geom;
                        v49->gjk_cg2 =
                            (const phys_gjk_geom*)actor_gjk_geom;
                        v49->cg2_to_world_xform = &erb->m_col_mat;
                        v49->cg1_to_world_xform = m_cg_mesh_mat;
                        v49->id1 = id1;
                        v49->id2 = id2;
                        v49->gjk_ci = nullptr;
                        v49->gjk_info = v50;
                        if (phys_collide_do_gjk_collide_stub(
                                v50, v49, 3.4000001f))
                        {
                            phys_gjk_collision_info* cg1_cinfo_loc =
                                v49->cg1_cinfo_loc;
                            math::Position3 p1_world;
                            p1_world.v = _mm_add_ps(
                                _mm_add_ps(
                                    _mm_mul_ps(
                                        _mm_shuffle_ps(
                                            cg1_cinfo_loc->m_p1.v,
                                            cg1_cinfo_loc->m_p1.v, 0),
                                        m_cg_mesh_mat->x.v),
                                    _mm_mul_ps(
                                        _mm_shuffle_ps(
                                            cg1_cinfo_loc->m_p1.v,
                                            cg1_cinfo_loc->m_p1.v, 0x55),
                                        m_cg_mesh_mat->y.v)),
                                _mm_add_ps(
                                    _mm_mul_ps(
                                        _mm_shuffle_ps(
                                            cg1_cinfo_loc->m_p1.v,
                                            cg1_cinfo_loc->m_p1.v, 0xAA),
                                        m_cg_mesh_mat->z.v),
                                    m_cg_mesh_mat->w.v));
                            math::Dir3 n_world;
                            n_world.v = _mm_add_ps(
                                _mm_add_ps(
                                    _mm_mul_ps(
                                        _mm_shuffle_ps(
                                            cg1_cinfo_loc->m_n.v,
                                            cg1_cinfo_loc->m_n.v, 0),
                                        m_cg_mesh_mat->x.v),
                                    _mm_mul_ps(
                                        _mm_shuffle_ps(
                                            cg1_cinfo_loc->m_n.v,
                                            cg1_cinfo_loc->m_n.v, 0x55),
                                        m_cg_mesh_mat->y.v)),
                                _mm_mul_ps(
                                    _mm_shuffle_ps(
                                        cg1_cinfo_loc->m_n.v,
                                        cg1_cinfo_loc->m_n.v, 0xAA),
                                    m_cg_mesh_mat->z.v));
                            __m128 neg = _mm_xor_ps(Float4_SignMask_12,
                                                    n_world.v);
                            __m128 v63 = _mm_mul_ps(neg, m_rb->m_t_vel.v);
                            float rel =
                                v63.m128_f32[0]
                                + (_mm_shuffle_ps(v63, v63, 85).m128_f32[0]
                                   + _mm_shuffle_ps(v63, v63, 170)
                                         .m128_f32[0]);
                            if (rel <= 300.0f)
                                rel = 0.0f;
                            int damage = (int)(rel * 0.23999999f);
                            if (damage > 0)
                            {
                                G_Damage(
                                    mObject, m_ent, m_ent,
                                    &neg.m128_f32[0],
                                    &p1_world.v.m128_f32[0], damage, 0, 32,
                                    (EHitLocation)0, -1);
                            }
                            process_actor = false;
                        }
                    }
                }
            }
        }
        else
        {
            // Static mesh path.
            mObject->CalcRotTranMat43();
            DCGSet* v14 = (DCGSet*)mObject->r.bmodel;
            int obj_count = v14->objects_m_count;
            if (obj_count <= 0)
                goto LABEL_42;
            {
                int v15 = 0;
                while (1)
                {
                    if (v15 >= obj_count
                        && _tlAssert(
                               "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h",
                               91, "index >= 0 && index < size()",
                               "invalid index"))
                        __debugbreak();
                    if (((cdl_object_t*)v14->objects_m_elements)[v15]
                            .cflags
                            == 0x10200
                        && !vehicle)
                        goto NEXT_ENTITY;
                    ++v15;
                    if (v15 >= obj_count)
                        goto LABEL_42;
                }
            }
        LABEL_42:
            {
                math::Position3 center =
                    *(math::Position3*)((char*)v14 + 0x50);
                float radius = *(float*)((char*)v14 + 0x60);
                __m128 v24 = _mm_add_ps(
                    _mm_add_ps(
                        _mm_mul_ps(_mm_shuffle_ps(center.v, center.v, 0),
                                   mObject->r.currentMat.x.v),
                        _mm_mul_ps(_mm_shuffle_ps(center.v, center.v, 0x55),
                                   mObject->r.currentMat.y.v)),
                    _mm_add_ps(
                        _mm_mul_ps(_mm_shuffle_ps(center.v, center.v, 0xAA),
                                   mObject->r.currentMat.z.v),
                        mObject->r.currentMat.w.v));
                __m128 rv = _mm_setr_ps(radius, radius, radius, 0.0f);
                math::Dir3 bmax;
                bmax.v = _mm_add_ps(v24, rv);
                math::Dir3 bmin;
                bmin.v = _mm_sub_ps(v24, rv);
                if (!are_potentially_colliding(&aabb_mn, &aabb_mx, &bmin,
                                               &bmax))
                    goto NEXT_ENTITY;
                phys_gjk_geom_list* ent_geom_list =
                    g_gjk_geom_database->get_gjk_geom(
                        mObject, &mObject->r.currentMat);
                if (ent_geom_list == nullptr)
                    goto NEXT_ENTITY;
                environment_rigid_body* erb =
                    phys_sys::get_environment_rigid_body();
                for (phys_gjk_geom_cod_base* rb =
                         (phys_gjk_geom_cod_base*)
                             gjk_geom_list->m_first_geom;
                     rb != nullptr; rb = rb->m_next_geom)
                {
                    for (phys_gjk_geom_cod_base* v25 =
                             (phys_gjk_geom_cod_base*)
                                 ent_geom_list->m_first_geom;
                         v25 != nullptr; v25 = v25->m_next_geom)
                    {
                        if ((mObject->flags & 0x400000) != 0)
                            break;
                        if (!are_potentially_colliding(
                                &rb->m_aabb_mn, &rb->m_aabb_mx,
                                &v25->m_aabb_mn, &v25->m_aabb_mx))
                            continue;
                        float fric_coef = m_rb->m_fric_coef;
                        if (m_rb_vehicle != nullptr
                            && m_rb->m_col_mat.z.v.m128_f32[2] > 0.2f)
                            fric_coef = 0.0f;
                        DObj* mDObj = rb_inf->m_ent->mDObj;
                        void* mValue = mDObj->mPhysDataValue;
                        TPakId mPakId = (TPakId)mDObj->mPhysDataPakId;
                        int bounce_bits = 0x3E800000;  // 0.25f
                        ValidatePakId(mPakId);
                        if (mValue != nullptr)
                        {
                            ValidatePakId(mPakId);
                            bounce_bits = *(int*)((char*)mValue + 8);
                        }
                        phys_collide_data* v32 = g_list_phys_collide_data;
                        if ((erb->m_flags & 0x50) == 0
                            && _tlAssert(
                                   "c:\\cod\\code\\tl\\physics\\include\\rigid_body.h",
                                   109, "debug_flag_is_in_collision()",
                                   defaultFileName))
                            __debugbreak();
                        int m_priority = rb_inf->m_priority;
                        collide_entities_callback pcd_callback;
                        pcd_callback.ent1 = m_ent;
                        pcd_callback.ent2 = mObject;
                        v32->gjk_cg1 = (const phys_gjk_geom*)rb;
                        v32->gjk_cg2 = (const phys_gjk_geom*)v25;
                        v32->cg1_to_world_xform = m_cg_mesh_mat;
                        v32->cg2_to_world_xform = &mObject->r.currentMat;
                        v32->cg1_to_rb1_xform = &rb_inf->m_transform;
                        v32->rb2_to_world_xform = &erb->m_col_mat;
                        v32->rb1 = m_rb;
                        v32->rb2 = erb;
                        v32->id1 = (unsigned int)rb->m_geom_id;
                        v32->id2 = (unsigned int)v25->m_geom_id;
                        v32->pcd_callback = &pcd_callback;
                        v32->fric_coef = fric_coef;
                        v32->bounce_coef = *(float*)&bounce_bits;
                        v32->gjk_ci = nullptr;
                        v32->gjk_info = g_gjk_info;
                        v32->cman_process = g_cman_process;
                        v32->no_overflow_error = true;
                        v32->solver_priority = m_priority;
                        list_phys_collide_data_add(v32);
                    }
                }
                if (m_rb_vehicle != nullptr)
                {
                    vehicle_collision_info* m_vci =
                        (vehicle_collision_info*)m_rb_vehicle->m_vci;
                    for (phys_gjk_geom_cod_base* i =
                             (phys_gjk_geom_cod_base*)
                                 ent_geom_list->m_first_geom;
                         i != nullptr; i = i->m_next_geom)
                    {
                        if ((mObject->flags & 0x400000) != 0)
                            break;
                        int wheel_count =
                            m_vci->m_list_wheel_collision_info_count;
                        for (int w = 0; w < wheel_count; ++w)
                        {
                            collide_segment(
                                &m_vci->m_list_wheel_collision_info[w],
                                m_ent, mObject, erb,
                                &mObject->r.currentMat, i);
                        }
                    }
                }
            }
        }
    NEXT_ENTITY:;
    }
}

// ea: 0x707180
void biped_system::setup_physics(Entity* owner)
{
    biped_phys_info* mBPInfo = owner->mBPInfo;
    if (mBPInfo == nullptr
        && _tlAssert("c:\\cod\\code\\game\\RBRagdoll.cpp", 1079,
                     "owner->get_bp_info()", defaultFileName))
        __debugbreak();
    owner->CalcRotTranMat43();
    float ox = owner->r.currentOrigin.v.m128_f32[0];
    float oy = owner->r.currentOrigin.v.m128_f32[1];
    float oz = owner->r.currentOrigin.v.m128_f32[2];
    if (IS_BAD_NUMBER(ox) || IS_BAD_NUMBER(oy) || IS_BAD_NUMBER(oz))
    {
        if (_tlAssert(
                "c:\\cod\\code\\game\\RBRagdoll.cpp", 1083,
                "!IS_BAD_NUMBER((owner->r.currentOrigin)[0]) && !IS_BAD_NUMBER((owner->r.currentOrigin)[1]) && !IS_BAD_NUMBER((owner->r.currentOrigin)[2])",
                "Invalid vector"))
            __debugbreak();
    }
    m_collision_callback.m_owner = owner;
    math::Mat43 skeleton_pose[10];
    phys_anim_bone_array::copy_skeleton(owner, skeleton_pose);
    bp_bone_array.copy_tween_start(owner);

    if ((S45_1_guard & 1) == 0)
    {
        S45_1_guard |= 1;
        tlFixedString name("Ragdoll");
        anim = (nalGeneric::nalGenericAnim*)cdGetAnim(CurPakId(), name);
    }
    nalGeneric::nalGenericAnim* Anim = anim;
    if (Anim != nullptr)
    {
        nalGeneric::nalGenericSkeleton* Skeleton = Anim->Skeleton;
        if (Skeleton != nullptr
            && Skeleton->__vftable
                   != (void*)nalGeneric::nalGenericSkeleton::vtbl_ptr)
            Skeleton = nullptr;
        if ((S45_1_guard & 2) == 0)
        {
            S45_1_guard |= 2;
            inst = Anim->CreateInstance(nullptr);
        }
        nalGeneric::nalGenericPose pose(Skeleton, 0);
        int HighLOD = owner->mDObj->GetHighLOD();
        inst->GetPose(0, pose, HighLOD);
        Skeleton->GetBoneMatrices(
            pose, (nalGeneric::nalMatrix4x4*)((char*)owner->mDObj->skel + 0x30),
            HighLOD);
    }
    else
    {
        AeAssert::gCurrentAuthor = AeAssert::JRS;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\RBRagdoll.cpp";
        AeAssert::gCurrentLine = 1109;
        AeAssert::gCurrentExpr = nullptr;
        if (!AeAssert::IsIgnored()
            && AeAssert::Warning(
                   "You need to add ragdoll.txtanim to your pakfile"))
            __debugbreak();
        DObjGetBasePose(owner->mDObj);
    }
    bp_bone_array.attach_physics_bones(owner);
    setup_bone_mass_info(owner);
    m_bp_info->update_vel_matrices();
    set_max_rb_index(10);

    // Per-bone rigid bodies. The decompile walks m_bone_mass_info[i].m_com by
    // 29 Position3 (= 464 = sizeof(bone_mass_info)) and reads the fields by
    // relative offset.
    for (int i = 0; i < 10; ++i)
    {
        bone_mass_info& bmi = m_bone_mass_info[i];
        rigid_body* v14 = add_rigid_body(bmi.m_rb_id);
        math::Dir3 t_vel, a_vel;
        m_bp_info->get_cur_vel(bmi.m_rb_id, bmi.m_com, &t_vel, &a_vel);
        math::Mat43 abs_mat = owner->CalcAbsMat(bmi.m_rb_bone);
        math::Mat43 identity;
        SetIdentity(identity);
        math::Position3 com_world;
        com_world.v = _mm_add_ps(
            _mm_add_ps(
                _mm_mul_ps(_mm_shuffle_ps(bmi.m_com.v, bmi.m_com.v, 0),
                           abs_mat.x.v),
                _mm_mul_ps(_mm_shuffle_ps(bmi.m_com.v, bmi.m_com.v, 0x55),
                           abs_mat.y.v)),
            _mm_add_ps(
                _mm_mul_ps(_mm_shuffle_ps(bmi.m_com.v, bmi.m_com.v, 0xAA),
                           abs_mat.z.v),
                abs_mat.w.v));
        (void)com_world;
        if (IS_BAD_NUMBER(bmi.m_com.v.m128_f32[0])
            || IS_BAD_NUMBER(bmi.m_com.v.m128_f32[1])
            || IS_BAD_NUMBER(bmi.m_com.v.m128_f32[2]))
        {
            if (_tlAssert(
                    "c:\\cod\\code\\game\\RBRagdoll.cpp", 1142,
                    "!IS_BAD_NUMBER((bmi.m_com)[0]) && !IS_BAD_NUMBER((bmi.m_com)[1]) && !IS_BAD_NUMBER((bmi.m_com)[2])",
                    "Invalid vector"))
                __debugbreak();
        }
        if (IS_BAD_NUMBER(t_vel.v.m128_f32[0])
            || IS_BAD_NUMBER(t_vel.v.m128_f32[1])
            || IS_BAD_NUMBER(t_vel.v.m128_f32[2]))
        {
            if (_tlAssert(
                    "c:\\cod\\code\\game\\RBRagdoll.cpp", 1143,
                    "!IS_BAD_NUMBER((t_vel)[0]) && !IS_BAD_NUMBER((t_vel)[1]) && !IS_BAD_NUMBER((t_vel)[2])",
                    "Invalid vector"))
                __debugbreak();
        }
        if (IS_BAD_NUMBER(a_vel.v.m128_f32[0])
            || IS_BAD_NUMBER(a_vel.v.m128_f32[1])
            || IS_BAD_NUMBER(a_vel.v.m128_f32[2]))
        {
            if (_tlAssert(
                    "c:\\cod\\code\\game\\RBRagdoll.cpp", 1144,
                    "!IS_BAD_NUMBER((a_vel)[0]) && !IS_BAD_NUMBER((a_vel)[1]) && !IS_BAD_NUMBER((a_vel)[2])",
                    "Invalid vector"))
                __debugbreak();
        }
        v14->set(bmi.m_mass,
                 *reinterpret_cast<const math::Dir3*>(&bmi.m_inertia),
                 identity, t_vel, a_vel,
                 bmi.m_friction_k, 0);
        v14->m_flags |= 2u;
        v14->set_gravity_dir(PHYSICS_GRAVITY_DIRECTION_3);
        math::Mat43 bone_mat_loc;
        phys_full_inv_multiply_mat(bone_mat_loc, identity, abs_mat);
        if (bmi.m_rb_id >= 10
            && _tlAssert("c:/cod/code/tl/physics/include\\phys_mem.h", 374,
                         "i >= 0 && i < size", defaultFileName))
            __debugbreak();
        ((client_bone_info*)((char*)this + 0x1D20))[bmi.m_rb_id].set(
            bone_mat_loc, owner, bmi.m_rb_bone, bmi.m_rb_id);
    }

    // Joints.
    for (int i = 0; i < 10; ++i)
    {
        bone_mass_info& bmi = m_bone_mass_info[i];
        if (bmi.m_rb_id == 0)
            continue;
        if (bmi.m_rb_parent_id < 0
            && _tlAssert("c:\\cod\\code\\game\\RBRagdoll.cpp", 1164,
                         "bmi.m_rb_parent_id >= 0", defaultFileName))
            __debugbreak();
        if (bmi.m_rb_id < 0
            && _tlAssert("c:\\cod\\code\\game\\RBRagdoll.cpp", 1165,
                         "bmi.m_rb_id >= 0", defaultFileName))
            __debugbreak();
        if (bmi.m_rb_parent_bone < 0
            && _tlAssert("c:\\cod\\code\\game\\RBRagdoll.cpp", 1166,
                         "bmi.m_rb_parent_bone >= NULL", defaultFileName))
            __debugbreak();
        if (bmi.m_rb_bone < 0
            && _tlAssert("c:\\cod\\code\\game\\RBRagdoll.cpp", 1167,
                         "bmi.m_rb_bone >= NULL", defaultFileName))
            __debugbreak();
        rigid_body_constraint_ragdoll* joint =
            add_joint(bmi.m_rb_parent_id, bmi.m_rb_id);
        if ((bmi.m_rb_parent_id < 0
             || bmi.m_rb_parent_id >= m_list_rigid_body.m_alloc_count)
            && _tlAssert(
                   "c:\\cod\\code\\tl\\physics\\include\\phys_array_base.inc",
                   108, "i >= 0 && i < m_alloc_count", defaultFileName))
            __debugbreak();
        rigid_body* v24 = m_list_rigid_body.m_slot_array[bmi.m_rb_parent_id];
        if ((bmi.m_rb_id < 0 || bmi.m_rb_id >= m_list_rigid_body.m_alloc_count)
            && _tlAssert(
                   "c:\\cod\\code\\tl\\physics\\include\\phys_array_base.inc",
                   108, "i >= 0 && i < m_alloc_count", defaultFileName))
            __debugbreak();
        rigid_body* rb_child = m_list_rigid_body.m_slot_array[bmi.m_rb_id];
        math::Mat43 parent_mat = owner->CalcAbsMat(bmi.m_rb_parent_bone);
        math::Mat43 child_mat = owner->CalcAbsMat(bmi.m_rb_bone);

        // World pivots from the bone mass pivots, then into the child rb frame.
        math::Position3 parent_pivot_world;
        parent_pivot_world.v = _mm_add_ps(
            _mm_add_ps(
                _mm_mul_ps(
                    _mm_shuffle_ps(bmi.m_rb_parent_pivot_loc.v,
                                   bmi.m_rb_parent_pivot_loc.v, 0),
                    parent_mat.x.v),
                _mm_mul_ps(
                    _mm_shuffle_ps(bmi.m_rb_parent_pivot_loc.v,
                                   bmi.m_rb_parent_pivot_loc.v, 0x55),
                    parent_mat.y.v)),
            _mm_add_ps(
                _mm_mul_ps(
                    _mm_shuffle_ps(bmi.m_rb_parent_pivot_loc.v,
                                   bmi.m_rb_parent_pivot_loc.v, 0xAA),
                    parent_mat.z.v),
                parent_mat.w.v));
        math::Position3 child_pivot_world;
        child_pivot_world.v = _mm_add_ps(
            _mm_add_ps(
                _mm_mul_ps(
                    _mm_shuffle_ps(bmi.m_rb_pivot_loc.v,
                                   bmi.m_rb_pivot_loc.v, 0),
                    child_mat.x.v),
                _mm_mul_ps(
                    _mm_shuffle_ps(bmi.m_rb_pivot_loc.v,
                                   bmi.m_rb_pivot_loc.v, 0x55),
                    child_mat.y.v)),
            _mm_add_ps(
                _mm_mul_ps(
                    _mm_shuffle_ps(bmi.m_rb_pivot_loc.v,
                                   bmi.m_rb_pivot_loc.v, 0xAA),
                    child_mat.z.v),
                child_mat.w.v));
        // world -> child rb local (transpose of rb mat)
        __m128 wp = _mm_sub_ps(parent_pivot_world.v, rb_child->m_mat.w.v);
        math::Dir3 local_parent_pivot;
        local_parent_pivot.v = _mm_add_ps(
            _mm_add_ps(
                _mm_mul_ps(_mm_shuffle_ps(wp, wp, 0), rb_child->m_mat.x.v),
                _mm_mul_ps(_mm_shuffle_ps(wp, wp, 0x55),
                           rb_child->m_mat.y.v)),
            _mm_mul_ps(_mm_shuffle_ps(wp, wp, 0xAA), rb_child->m_mat.z.v));
        __m128 wp2 = _mm_sub_ps(child_pivot_world.v, rb_child->m_mat.w.v);
        math::Dir3 local_child_pivot;
        local_child_pivot.v = _mm_add_ps(
            _mm_add_ps(
                _mm_mul_ps(_mm_shuffle_ps(wp2, wp2, 0), rb_child->m_mat.x.v),
                _mm_mul_ps(_mm_shuffle_ps(wp2, wp2, 0x55),
                           rb_child->m_mat.y.v)),
            _mm_mul_ps(_mm_shuffle_ps(wp2, wp2, 0xAA), rb_child->m_mat.z.v));
        joint->set(local_parent_pivot, local_child_pivot);
        if (bmi.m_damp_k > 0.000001f)
            joint->set_damp_k(bmi.m_damp_k);
        if (bmi.m_joint_type == 1)
        {
            math::Dir3 axis_b;
            axis_b.v = _mm_add_ps(
                _mm_add_ps(
                    _mm_mul_ps(
                        _mm_shuffle_ps(bmi.m_rb_axis_loc.v,
                                       bmi.m_rb_axis_loc.v, 0),
                        child_mat.x.v),
                    _mm_mul_ps(
                        _mm_shuffle_ps(bmi.m_rb_axis_loc.v,
                                       bmi.m_rb_axis_loc.v, 0x55),
                        child_mat.y.v)),
                _mm_mul_ps(
                    _mm_shuffle_ps(bmi.m_rb_axis_loc.v,
                                   bmi.m_rb_axis_loc.v, 0xAA),
                    child_mat.z.v));
            math::Dir3 axis_p;
            axis_p.v = _mm_add_ps(
                _mm_add_ps(
                    _mm_mul_ps(
                        _mm_shuffle_ps(bmi.m_rb_parent_axis_loc.v,
                                       bmi.m_rb_parent_axis_loc.v, 0),
                        parent_mat.x.v),
                    _mm_mul_ps(
                        _mm_shuffle_ps(bmi.m_rb_parent_axis_loc.v,
                                       bmi.m_rb_parent_axis_loc.v, 0x55),
                        parent_mat.y.v)),
                _mm_mul_ps(
                    _mm_shuffle_ps(bmi.m_rb_parent_axis_loc.v,
                                   bmi.m_rb_parent_axis_loc.v, 0xAA),
                    parent_mat.z.v));
            math::Dir3 ref_b;
            ref_b.v = _mm_add_ps(
                _mm_add_ps(
                    _mm_mul_ps(
                        _mm_shuffle_ps(bmi.m_rb_ref_loc.v,
                                       bmi.m_rb_ref_loc.v, 0),
                        child_mat.x.v),
                    _mm_mul_ps(
                        _mm_shuffle_ps(bmi.m_rb_ref_loc.v,
                                       bmi.m_rb_ref_loc.v, 0x55),
                        child_mat.y.v)),
                _mm_mul_ps(
                    _mm_shuffle_ps(bmi.m_rb_ref_loc.v,
                                   bmi.m_rb_ref_loc.v, 0xAA),
                    child_mat.z.v));
            math::Dir3 ref_p;
            ref_p.v = _mm_add_ps(
                _mm_add_ps(
                    _mm_mul_ps(
                        _mm_shuffle_ps(bmi.m_rb_parent_ref_loc.v,
                                       bmi.m_rb_parent_ref_loc.v, 0),
                        parent_mat.x.v),
                    _mm_mul_ps(
                        _mm_shuffle_ps(bmi.m_rb_parent_ref_loc.v,
                                       bmi.m_rb_parent_ref_loc.v, 0x55),
                        parent_mat.y.v)),
                _mm_mul_ps(
                    _mm_shuffle_ps(bmi.m_rb_parent_ref_loc.v,
                                   bmi.m_rb_parent_ref_loc.v, 0xAA),
                    parent_mat.z.v));
            joint->set_hinge(axis_p, ref_p, ref_b, axis_b, bmi.m_theta_min,
                             bmi.m_theta_max);
        }
        else if (bmi.m_joint_type == 2)
        {
            math::Dir3 axis_b;
            axis_b.v = _mm_add_ps(
                _mm_add_ps(
                    _mm_mul_ps(
                        _mm_shuffle_ps(bmi.m_rb_axis_loc.v,
                                       bmi.m_rb_axis_loc.v, 0),
                        child_mat.x.v),
                    _mm_mul_ps(
                        _mm_shuffle_ps(bmi.m_rb_axis_loc.v,
                                       bmi.m_rb_axis_loc.v, 0x55),
                        child_mat.y.v)),
                _mm_mul_ps(
                    _mm_shuffle_ps(bmi.m_rb_axis_loc.v,
                                   bmi.m_rb_axis_loc.v, 0xAA),
                    child_mat.z.v));
            math::Dir3 axis_p;
            axis_p.v = _mm_add_ps(
                _mm_add_ps(
                    _mm_mul_ps(
                        _mm_shuffle_ps(bmi.m_rb_parent_axis_loc.v,
                                       bmi.m_rb_parent_axis_loc.v, 0),
                        parent_mat.x.v),
                    _mm_mul_ps(
                        _mm_shuffle_ps(bmi.m_rb_parent_axis_loc.v,
                                       bmi.m_rb_parent_axis_loc.v, 0x55),
                        parent_mat.y.v)),
                _mm_mul_ps(
                    _mm_shuffle_ps(bmi.m_rb_parent_axis_loc.v,
                                   bmi.m_rb_parent_axis_loc.v, 0xAA),
                    parent_mat.z.v));
            math::Dir3 ref_b;
            ref_b.v = _mm_add_ps(
                _mm_add_ps(
                    _mm_mul_ps(
                        _mm_shuffle_ps(bmi.m_rb_ref_loc.v,
                                       bmi.m_rb_ref_loc.v, 0),
                        child_mat.x.v),
                    _mm_mul_ps(
                        _mm_shuffle_ps(bmi.m_rb_ref_loc.v,
                                       bmi.m_rb_ref_loc.v, 0x55),
                        child_mat.y.v)),
                _mm_mul_ps(
                    _mm_shuffle_ps(bmi.m_rb_ref_loc.v,
                                   bmi.m_rb_ref_loc.v, 0xAA),
                    child_mat.z.v));
            math::Dir3 ref_p;
            ref_p.v = _mm_add_ps(
                _mm_add_ps(
                    _mm_mul_ps(
                        _mm_shuffle_ps(bmi.m_rb_parent_ref_loc.v,
                                       bmi.m_rb_parent_ref_loc.v, 0),
                        parent_mat.x.v),
                    _mm_mul_ps(
                        _mm_shuffle_ps(bmi.m_rb_parent_ref_loc.v,
                                       bmi.m_rb_parent_ref_loc.v, 0x55),
                        parent_mat.y.v)),
                _mm_mul_ps(
                    _mm_shuffle_ps(bmi.m_rb_parent_ref_loc.v,
                                   bmi.m_rb_parent_ref_loc.v, 0xAA),
                    parent_mat.z.v));
            joint->set_swivel(axis_p, ref_p, ref_b, axis_b, bmi.m_theta_min,
                              bmi.m_theta_max);
        }
        for (int j = 0; j < bmi.m_joint_limit_count; ++j)
        {
            math::Dir3 limit_axis_world;
            limit_axis_world.v = _mm_add_ps(
                _mm_add_ps(
                    _mm_mul_ps(
                        _mm_shuffle_ps(bmi.m_joint_limit_axis[j].v,
                                       bmi.m_joint_limit_axis[j].v, 0),
                        child_mat.x.v),
                    _mm_mul_ps(
                        _mm_shuffle_ps(bmi.m_joint_limit_axis[j].v,
                                       bmi.m_joint_limit_axis[j].v, 0x55),
                        child_mat.y.v)),
                _mm_mul_ps(
                    _mm_shuffle_ps(bmi.m_joint_limit_axis[j].v,
                                   bmi.m_joint_limit_axis[j].v, 0xAA),
                    child_mat.z.v));
            joint->add_joint_limit(limit_axis_world,
                                   bmi.m_joint_limit_angle[j]);
        }
    }

    // Collision geometry per bone.
    for (int i = 0; i < 10; ++i)
    {
        bone_mass_info& bmi = m_bone_mass_info[i];
        int rb_id = bmi.m_rb_id;
        if ((rb_id < 0 || rb_id >= m_list_rigid_body.m_alloc_count)
            && _tlAssert(
                   "c:\\cod\\code\\tl\\physics\\include\\phys_array_base.inc",
                   108, "i >= 0 && i < m_alloc_count", defaultFileName))
            __debugbreak();
        rigid_body* rb = m_list_rigid_body.m_slot_array[rb_id];
        rigid_body_sphere_list* colgeom =
            m_collision_callback.get_colgeom(rb_id);
        if (colgeom == nullptr)
        {
            colgeom = m_collision_callback.add_colgeom(rb_id);
            if (colgeom == nullptr)
            {
                tlFatal("phys memory pool add overflow.");
                mem_break();
                continue;
            }
        }
        colgeom->set(rb);
        // capsule (bone-mass capsule in rb frame)
        colgeom->m_capsule.set(bmi.m_capsule_p1, bmi.m_capsule_p2,
                               bmi.m_capsule_radius);
        colgeom->set(rb);
        colgeom->m_rb_id = bmi.m_rb_id;
        colgeom->m_gjk_geom.init(colgeom);
        colgeom->m_tunnel_test_radius = 0.0f;
        colgeom->m_tunnel_test_active_counter = 0;
        colgeom->m_tunnel_test_last_pos.v = _mm_setzero_ps();
        // spheres from the capsule endpoints (max 2)
        rb_collision_sphere* spheres = colgeom->m_slot_array;
        int n = bmi.m_collision_sphere_count;
        if (n <= 1)
            n = 1;
        else
            n = 2;
        colgeom->m_alloc_count = n;
        spheres[0].m_center_loc.v = bmi.m_capsule_p1.v;
        spheres[0].m_radius = bmi.m_p1_radius;
        if (n == 2)
        {
            spheres[1].m_center_loc.v = bmi.m_capsule_p2.v;
            spheres[1].m_radius = bmi.m_p2_radius;
        }
    }

    // Capsule pairs (bone pairings from the binary).
    {
        rigid_body_sphere_list* cg9 = m_collision_callback.get_colgeom(9);
        rigid_body_sphere_list* cg7 = m_collision_callback.get_colgeom(7);
        if (m_collision_callback.m_rb_cp_count < 7)
        {
            rb_capsule_pair* cp =
                m_collision_callback.m_rb_cp_alloc_list
                    [m_collision_callback.m_rb_cp_count++];
            if (cp != nullptr)
            {
                cp->m_b2_cg = cg9;
                cp->m_b1_cg = cg7;
            }
        }
        else
        {
            tlFatal("phys memory pool add overflow.");
        }
    }
    {
        rigid_body_sphere_list* cg8 = m_collision_callback.get_colgeom(8);
        rigid_body_sphere_list* cg7 = m_collision_callback.get_colgeom(7);
        if (m_collision_callback.m_rb_cp_count < 7)
        {
            rb_capsule_pair* cp =
                m_collision_callback.m_rb_cp_alloc_list
                    [m_collision_callback.m_rb_cp_count++];
            if (cp != nullptr)
            {
                cp->m_b2_cg = cg8;
                cp->m_b1_cg = cg7;
            }
        }
        else
        {
            tlFatal("phys memory pool add overflow.");
        }
    }
    {
        rigid_body_sphere_list* cg6 = m_collision_callback.get_colgeom(6);
        rigid_body_sphere_list* cg9 = m_collision_callback.get_colgeom(9);
        if (m_collision_callback.m_rb_cp_count < 7)
        {
            rb_capsule_pair* cp =
                m_collision_callback.m_rb_cp_alloc_list
                    [m_collision_callback.m_rb_cp_count++];
            if (cp != nullptr)
            {
                cp->m_b2_cg = cg6;
                cp->m_b1_cg = cg9;
            }
        }
        else
        {
            tlFatal("phys memory pool add overflow.");
        }
    }
    {
        rigid_body_sphere_list* cg0 = m_collision_callback.get_colgeom(0);
        rigid_body_sphere_list* cg3 = m_collision_callback.get_colgeom(3);
        if (m_collision_callback.m_rb_cp_count < 7)
        {
            rb_capsule_pair* cp =
                m_collision_callback.m_rb_cp_alloc_list
                    [m_collision_callback.m_rb_cp_count++];
            if (cp != nullptr)
            {
                cp->m_b2_cg = cg0;
                cp->m_b1_cg = cg3;
            }
        }
        else
        {
            tlFatal("phys memory pool add overflow.");
        }
    }
    {
        rigid_body_sphere_list* cg5 = m_collision_callback.get_colgeom(5);
        rigid_body_sphere_list* cg0 = m_collision_callback.get_colgeom(0);
        if (m_collision_callback.m_rb_cp_count < 7)
        {
            rb_capsule_pair* cp =
                m_collision_callback.m_rb_cp_alloc_list
                    [m_collision_callback.m_rb_cp_count++];
            if (cp != nullptr)
            {
                cp->m_b2_cg = cg5;
                cp->m_b1_cg = cg0;
            }
        }
        else
        {
            tlFatal("phys memory pool add overflow.");
        }
    }
    {
        rigid_body_sphere_list* cg6 = m_collision_callback.get_colgeom(6);
        rigid_body_sphere_list* cg3 = m_collision_callback.get_colgeom(3);
        if (m_collision_callback.m_rb_cp_count < 7)
        {
            rb_capsule_pair* cp =
                m_collision_callback.m_rb_cp_alloc_list
                    [m_collision_callback.m_rb_cp_count++];
            if (cp != nullptr)
            {
                cp->m_b2_cg = cg6;
                cp->m_b1_cg = cg3;
            }
        }
        else
        {
            tlFatal("phys memory pool add overflow.");
        }
    }
    {
        rigid_body_sphere_list* cg8 = m_collision_callback.get_colgeom(8);
        rigid_body_sphere_list* cg5 = m_collision_callback.get_colgeom(5);
        if (m_collision_callback.m_rb_cp_count < 7)
        {
            rb_capsule_pair* cp =
                m_collision_callback.m_rb_cp_alloc_list
                    [m_collision_callback.m_rb_cp_count++];
            if (cp != nullptr)
            {
                cp->m_b2_cg = cg8;
                cp->m_b1_cg = cg5;
            }
        }
        else
        {
            tlFatal("phys memory pool add overflow.");
        }
    }

    phys_anim_bone_array::write_skeleton(owner, skeleton_pose);
    for (int i = 0; i < 10; ++i)
    {
        client_bone_info& cbi =
            ((client_bone_info*)((char*)this + 0x1D20))[i];
        rigid_body* rigid_body = get_rigid_body(cbi.m_rb_index);
        calc_rb_mat_from_bone(cbi.m_ent, cbi.m_bone_index, rigid_body,
                              cbi.m_bone_mat_loc);
    }
    pull_joints_together();
    if (g_in_physics_collision_callback)
    {
        for (int j = 0; j < 10; ++j)
        {
            rigid_body* v203 = get_rigid_body(j);
            v203->update_col_mat();
        }
    }
    math::Dir3 tunnel_center;
    tunnel_center.v = _mm_setr_ps(0.0f, 0.0f, 65.0f, 0.0f);
    for (int k = 0; k < 10; ++k)
    {
        math::Position3 center;
        center.v = _mm_add_ps(owner->r.currentOrigin.v, tunnel_center.v);
        rigid_body_sphere_list* v206 = m_collision_callback.get_colgeom(k);
        if (v206 != nullptr)
            v206->init_tunnel_test(center);
    }
    set_gravity_multiplier(1.0f * PHYSICS_GRAVITY_SCALE_1);
}
