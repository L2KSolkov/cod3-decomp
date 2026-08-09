// ============================================================================
// g_vehiclefuncs.cpp - vehicleFuncs + vehicle physics stat helpers (game2.o)
// ============================================================================

#include "game/logic/g_vehiclefuncs.h"

namespace vehicleFuncs {

// ea: 0x004EFB70
int steerWheels_Function(int v)
{
    vehicle_info_t* info = VEH_GetPlayerVehicleInfo();
    if (info == nullptr)
        return 0;
    int r = v + info->steerWheels;
    info->steerWheels = r;
    return r;
}

// ea: 0x004EFB90
int quadBarrel_Function(int v)
{
    vehicle_info_t* info = VEH_GetPlayerVehicleInfo();
    if (info == nullptr)
        return 0;
    int r = v + info->quadBarrel;
    info->quadBarrel = r;
    return r;
}

// ea: 0x004EFBB0
double bulletDamage_Function(float v)
{
    vehicle_info_t* info = VEH_GetPlayerVehicleInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->bulletDamage;
    info->bulletDamage = r;
    return r;
}

// ea: 0x004EFBF0
double grenadeDamage_Function(float v)
{
    vehicle_info_t* info = VEH_GetPlayerVehicleInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->grenadeDamage;
    info->grenadeDamage = r;
    return r;
}

// ea: 0x004EFC30
double mineDamage_Function(float v)
{
    vehicle_info_t* info = VEH_GetPlayerVehicleInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->mineDamage;
    info->mineDamage = r;
    return r;
}

// ea: 0x004EFC70
double projectileDamage_Function(float v)
{
    vehicle_info_t* info = VEH_GetPlayerVehicleInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->projectileDamage;
    info->projectileDamage = r;
    return r;
}

// ea: 0x004EFCB0
int spClientSeat_Function(int v)
{
    vehicle_info_t* info = VEH_GetPlayerVehicleInfo();
    if (info == nullptr)
        return 0;
    int r = v + info->spClientSeat;
    info->spClientSeat = r;
    return r;
}

// ea: 0x004EFCD0
int hudIndex_Function(int v)
{
    vehicle_info_t* info = VEH_GetPlayerVehicleInfo();
    if (info == nullptr)
        return 0;
    int r = v + info->hudIndex;
    info->hudIndex = r;
    return r;
}

// ea: 0x004EFCF0
int numSeats_Function(int v)
{
    vehicle_info_t* info = VEH_GetPlayerVehicleInfo();
    if (info == nullptr)
        return 0;
    int r = v + info->numSeats;
    info->numSeats = r;
    return r;
}

// ea: 0x004EFD10
int health_Function(int v)
{
    vehicle_info_t* info = VEH_GetPlayerVehicleInfo();
    if (info == nullptr)
        return 0;
    int r = v + info->health;
    info->health = r;
    return r;
}

// ea: 0x004EFD40
double texureScroll_Function(float v)
{
    vehicle_info_t* info = VEH_GetPlayerVehicleInfo();
    if (info == nullptr)
        return 0;
    int r = (int)v + info->texScroll;
    info->texScroll = r;
    return r;
}

// ea: 0x004EFD80
double texureScrollScale_Function(float v)
{
    vehicle_info_t* info = VEH_GetPlayerVehicleInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->texScrollScale;
    info->texScrollScale = r;
    return r;
}

// ea: 0x004EFDC0
double engineSndSpeed_Function(float v)
{
    vehicle_info_t* info = VEH_GetPlayerVehicleInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->engineSndSpeed;
    info->engineSndSpeed = r;
    return r;
}

// ea: 0x004EFE00
double maxSpeed_Function(float v)
{
    vehicle_info_t* info = VEH_GetPlayerVehicleInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->maxSpeed;
    info->maxSpeed = r;
    return r;
}

// ea: 0x004EFE40
double accel_Function(float v)
{
    vehicle_info_t* info = VEH_GetPlayerVehicleInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->accel;
    info->accel = r;
    return r;
}

// ea: 0x004EFE80
double rotRate_Function(float v)
{
    vehicle_info_t* info = VEH_GetPlayerVehicleInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->rotRate;
    info->rotRate = r;
    return r;
}

// ea: 0x004EFEC0
double rotAccel_Function(float v)
{
    vehicle_info_t* info = VEH_GetPlayerVehicleInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->rotAccel;
    info->rotAccel = r;
    return r;
}

// ea: 0x004EFF00
double collisionDamage_Function(float v)
{
    vehicle_info_t* info = VEH_GetPlayerVehicleInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->collisionDamage;
    info->collisionDamage = r;
    return r;
}

// ea: 0x004EFF40
double collisionSpeed_Function(float v)
{
    vehicle_info_t* info = VEH_GetPlayerVehicleInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->collisionSpeed;
    info->collisionSpeed = r;
    return r;
}

// ea: 0x004EFF80
double suspensionTravel_Function(float v)
{
    vehicle_info_t* info = VEH_GetPlayerVehicleInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->suspensionTravel;
    info->suspensionTravel = r;
    return r;
}

// ea: 0x004EFFC0
double maxBodyPitch_Function(float v)
{
    vehicle_info_t* info = VEH_GetPlayerVehicleInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->maxBodyPitch;
    info->maxBodyPitch = r;
    return r;
}

// ea: 0x004F0000
double maxBodyRoll_Function(float v)
{
    vehicle_info_t* info = VEH_GetPlayerVehicleInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->maxBodyRoll;
    info->maxBodyRoll = r;
    return r;
}

// ea: 0x004F0040
double boundsRadius_Function(float v)
{
    vehicle_info_t* info = VEH_GetPlayerVehicleInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->boundsRadius;
    info->boundsRadius = r;
    return r;
}

// ea: 0x004F0080
double boundsHeight_Function(float v)
{
    vehicle_info_t* info = VEH_GetPlayerVehicleInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->boundsHeight;
    info->boundsHeight = r;
    return r;
}

// ea: 0x004F00C0
double boundsLength_Function(float v)
{
    vehicle_info_t* info = VEH_GetPlayerVehicleInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->boundsLength;
    info->boundsLength = r;
    return r;
}

// ea: 0x004F0100
double turretHorizSpanLeft_Function(float v)
{
    vehicle_info_t* info = VEH_GetPlayerVehicleInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->turretHorizSpanLeft;
    info->turretHorizSpanLeft = r;
    return r;
}

// ea: 0x004F0140
double turretHorizSpanRight_Function(float v)
{
    vehicle_info_t* info = VEH_GetPlayerVehicleInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->turretHorizSpanRight;
    info->turretHorizSpanRight = r;
    return r;
}

// ea: 0x004F0180
double turretVertSpanUp_Function(float v)
{
    vehicle_info_t* info = VEH_GetPlayerVehicleInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->turretVertSpanUp;
    info->turretVertSpanUp = r;
    return r;
}

// ea: 0x004F01C0
double turretVertSpanDown_Function(float v)
{
    vehicle_info_t* info = VEH_GetPlayerVehicleInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->turretVertSpanDown;
    info->turretVertSpanDown = r;
    return r;
}

// ea: 0x004F0200
double turretRotRate_Function(float v)
{
    vehicle_info_t* info = VEH_GetPlayerVehicleInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->turretRotRate;
    info->turretRotRate = r;
    return r;
}

// ea: 0x004F0240
double turretSwirlLerpRate_Function(float v)
{
    vehicle_info_t* info = VEH_GetPlayerVehicleInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->turretSwirlLerpRate;
    info->turretSwirlLerpRate = r;
    return r;
}

// ea: 0x004F0280
double turretSwirlPitchFactor_Function(float v)
{
    vehicle_info_t* info = VEH_GetPlayerVehicleInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->turretSwirlPitchFactor;
    info->turretSwirlPitchFactor = r;
    return r;
}

// ea: 0x004F02C0
double turretGunnerVertSpanUp_Function(float v)
{
    vehicle_info_t* info = VEH_GetPlayerVehicleInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->turretGunnerVertSpanUp;
    info->turretGunnerVertSpanUp = r;
    return r;
}

// ea: 0x004F0300
double turretGunnerVertSpanDown_Function(float v)
{
    vehicle_info_t* info = VEH_GetPlayerVehicleInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->turretGunnerVertSpanDown;
    info->turretGunnerVertSpanDown = r;
    return r;
}

// ea: 0x004F0340
double cameraZOffset_Function(float v)
{
    vehicle_info_t* info = VEH_GetPlayerVehicleInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->cameraZOffset;
    info->cameraZOffset = r;
    return r;
}

// ea: 0x004F0380
double cameraFPHeightOffset_Function(float v)
{
    vehicle_info_t* info = VEH_GetPlayerVehicleInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->cameraFPHeightOffset;
    info->cameraFPHeightOffset = r;
    return r;
}

// ea: 0x004F03C0
double cameraFPFwdOffset_Function(float v)
{
    vehicle_info_t* info = VEH_GetPlayerVehicleInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->cameraFPFwdOffset;
    info->cameraFPFwdOffset = r;
    return r;
}

// ea: 0x004F0400
double cameraFPHeightLerp_Function(float v)
{
    vehicle_info_t* info = VEH_GetPlayerVehicleInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->cameraFPHeightLerp;
    info->cameraFPHeightLerp = r;
    return r;
}

// ea: 0x004F0440
double cameraChaseOffsetX_Function(float v)
{
    vehicle_info_t* info = VEH_GetPlayerVehicleInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->cameraChaseOffsetX;
    info->cameraChaseOffsetX = r;
    return r;
}

// ea: 0x004F0480
double cameraChaseOffsetY_Function(float v)
{
    vehicle_info_t* info = VEH_GetPlayerVehicleInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->cameraChaseOffsetY;
    info->cameraChaseOffsetY = r;
    return r;
}

// ea: 0x004F04C0
double cameraChaseOffsetZ_Function(float v)
{
    vehicle_info_t* info = VEH_GetPlayerVehicleInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->cameraChaseOffsetZ;
    info->cameraChaseOffsetZ = r;
    return r;
}

// ea: 0x004F0500
double cameraChaseRadiusInner_Function(float v)
{
    vehicle_info_t* info = VEH_GetPlayerVehicleInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->cameraChaseRadiusInner;
    info->cameraChaseRadiusInner = r;
    return r;
}

// ea: 0x004F0540
double cameraChaseRadiusOuter_Function(float v)
{
    vehicle_info_t* info = VEH_GetPlayerVehicleInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->cameraChaseRadiusOuter;
    info->cameraChaseRadiusOuter = r;
    return r;
}

// ea: 0x004F0580
double cameraVehViewRadius_Function(float v)
{
    vehicle_info_t* info = VEH_GetPlayerVehicleInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->cameraVehViewRadius;
    info->cameraVehViewRadius = r;
    return r;
}

// ea: 0x004F05C0
double cameraVehViewMaxPitch_Function(float v)
{
    vehicle_info_t* info = VEH_GetPlayerVehicleInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->cameraVehViewMaxPitch;
    info->cameraVehViewMaxPitch = r;
    return r;
}

// ea: 0x004F0600
double cameraVehViewMaxPitchDistAdj_Function(float v)
{
    vehicle_info_t* info = VEH_GetPlayerVehicleInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->cameraVehViewMaxPitchDistAdj;
    info->cameraVehViewMaxPitchDistAdj = r;
    return r;
}

// ea: 0x004F0640
double cameraVehViewFwdBackRatio_Function(float v)
{
    vehicle_info_t* info = VEH_GetPlayerVehicleInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->cameraVehViewFwdBackRatio;
    info->cameraVehViewFwdBackRatio = r;
    return r;
}

// ea: 0x004F0680
double cameraVehViewMoveInPitch_Function(float v)
{
    vehicle_info_t* info = VEH_GetPlayerVehicleInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->cameraVehViewMoveInPitch;
    info->cameraVehViewMoveInPitch = r;
    return r;
}

// ea: 0x004F06C0
double camLinkedPitchFactor_Function(float v)
{
    vehicle_info_t* info = VEH_GetPlayerVehicleInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->camLinkedPitchFactor;
    info->camLinkedPitchFactor = r;
    return r;
}

// ea: 0x004F0700
double pitchBasedCamOffsetX_Function(float v)
{
    vehicle_info_t* info = VEH_GetPlayerVehicleInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->pitchBasedCamOffsetX;
    info->pitchBasedCamOffsetX = r;
    return r;
}

// ea: 0x004F0740
double pitchBasedCamOffsetZ_Function(float v)
{
    vehicle_info_t* info = VEH_GetPlayerVehicleInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->pitchBasedCamOffsetZ;
    info->pitchBasedCamOffsetZ = r;
    return r;
}

// ea: 0x004F0780
int vehicleAnimMatrixColumn_Function(int v)
{
    vehicle_info_t* info = VEH_GetPlayerVehicleInfo();
    if (info == nullptr)
        return 0;
    int r = v + info->vehicleAnimMatrixColumn;
    info->vehicleAnimMatrixColumn = r;
    return r;
}

// ea: 0x004F07B0
double hatchOpenAngleRight_Function(float v)
{
    vehicle_info_t* info = VEH_GetPlayerVehicleInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->hatchOpenAngleRight;
    info->hatchOpenAngleRight = r;
    return r;
}

// ea: 0x004F07F0
double hatchOpenAngleLeft_Function(float v)
{
    vehicle_info_t* info = VEH_GetPlayerVehicleInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->hatchOpenAngleLeft;
    info->hatchOpenAngleLeft = r;
    return r;
}

// ea: 0x004F0830
int inactiveBlowupSeconds_Function(int v)
{
    vehicle_info_t* info = VEH_GetPlayerVehicleInfo();
    if (info == nullptr)
        return 0;
    int r = v + info->inactiveBlowupSeconds;
    info->inactiveBlowupSeconds = r;
    return r;
}

} // namespace vehicleFuncs

