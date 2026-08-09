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
int steerWheels_Function(int v);
int quadBarrel_Function(int v);
int spClientSeat_Function(int v);
int hudIndex_Function(int v);
int numSeats_Function(int v);
int health_Function(int v);
int vehicleAnimMatrixColumn_Function(int v);
int inactiveBlowupSeconds_Function(int v);
double bulletDamage_Function(float v);
double grenadeDamage_Function(float v);
double mineDamage_Function(float v);
double projectileDamage_Function(float v);
double texureScrollScale_Function(float v);
double maxSpeed_Function(float v);
double accel_Function(float v);
double rotRate_Function(float v);
double rotAccel_Function(float v);
double maxBodyPitch_Function(float v);
double maxBodyRoll_Function(float v);
double collisionDamage_Function(float v);
double collisionSpeed_Function(float v);
double suspensionTravel_Function(float v);
double boundsRadius_Function(float v);
double boundsHeight_Function(float v);
double boundsLength_Function(float v);
double turretHorizSpanLeft_Function(float v);
double turretHorizSpanRight_Function(float v);
double turretVertSpanUp_Function(float v);
double turretVertSpanDown_Function(float v);
double turretRotRate_Function(float v);
double turretSwirlLerpRate_Function(float v);
double turretSwirlPitchFactor_Function(float v);
double turretGunnerVertSpanUp_Function(float v);
double turretGunnerVertSpanDown_Function(float v);
double engineSndSpeed_Function(float v);
double cameraZOffset_Function(float v);
double cameraFPHeightOffset_Function(float v);
double cameraFPFwdOffset_Function(float v);
double cameraFPHeightLerp_Function(float v);
double cameraChaseOffsetX_Function(float v);
double cameraChaseOffsetY_Function(float v);
double cameraChaseOffsetZ_Function(float v);
double cameraChaseRadiusInner_Function(float v);
double cameraChaseRadiusOuter_Function(float v);
double cameraVehViewRadius_Function(float v);
double cameraVehViewMaxPitch_Function(float v);
double cameraVehViewMaxPitchDistAdj_Function(float v);
double cameraVehViewFwdBackRatio_Function(float v);
double cameraVehViewMoveInPitch_Function(float v);
double camLinkedPitchFactor_Function(float v);
double pitchBasedCamOffsetX_Function(float v);
double pitchBasedCamOffsetZ_Function(float v);
double hatchOpenAngleRight_Function(float v);
double hatchOpenAngleLeft_Function(float v);
double texureScroll_Function(float v);
} // namespace vehicleFuncs

// vehicle_rb_parameter - 0xD0 (IDA verified; subset)
struct vehicle_rb_parameter {
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

struct rb_vehicle {
    unsigned char _pad[0x250];      // +0x000
    vehicle_rb_parameter* m_parameter;  // +0x250
    unsigned char _pad254[0x280 - 0x254];
    unsigned int m_flags;           // +0x280
    void update_parms(vehicle_rb_parameter* params, bool initialization);  // ?update_parms@rb_vehicle@@QAEXPAVvehicle_rb_parameter@@_N@Z
};
rb_vehicle* GetPlayerRBVehicle();  // game2.o
double SetVehicleInertiaBox(bool setMin, int xyz, float f);
double SetVehicleInertiaBoxMinX(float f);
double SetVehicleInertiaBoxMaxX(float f);
double SetVehicleInertiaBoxMinY(float f);
double SetVehicleInertiaBoxMaxY(float f);
double SetVehicleInertiaBoxMinZ(float f);
double SetVehicleInertiaBoxMaxZ(float f);
double speed_max_Function(float f);
double accel_max_Function(float f);
double reverse_scale_Function(float f);
double steer_angle_max_Function(float f);
double steer_speed_Function(float f);
double wheel_radius_Function(float f);
double susp_spring_k_Function(float f);
double susp_damp_k_Function(float f);
double susp_adj_Function(float f);
double susp_hard_limit_Function(float f);
double tire_fric_fwd_Function(float f);
double tire_fric_side_Function(float f);
double tire_fric_brake_Function(float f);
double tire_fric_hand_brake_Function(float f);
double body_mass_Function(float f);
double mass_center_delta_x_Function(float f);
double mass_center_delta_y_Function(float f);
double mass_center_delta_z_Function(float f);
double roll_stability_Function(float f);
double roll_resistance_Function(float f);
double upright_strength_Function(float f);
double tilt_fakey_Function(float f);
double peel_out_max_speed_Function(float f);
double inertia_scale_x_Function(float f);
double tire_damp_coast_Function(float f);
double tire_damp_brake_Function(float f);
double tire_damp_hand_Function(float f);
