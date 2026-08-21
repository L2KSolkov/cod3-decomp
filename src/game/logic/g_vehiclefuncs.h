// ============================================================================
// g_vehiclefuncs.h - vehicleFuncs + vehicle physics stat helpers (game2.o)
// Layouts verified against IDA local types.
// ============================================================================

#pragma once

#include <stddef.h>

// vehicle_info_t - 0x310 (IDA verified; subset for stat helpers)
struct vehicle_info_t {
    char name[0x20];             // +0x00
    short type;                  // +0x20
    short subtype;               // +0x22
    int steerWheels;             // +0x24
    int texScroll;               // +0x28
    int quadBarrel;              // +0x2C
    float bulletDamage;          // +0x30
    float grenadeDamage;         // +0x34
    float mineDamage;            // +0x38
    float projectileDamage;      // +0x3C
    int spClientSeat;            // +0x40
    int numSeats;                // +0x44
    int hudIndex;                // +0x48
    float texScrollScale;        // +0x4C
    float maxSpeed;              // +0x50
    float accel;                 // +0x54
    float rotRate;               // +0x58
    float rotAccel;              // +0x5C
    float maxBodyPitch;          // +0x60
    float maxBodyRoll;           // +0x64
    float collisionDamage;       // +0x68
    float collisionSpeed;        // +0x6C
    float suspensionTravel;      // +0x70
    float boundsRadius;          // +0x74
    float boundsHeight;          // +0x78
    float boundsLength;          // +0x7C
    int health;                  // +0x80
    unsigned char _pad84[0x164 - 0x84];
    float turretHorizSpanLeft;   // +0x164
    float turretHorizSpanRight;  // +0x168
    float turretVertSpanUp;      // +0x16C
    float turretVertSpanDown;    // +0x170
    float turretRotRate;         // +0x174
    float turretSwirlLerpRate;   // +0x178
    float turretSwirlPitchFactor;// +0x17C
    float turretGunnerVertSpanUp;   // +0x180
    float turretGunnerVertSpanDown; // +0x184
    float engineSndSpeed;        // +0x188
    unsigned char _pad18C[0x1B0 - 0x18C];
    float cameraZOffset;         // +0x1B0
    float cameraFPHeightOffset;  // +0x1B4
    float cameraFPFwdOffset;     // +0x1B8
    float cameraFPHeightLerp;    // +0x1BC
    float cameraChaseOffsetX;    // +0x1C0
    float cameraChaseOffsetY;    // +0x1C4
    float cameraChaseOffsetZ;    // +0x1C8
    float cameraChaseRadiusInner;   // +0x1CC
    float cameraChaseRadiusOuter;   // +0x1D0
    float cameraVehViewRadius;      // +0x1D4
    float cameraVehViewMaxPitch;    // +0x1D8
    float cameraVehViewMaxPitchDistAdj;  // +0x1DC
    float cameraVehViewFwdBackRatio;     // +0x1E0
    float cameraVehViewMoveInPitch;     // +0x1E4
    float camLinkedPitchFactor;         // +0x1E8
    float pitchBasedCamOffsetX;         // +0x1EC
    float pitchBasedCamOffsetZ;         // +0x1F0
    unsigned char _pad1F4[0x280 - 0x1F4];
    int vehicleAnimMatrixColumn;        // +0x280
    unsigned char _pad284[0x2C4 - 0x284];
    float hatchOpenAngleRight;   // +0x2C4
    float hatchOpenAngleLeft;    // +0x2C8
    unsigned char _pad2CC[0x30C - 0x2CC];
    int inactiveBlowupSeconds;   // +0x30C
};
static_assert(sizeof(vehicle_info_t) == 0x310, "vehicle_info_t size mismatch");

vehicle_info_t* VEH_GetPlayerVehicleInfo();  // game.o