// ============================================================================
// vehicle physics stat helpers (GetPlayerRBVehicle + vehicle_rb_parameter)
// ============================================================================

// ea: 0x004EF150
double speed_max_Function(float f)
{
    rb_vehicle* vehicle = GetPlayerRBVehicle();
    if (vehicle == nullptr)
        return 0;
    vehicle_rb_parameter* params = vehicle->m_parameter;
    if (params == nullptr)
        return 0;
    params->m_speed_max = params->m_speed_max + f;
    if (f != 0.0f)
        vehicle->update_parms(params, false);
    return params->m_speed_max;
}

// ea: 0x004EF1B0
double accel_max_Function(float f)
{
    rb_vehicle* vehicle = GetPlayerRBVehicle();
    if (vehicle == nullptr)
        return 0;
    vehicle_rb_parameter* params = vehicle->m_parameter;
    if (params == nullptr)
        return 0;
    params->m_accel_max = params->m_accel_max + f;
    if (f != 0.0f)
        vehicle->update_parms(params, false);
    return params->m_accel_max;
}

// ea: 0x004EF210
double reverse_scale_Function(float f)
{
    rb_vehicle* vehicle = GetPlayerRBVehicle();
    if (vehicle == nullptr)
        return 0;
    vehicle_rb_parameter* params = vehicle->m_parameter;
    if (params == nullptr)
        return 0;
    params->m_reverse_scale = params->m_reverse_scale + f;
    if (f != 0.0f)
        vehicle->update_parms(params, false);
    return params->m_reverse_scale;
}

// ea: 0x004EF270
double steer_angle_max_Function(float f)
{
    rb_vehicle* vehicle = GetPlayerRBVehicle();
    if (vehicle == nullptr)
        return 0;
    vehicle_rb_parameter* params = vehicle->m_parameter;
    if (params == nullptr)
        return 0;
    params->m_steer_angle_max = params->m_steer_angle_max + f;
    if (f != 0.0f)
        vehicle->update_parms(params, false);
    return params->m_steer_angle_max;
}

// ea: 0x004EF2D0
double steer_speed_Function(float f)
{
    rb_vehicle* vehicle = GetPlayerRBVehicle();
    if (vehicle == nullptr)
        return 0;
    vehicle_rb_parameter* params = vehicle->m_parameter;
    if (params == nullptr)
        return 0;
    params->m_steer_speed = params->m_steer_speed + f;
    if (f != 0.0f)
        vehicle->update_parms(params, false);
    return params->m_steer_speed;
}

// ea: 0x004EF330
double wheel_radius_Function(float f)
{
    rb_vehicle* vehicle = GetPlayerRBVehicle();
    if (vehicle == nullptr)
        return 0;
    vehicle_rb_parameter* params = vehicle->m_parameter;
    if (params == nullptr)
        return 0;
    params->m_wheel_radius = params->m_wheel_radius + f;
    if (f != 0.0f)
        vehicle->update_parms(params, false);
    return params->m_wheel_radius;
}