namespace vehicleFuncs {
bool steerWheels_Function(bool v);
bool quadBarrel_Function(bool v);
int spClientSeat_Function(int v);
int hudIndex_Function(int v);
int numSeats_Function(int v);
int health_Function(int v);
int vehicleAnimMatrixColumn_Function(int v);
int inactiveBlowupSeconds_Function(int v);
float bulletDamage_Function(float v);
float grenadeDamage_Function(float v);
float mineDamage_Function(float v);
float projectileDamage_Function(float v);
float texureScrollScale_Function(float v);
float maxSpeed_Function(float v);
float accel_Function(float v);
float rotRate_Function(float v);
float rotAccel_Function(float v);
float maxBodyPitch_Function(float v);
float maxBodyRoll_Function(float v);
float collisionDamage_Function(float v);
float collisionSpeed_Function(float v);
float suspensionTravel_Function(float v);
float boundsRadius_Function(float v);
float boundsHeight_Function(float v);
float boundsLength_Function(float v);
float turretHorizSpanLeft_Function(float v);
float turretHorizSpanRight_Function(float v);
float turretVertSpanUp_Function(float v);
float turretVertSpanDown_Function(float v);
float turretRotRate_Function(float v);
float turretSwirlLerpRate_Function(float v);
float turretSwirlPitchFactor_Function(float v);
float turretGunnerVertSpanUp_Function(float v);
float turretGunnerVertSpanDown_Function(float v);
float engineSndSpeed_Function(float v);
float cameraZOffset_Function(float v);
float cameraFPHeightOffset_Function(float v);
float cameraFPFwdOffset_Function(float v);
float cameraFPHeightLerp_Function(float v);
float cameraChaseOffsetX_Function(float v);
float cameraChaseOffsetY_Function(float v);
float cameraChaseOffsetZ_Function(float v);
float cameraChaseRadiusInner_Function(float v);
float cameraChaseRadiusOuter_Function(float v);
float cameraVehViewRadius_Function(float v);
float cameraVehViewMaxPitch_Function(float v);
float cameraVehViewMaxPitchDistAdj_Function(float v);
float cameraVehViewFwdBackRatio_Function(float v);
float cameraVehViewMoveInPitch_Function(float v);
float camLinkedPitchFactor_Function(float v);
float pitchBasedCamOffsetX_Function(float v);
float pitchBasedCamOffsetZ_Function(float v);
float hatchOpenAngleRight_Function(float v);
float hatchOpenAngleLeft_Function(float v);
float texureScroll_Function(float v);
} // namespace vehicleFuncs

// vehicle_rb_parameter - 0xD0 (IDA verified; subset)
class vehicle_rb_parameter {
public:
    float m_speed_max;          // +0x00
    float m_accel_max;          // +0x04
    float m_reverse_scale;      // +0x08
    float m_steer_angle_max;    // +0x0C
    float m_steer_speed;        // +0x10
    float m_wheel_radius;       // +0x14
    float m_susp_spring_k;      // +0x18
    float m_susp_damp_k;        // +0x1C
    float m_susp_adj;           // +0x20
    float m_susp_hard_limit;    // +0x24
    float m_tire_fric_fwd;      // +0x28
    float m_tire_fric_side;     // +0x2C
    float m_tire_fric_brake;    // +0x30
    float m_tire_fric_hand_brake;  // +0x34
    float m_body_mass;          // +0x38
    float m_mass_center_delta_x;  // +0x3C
    float m_mass_center_delta_y;  // +0x40
    float m_mass_center_delta_z;  // +0x44
    float m_roll_stability;     // +0x48
    float m_roll_resistance;    // +0x4C
    float m_upright_strength;   // +0x50
    float m_tilt_fakey;         // +0x54
    float m_peel_out_max_speed; // +0x58
    float m_inertia_scale_x;    // +0x5C
    float m_tire_damp_coast;    // +0x60
    float m_tire_damp_brake;    // +0x64
    float m_tire_damp_hand;     // +0x68
    unsigned char _pad6C[0xB0 - 0x6C];
    struct BBox { float v[4]; } m_bbox_min;  // +0xB0
    struct BBox m_bbox_max;                 // +0xC0
};
static_assert(sizeof(vehicle_rb_parameter) == 0xD0, "vehicle_rb_parameter size mismatch");

class rb_vehicle {
public:
    unsigned char _pad[0x250];      // +0x000
    vehicle_rb_parameter* m_parameter;  // +0x250
    unsigned char _pad254[0x280 - 0x254];
    unsigned int m_flags;           // +0x280
    void update_parms(vehicle_rb_parameter* params, bool initialization);  // ?update_parms@rb_vehicle@@QAEXPAVvehicle_rb_parameter@@_N@Z
};
rb_vehicle* GetPlayerRBVehicle();  // game2.o
float SetVehicleInertiaBox(bool setMin, int xyz, float f);
float SetVehicleInertiaBoxMinX(float f);
float SetVehicleInertiaBoxMaxX(float f);
float SetVehicleInertiaBoxMinY(float f);
float SetVehicleInertiaBoxMaxY(float f);
float SetVehicleInertiaBoxMinZ(float f);
float SetVehicleInertiaBoxMaxZ(float f);
float speed_max_Function(float f);
float accel_max_Function(float f);
float reverse_scale_Function(float f);
float steer_angle_max_Function(float f);
float steer_speed_Function(float f);
float wheel_radius_Function(float f);
float susp_spring_k_Function(float f);
float susp_damp_k_Function(float f);
float susp_adj_Function(float f);
float susp_hard_limit_Function(float f);
float tire_fric_fwd_Function(float f);
float tire_fric_side_Function(float f);
float tire_fric_brake_Function(float f);
float tire_fric_hand_brake_Function(float f);
float body_mass_Function(float f);
float mass_center_delta_x_Function(float f);
float mass_center_delta_y_Function(float f);
float mass_center_delta_z_Function(float f);
float roll_stability_Function(float f);
float roll_resistance_Function(float f);
float upright_strength_Function(float f);
float tilt_fakey_Function(float f);
float peel_out_max_speed_Function(float f);
float inertia_scale_x_Function(float f);
float tire_damp_coast_Function(float f);
float tire_damp_brake_Function(float f);
float tire_damp_hand_Function(float f);