// ea: 0x004EF390
double susp_spring_k_Function(float f)
{
    rb_vehicle* vehicle = GetPlayerRBVehicle();
    if (vehicle == nullptr)
        return 0;
    vehicle_rb_parameter* params = vehicle->m_parameter;
    if (params == nullptr)
        return 0;
    params->m_susp_spring_k = params->m_susp_spring_k + f;
    if (f != 0.0f)
        vehicle->update_parms(params, false);
    return params->m_susp_spring_k;
}

// ea: 0x004EF3F0
double susp_damp_k_Function(float f)
{
    rb_vehicle* vehicle = GetPlayerRBVehicle();
    if (vehicle == nullptr)
        return 0;
    vehicle_rb_parameter* params = vehicle->m_parameter;
    if (params == nullptr)
        return 0;
    params->m_susp_damp_k = params->m_susp_damp_k + f;
    if (f != 0.0f)
        vehicle->update_parms(params, false);
    return params->m_susp_damp_k;
}

// ea: 0x004EF450
double susp_adj_Function(float f)
{
    rb_vehicle* vehicle = GetPlayerRBVehicle();
    if (vehicle == nullptr)
        return 0;
    vehicle_rb_parameter* params = vehicle->m_parameter;
    if (params == nullptr)
        return 0;
    params->m_susp_adj = params->m_susp_adj + f;
    if (f != 0.0f)
        vehicle->update_parms(params, false);
    return params->m_susp_adj;
}

// ea: 0x004EF4B0
double susp_hard_limit_Function(float f)
{
    rb_vehicle* vehicle = GetPlayerRBVehicle();
    if (vehicle == nullptr)
        return 0;
    vehicle_rb_parameter* params = vehicle->m_parameter;
    if (params == nullptr)
        return 0;
    params->m_susp_hard_limit = params->m_susp_hard_limit + f;
    if (f != 0.0f)
        vehicle->update_parms(params, false);
    return params->m_susp_hard_limit;
}

// ea: 0x004EF510
double tire_fric_fwd_Function(float f)
{
    rb_vehicle* vehicle = GetPlayerRBVehicle();
    if (vehicle == nullptr)
        return 0;
    vehicle_rb_parameter* params = vehicle->m_parameter;
    if (params == nullptr)
        return 0;
    params->m_tire_fric_fwd = params->m_tire_fric_fwd + f;
    if (f != 0.0f)
        vehicle->update_parms(params, false);
    return params->m_tire_fric_fwd;
}

// ea: 0x004EF570
double tire_fric_side_Function(float f)
{
    rb_vehicle* vehicle = GetPlayerRBVehicle();
    if (vehicle == nullptr)
        return 0;
    vehicle_rb_parameter* params = vehicle->m_parameter;
    if (params == nullptr)
        return 0;
    params->m_tire_fric_side = params->m_tire_fric_side + f;
    if (f != 0.0f)
        vehicle->update_parms(params, false);
    return params->m_tire_fric_side;
}

// ea: 0x004EF5D0
double tire_fric_brake_Function(float f)
{
    rb_vehicle* vehicle = GetPlayerRBVehicle();
    if (vehicle == nullptr)
        return 0;
    vehicle_rb_parameter* params = vehicle->m_parameter;
    if (params == nullptr)
        return 0;
    params->m_tire_fric_brake = params->m_tire_fric_brake + f;
    if (f != 0.0f)
        vehicle->update_parms(params, false);
    return params->m_tire_fric_brake;
}

// ea: 0x004EF630
double tire_fric_hand_brake_Function(float f)
{
    rb_vehicle* vehicle = GetPlayerRBVehicle();
    if (vehicle == nullptr)
        return 0;
    vehicle_rb_parameter* params = vehicle->m_parameter;
    if (params == nullptr)
        return 0;
    params->m_tire_fric_hand_brake = params->m_tire_fric_hand_brake + f;
    if (f != 0.0f)
        vehicle->update_parms(params, false);
    return params->m_tire_fric_hand_brake;
}

// ea: 0x004EF690
double body_mass_Function(float f)
{
    rb_vehicle* vehicle = GetPlayerRBVehicle();
    if (vehicle == nullptr)
        return 0;
    vehicle_rb_parameter* params = vehicle->m_parameter;
    if (params == nullptr)
        return 0;
    params->m_body_mass = params->m_body_mass + f;
    if (f != 0.0f)
        vehicle->update_parms(params, false);
    return params->m_body_mass;
}

// ea: 0x004EF6F0
double mass_center_delta_x_Function(float f)
{
    rb_vehicle* vehicle = GetPlayerRBVehicle();
    if (vehicle == nullptr)
        return 0;
    vehicle_rb_parameter* params = vehicle->m_parameter;
    if (params == nullptr)
        return 0;
    params->m_mass_center_delta_x = params->m_mass_center_delta_x + f;
    if (f != 0.0f)
        vehicle->update_parms(params, false);
    return params->m_mass_center_delta_x;
}

// ea: 0x004EF750
double mass_center_delta_y_Function(float f)
{
    rb_vehicle* vehicle = GetPlayerRBVehicle();
    if (vehicle == nullptr)
        return 0;
    vehicle_rb_parameter* params = vehicle->m_parameter;
    if (params == nullptr)
        return 0;
    params->m_mass_center_delta_y = params->m_mass_center_delta_y + f;
    if (f != 0.0f)
        vehicle->update_parms(params, false);
    return params->m_mass_center_delta_y;
}

// ea: 0x004EF7B0
double mass_center_delta_z_Function(float f)
{
    rb_vehicle* vehicle = GetPlayerRBVehicle();
    if (vehicle == nullptr)
        return 0;
    vehicle_rb_parameter* params = vehicle->m_parameter;
    if (params == nullptr)
        return 0;
    params->m_mass_center_delta_z = params->m_mass_center_delta_z + f;
    if (f != 0.0f)
        vehicle->update_parms(params, false);
    return params->m_mass_center_delta_z;
}

// ea: 0x004EF810
double roll_stability_Function(float f)
{
    rb_vehicle* vehicle = GetPlayerRBVehicle();
    if (vehicle == nullptr)
        return 0;
    vehicle_rb_parameter* params = vehicle->m_parameter;
    if (params == nullptr)
        return 0;
    params->m_roll_stability = params->m_roll_stability + f;
    if (f != 0.0f)
        vehicle->update_parms(params, false);
    return params->m_roll_stability;
}

// ea: 0x004EF870
double roll_resistance_Function(float f)
{
    rb_vehicle* vehicle = GetPlayerRBVehicle();
    if (vehicle == nullptr)
        return 0;
    vehicle_rb_parameter* params = vehicle->m_parameter;
    if (params == nullptr)
        return 0;
    params->m_roll_resistance = params->m_roll_resistance + f;
    if (f != 0.0f)
        vehicle->update_parms(params, false);
    return params->m_roll_resistance;
}

// ea: 0x004EF8D0
double upright_strength_Function(float f)
{
    rb_vehicle* vehicle = GetPlayerRBVehicle();
    if (vehicle == nullptr)
        return 0;
    vehicle_rb_parameter* params = vehicle->m_parameter;
    if (params == nullptr)
        return 0;
    params->m_upright_strength = params->m_upright_strength + f;
    if (f != 0.0f)
        vehicle->update_parms(params, false);
    return params->m_upright_strength;
}

// ea: 0x004EF930
double tilt_fakey_Function(float f)
{
    rb_vehicle* vehicle = GetPlayerRBVehicle();
    if (vehicle == nullptr)
        return 0;
    vehicle_rb_parameter* params = vehicle->m_parameter;
    if (params == nullptr)
        return 0;
    params->m_tilt_fakey = params->m_tilt_fakey + f;
    if (f != 0.0f)
        vehicle->update_parms(params, false);
    return params->m_tilt_fakey;
}

// ea: 0x004EF990
double peel_out_max_speed_Function(float f)
{
    rb_vehicle* vehicle = GetPlayerRBVehicle();
    if (vehicle == nullptr)
        return 0;
    vehicle_rb_parameter* params = vehicle->m_parameter;
    if (params == nullptr)
        return 0;
    params->m_peel_out_max_speed = params->m_peel_out_max_speed + f;
    if (f != 0.0f)
        vehicle->update_parms(params, false);
    return params->m_peel_out_max_speed;
}

// ea: 0x004EF9F0
double inertia_scale_x_Function(float f)
{
    rb_vehicle* vehicle = GetPlayerRBVehicle();
    if (vehicle == nullptr)
        return 0;
    vehicle_rb_parameter* params = vehicle->m_parameter;
    if (params == nullptr)
        return 0;
    params->m_inertia_scale_x = params->m_inertia_scale_x + f;
    if (f != 0.0f)
        vehicle->update_parms(params, false);
    return params->m_inertia_scale_x;
}

// ea: 0x004EFA50
double tire_damp_coast_Function(float f)
{
    rb_vehicle* vehicle = GetPlayerRBVehicle();
    if (vehicle == nullptr)
        return 0;
    vehicle_rb_parameter* params = vehicle->m_parameter;
    if (params == nullptr)
        return 0;
    params->m_tire_damp_coast = params->m_tire_damp_coast + f;
    if (f != 0.0f)
        vehicle->update_parms(params, false);
    return params->m_tire_damp_coast;
}

// ea: 0x004EFAB0
double tire_damp_brake_Function(float f)
{
    rb_vehicle* vehicle = GetPlayerRBVehicle();
    if (vehicle == nullptr)
        return 0;
    vehicle_rb_parameter* params = vehicle->m_parameter;
    if (params == nullptr)
        return 0;
    params->m_tire_damp_brake = params->m_tire_damp_brake + f;
    if (f != 0.0f)
        vehicle->update_parms(params, false);
    return params->m_tire_damp_brake;
}

// ea: 0x004EFB10
double tire_damp_hand_Function(float f)
{
    rb_vehicle* vehicle = GetPlayerRBVehicle();
    if (vehicle == nullptr)
        return 0;
    vehicle_rb_parameter* params = vehicle->m_parameter;
    if (params == nullptr)
        return 0;
    params->m_tire_damp_hand = params->m_tire_damp_hand + f;
    if (f != 0.0f)
        vehicle->update_parms(params, false);
    return params->m_tire_damp_hand;
}

// ea: 0x004F0860
double SetVehicleInertiaBox(bool setMin, int xyz, float f)
{
    rb_vehicle* vehicle = GetPlayerRBVehicle();
    if (vehicle == nullptr)
        return 0;
    vehicle_rb_parameter* params = vehicle->m_parameter;
    if (params == nullptr)
        return 0;
    if (setMin)
        params->m_bbox_min.v[xyz] = f + params->m_bbox_min.v[xyz];
    else
        params->m_bbox_max.v[xyz] = params->m_bbox_max.v[xyz] + f;
    if (f != 0.0f)
        vehicle->update_parms(params, false);
    if (setMin)
        return params->m_bbox_min.v[xyz];
    return params->m_bbox_max.v[xyz];
}

// ea: 0x004F0900
double SetVehicleInertiaBoxMinX(float f)
{
    rb_vehicle* vehicle = GetPlayerRBVehicle();
    if (vehicle == nullptr)
        return 0;
    vehicle_rb_parameter* params = vehicle->m_parameter;
    if (params == nullptr)
        return 0;
    params->m_bbox_min.v[0] = f + params->m_bbox_min.v[0];
    if (f != 0.0f)
        vehicle->update_parms(params, false);
    return params->m_bbox_min.v[0];
}

// ea: 0x004F0980
double SetVehicleInertiaBoxMinY(float f)
{
    rb_vehicle* vehicle = GetPlayerRBVehicle();
    if (vehicle == nullptr)
        return 0;
    vehicle_rb_parameter* params = vehicle->m_parameter;
    if (params == nullptr)
        return 0;
    params->m_bbox_min.v[1] = f + params->m_bbox_min.v[1];
    if (f != 0.0f)
        vehicle->update_parms(params, false);
    return params->m_bbox_min.v[1];
}

// ea: 0x004F0A00
double SetVehicleInertiaBoxMinZ(float f)
{
    rb_vehicle* vehicle = GetPlayerRBVehicle();
    if (vehicle == nullptr)
        return 0;
    vehicle_rb_parameter* params = vehicle->m_parameter;
    if (params == nullptr)
        return 0;
    params->m_bbox_min.v[2] = f + params->m_bbox_min.v[2];
    if (f != 0.0f)
        vehicle->update_parms(params, false);
    return params->m_bbox_min.v[2];
}

// ea: 0x004F0A80
double SetVehicleInertiaBoxMaxX(float f)
{
    rb_vehicle* vehicle = GetPlayerRBVehicle();
    if (vehicle == nullptr)
        return 0;
    vehicle_rb_parameter* params = vehicle->m_parameter;
    if (params == nullptr)
        return 0;
    params->m_bbox_max.v[0] = f + params->m_bbox_max.v[0];
    if (f != 0.0f)
        vehicle->update_parms(params, false);
    return params->m_bbox_max.v[0];
}

// ea: 0x004F0B00
double SetVehicleInertiaBoxMaxY(float f)
{
    rb_vehicle* vehicle = GetPlayerRBVehicle();
    if (vehicle == nullptr)
        return 0;
    vehicle_rb_parameter* params = vehicle->m_parameter;
    if (params == nullptr)
        return 0;
    params->m_bbox_max.v[1] = f + params->m_bbox_max.v[1];
    if (f != 0.0f)
        vehicle->update_parms(params, false);
    return params->m_bbox_max.v[1];
}

// ea: 0x004F0B80
double SetVehicleInertiaBoxMaxZ(float f)
{
    rb_vehicle* vehicle = GetPlayerRBVehicle();
    if (vehicle == nullptr)
        return 0;
    vehicle_rb_parameter* params = vehicle->m_parameter;
    if (params == nullptr)
        return 0;
    params->m_bbox_max.v[2] = f + params->m_bbox_max.v[2];
    if (f != 0.0f)
        vehicle->update_parms(params, false);
    return params->m_bbox_max.v[2];
}
