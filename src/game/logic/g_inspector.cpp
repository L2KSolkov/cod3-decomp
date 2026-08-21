// ============================================================================
// g_inspector.cpp - InspectorManager debug menu system (game2.o)
// Verified against IDA (release map offsets + 0x40C000 = VA).
// ============================================================================

#include "game/logic/g_inspector.h"
#include "game/logic/g_local.h"
#include "game/logic/g_camerashake.h"
#include "ngl/ngl_scene.h"
#include "ngl/nglDebug.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

// nsl handle types (nsl.cpp stub surface)
enum nslSourceState {
    NSL_SOURCE_STATE_INVALID = 0,
    NSL_SOURCE_STATE_QUEUING = 2,
    NSL_SOURCE_STATE_QUEUED = 3,
    NSL_SOURCE_STATE_PLAYING = 4,
    NSL_SOURCE_STATE_PAUSED = 5,
};

extern nglDebugStruct nglDebug;   // ?nglDebug@@3UnglDebugStruct@@A (ngl_debug.o)

// SoundOptions - effect sound toggles (core_systems.h layout, local view)
class SoundOptions {
public:
    int mFxDontPlayFootSteps;      // +0x00
    int mFxDontPlayGearRattle;     // +0x04
    int mFxDontPlayLanding;        // +0x08
    int mFxDontPlayScriptCall;     // +0x0C
    int mFxDontPlayScriptCall_Dir; // +0x10
    int mFxDontPlayWeapon;         // +0x14
    int mFxDontPlayBulletHit;      // +0x18
    int mFxDontPlayGrenadeBounce;  // +0x1C
    int mFxDontPlayProjExplode;    // +0x20
    int mFxDontPlayVehicle;        // +0x24
    int mFxDontPlayTurret;         // +0x28
    int mFxDontPlayVehicleWheel;   // +0x2C
    int mFxDontPlayLightFlash;     // +0x30
    int mFxDontPlayMusic;          // +0x34
};
static_assert(sizeof(SoundOptions) == 0x38, "SoundOptions size mismatch");

// Cross-object externs
extern void* mem_heap_malloc(unsigned int size);
extern void mem_heap_free(void* ptr);
extern level_locals_t level;
extern float scaleScalar;  // ?scaleScalar (render.o)
extern void RE_Text_Paint(float x, float y, int font, float scale,
                          const float* color, const char* text, float a7,
                          int a8, int a9);  // ?RE_Text_Paint@@YAXMMHMQBMPBDMHH@Z (render.o)
extern int RE_Text_Width(const char* text, int font, float scale,
                         float charWidth, int limit);  // ?RE_Text_Width

// ============================================================================
// Stat-adjuster callbacks (g_weaponfuncs.cpp / g_vehiclefuncs.cpp)
// ============================================================================
namespace weaponFuncs {
float adsAimPitch_Function(float v);
float adsBobFactor_Function(float v);
float adsBulletConeAngle_Function(float v);
float adsCrosshairInFrac_Function(float v);
float adsCrosshairOutFrac_Function(float v);
float adsGunKickAccel_Function(float v);
float adsGunKickPitchMax_Function(float v);
float adsGunKickPitchMin_Function(float v);
float adsGunKickSpeedDecay_Function(float v);
float adsGunKickSpeedMax_Function(float v);
float adsGunKickStaticDecay_Function(float v);
float adsGunKickYawMax_Function(float v);
float adsGunKickYawMin_Function(float v);
float adsIdleAmount_Function(float v);
float adsOverlayHeight_Function(float v);
float adsOverlayWidth_Function(float v);
float adsSensitivityScale_Function(float v);
float adsSensitivityScaleMP_Function(float v);
float adsSpread_Function(float v);
float adsSpreadDucked_Function(float v);
float adsSpreadDuckedMP_Function(float v);
float adsSpreadMP_Function(float v);
float adsSpreadProne_Function(float v);
float adsSpreadProneMP_Function(float v);
float adsSwayHorizScale_Function(float v);
float adsSwayLerpSpeed_Function(float v);
float adsSwayMaxAngle_Function(float v);
float adsSwayPitchScale_Function(float v);
float adsSwayVertScale_Function(float v);
float adsSwayYawScale_Function(float v);
float adsViewBobMult_Function(float v);
float adsViewKickCenterSpeed_Function(float v);
float adsViewKickPitchMax_Function(float v);
float adsViewKickPitchMin_Function(float v);
float adsViewKickYawMax_Function(float v);
float adsViewKickYawMin_Function(float v);
float adsZoomFov_Function(float v);
float adsZoomFovMP_Function(float v);
float adsZoomInFrac_Function(float v);
float adsZoomOutFrac_Function(float v);
float aiDamageMod_Function(float v);
float aiEffectiveRange_Function(float v);
float aiMissRange_Function(float v);
float animIKOffsetDist_Function(float v);
float animIKOffsetForce_Function(float v);
float animIKOffsetTime_Function(float v);
float animIKPitchAngle_Function(float v);
float animIKPitchForce_Function(float v);
float animIKPitchTime_Function(float v);
float animIKTorsoRecoilPitchAngle_Function(float v);
float animIKTorsoRecoilPitchForce_Function(float v);
float animIKTorsoRecoilPitchTime_Function(float v);
float bulletConeAngle_Function(float v);
float duckedMoveF_Function(float v);
float duckedMoveMinSpeed_Function(float v);
float duckedMoveR_Function(float v);
float duckedMoveU_Function(float v);
float duckedOfsF_Function(float v);
float duckedOfsR_Function(float v);
float duckedOfsU_Function(float v);
float gunMaxPitch_Function(float v);
float gunMaxYaw_Function(float v);
float hipGunKickAccel_Function(float v);
float hipGunKickPitchMax_Function(float v);
float hipGunKickPitchMin_Function(float v);
float hipGunKickSpeedDecay_Function(float v);
float hipGunKickSpeedMax_Function(float v);
float hipGunKickStaticDecay_Function(float v);
float hipGunKickYawMax_Function(float v);
float hipGunKickYawMin_Function(float v);
float hipIdleAmount_Function(float v);
float hipReticleSidePos_Function(float v);
float hipSpreadDecayRate_Function(float v);
float hipSpreadDecayRateMP_Function(float v);
float hipSpreadDuckedDecay_Function(float v);
float hipSpreadDuckedDecayMP_Function(float v);
float hipSpreadDuckedMin_Function(float v);
float hipSpreadDuckedMinMP_Function(float v);
float hipSpreadFireAdd_Function(float v);
float hipSpreadFireAddMP_Function(float v);
float hipSpreadMax_Function(float v);
float hipSpreadMaxMP_Function(float v);
float hipSpreadMoveAdd_Function(float v);
float hipSpreadMoveAddMP_Function(float v);
float hipSpreadProneDecay_Function(float v);
float hipSpreadProneDecayMP_Function(float v);
float hipSpreadProneMin_Function(float v);
float hipSpreadProneMinMP_Function(float v);
float hipSpreadStandMin_Function(float v);
float hipSpreadStandMinMP_Function(float v);
float hipSpreadTurnAdd_Function(float v);
float hipSpreadTurnAddMP_Function(float v);
float hipViewKickCenterSpeed_Function(float v);
float hipViewKickPitchMax_Function(float v);
float hipViewKickPitchMin_Function(float v);
float hipViewKickYawMax_Function(float v);
float hipViewKickYawMin_Function(float v);
float moveSpeedScale_Function(float v);
float posProneRotRate_Function(float v);
float proneMoveF_Function(float v);
float proneMoveMinSpeed_Function(float v);
float proneMoveR_Function(float v);
float proneMoveU_Function(float v);
float proneOfsF_Function(float v);
float proneOfsR_Function(float v);
float proneOfsU_Function(float v);
float proneRotMinSpeed_Function(float v);
float proneRotP_Function(float v);
float proneRotR_Function(float v);
float proneRotY_Function(float v);
float sensitivityScale_Function(float v);
float sensitivityScaleMP_Function(float v);
float standMoveF_Function(float v);
float standMoveMinSpeed_Function(float v);
float standMoveR_Function(float v);
float standMoveU_Function(float v);
float swayHorizScale_Function(float v);
float swayLerpSpeed_Function(float v);
float swayMaxAngle_Function(float v);
float swayPitchScale_Function(float v);
float swayShellShockScale_Function(float v);
float swayVertScale_Function(float v);
float swayYawScale_Function(float v);
int adsReloadTransTime_Function(int v);
int adsTransBlendTime_Function(int v);
int adsTransInTime_Function(int v);
int adsTransInTimeMP_Function(int v);
int adsTransOutTime_Function(int v);
int adsTransOutTimeMP_Function(int v);
int altDropTime_Function(int v);
int altRaiseTime_Function(int v);
int damage_Function(int v);
int damageInnerRadius_Function(int v);
int damageInnerRadiusMP_Function(int v);
int damageMP_Function(int v);
int damageOuterRadius_Function(int v);
int damageOuterRadiusMP_Function(int v);
int dropTime_Function(int v);
int explosionInnerDamage_Function(int v);
int explosionInnerDamageMP_Function(int v);
int explosionOuterDamage_Function(int v);
int explosionOuterDamageMP_Function(int v);
int explosionRadius_Function(int v);
int explosionRadiusMP_Function(int v);
int fireDelay_Function(int v);
int fireDelayMP_Function(int v);
int fireTime_Function(int v);
int fireTimeMP_Function(int v);
int fuseTime_Function(int v);
int holdFireTime_Function(int v);
int maxAmmoMP_Function(int v);
int meleeDamage_Function(int v);
int meleeDamageMP_Function(int v);
int meleeDelay_Function(int v);
int meleeDelayMP_Function(int v);
int meleeTime_Function(int v);
int meleeTimeMP_Function(int v);
int minDamagePercent_Function(int v);
int minDamagePercentMP_Function(int v);
int projectileSpeed_Function(int v);
int projectileSpeedUp_Function(int v);
int raiseTime_Function(int v);
int rechamberBoltTime_Function(int v);
int rechamberTime_Function(int v);
int reloadAddTime_Function(int v);
int reloadEmptyTime_Function(int v);
int reloadEndTime_Function(int v);
int reloadStartAddTime_Function(int v);
int reloadStartTime_Function(int v);
int reloadTime_Function(int v);
int reticleCenterSize_Function(int v);
int reticleMinOfs_Function(int v);
int reticleSideSize_Function(int v);
int takedamage_Function(int v);
int triggerRadius_Function(int v);
}  // namespace weaponFuncs

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
}  // namespace vehicleFuncs

// Global physics-stat helpers (binary manglings ?speed_max_Function@@YAMM@Z)
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

// ShaderCommon glow state (cdGlowShader.o / render.o)
namespace ShaderCommon {
extern float gGlowIntensity;  // ?gGlowIntensity@ShaderCommon@@3MA
extern float gGlowExpansion;  // ?gGlowExpansion@ShaderCommon@@3MA
extern int   gGlowEnable;     // ?gGlowEnable@ShaderCommon@@3HA
extern int   gGlowPasses;     // ?gGlowPasses@ShaderCommon@@3HA
extern bool  gGlowGodRays;    // ?gGlowGodRays@ShaderCommon@@3_NA
extern float gGlowBrighten;   // ?gGlowBrighten@ShaderCommon@@3MA
}

// FogConfig helpers (render.o)
namespace FogConfig {
void GetEnabled(int& enable);                    // ?GetEnabled@FogConfig@@YAXAAH@Z
void GetColor(float& red, float& green, float& blue);  // ?GetColor@FogConfig@@YAXAAM00@Z
void GetRange(float& n, float& f);               // ?GetRange@FogConfig@@YAXAAM0@Z
void GetVal(float& s, float& e);                 // ?GetVal@FogConfig@@YAXAAM0@Z
void SetColor(float r, float g, float b);        // ?SetColor@FogConfig@@YAXMMM@Z
void SetRange(float n, float f);                 // ?SetRange@FogConfig@@YAXMM@Z
void SetVal(float s, float e);                   // ?SetVal@FogConfig@@YAXMM@Z
void SetEnabled(int enable);                     // ?SetEnabled@FogConfig@@YAXH@Z
}

// game2.o data globals (glow + fog editor state, verified against the map)
int   g_GlowEnable;       // @ 0x011C86F8
int   g_GlowPasses;       // @ 0x011C86FC
float g_GlowIntensity;    // @ 0x011C8700
float g_GlowExpansion;    // @ 0x011C8704
float g_GlowBrightness;   // @ 0x011C8708
int   g_GlowGodRaysEnable;// @ 0x011C870C
int   g_FogEnable;        // @ 0x011C8710
float g_FogNear;          // @ 0x011C8714
float g_FogFar;           // @ 0x011C8718
float g_FogEnd;           // @ 0x011C871C
float g_FogRed;           // @ 0x011C8720
float g_FogGreen;         // @ 0x011C8724
float g_FogBlue;          // @ 0x011C8728
float g_FogStart;         // @ 0x012F3DF8

// cg.o / core.o externs used by Render
extern int gScreenshotInProgress;  // ?gScreenshotInProgress
extern const char* sBuildId;       // ?sBuildId
enum { CUBEMAPSHOT_NONE = 0 };

_INSPECTOR_MENU g_inspectorRootMenu;   // ?g_inspectorRootMenu (game2.o)
InspectorManager g_inspectorManager;  // ?g_inspectorManager (game2.o)

// Minimal controller view (full implementation in input/controller.cpp).
// ButtonIndex values verified against ReadKeys disassembly.
class controller { public:
    enum ButtonIndex {
        LEFTBUTTON = 0,
        DOWNBUTTON = 1,
        RIGHTBUTTON = 2,
        UPBUTTON = 3,
        SQUARE = 4,
        CIRCLE = 5,
        TRIANGLE = 7,
        R1 = 8,
        L1 = 9,
        R2 = 10,
        L2 = 11,
        R3 = 12,
        SELECT = 15,
    };
    static controller* inst();
    int button_value(ButtonIndex i_button, int* p_controller);
    bool button_pressed(ButtonIndex i_button, int* p_controller);
};

// ============================================================================
// InspectorManager::InspectorManager - ea: 0x4F7100
// ============================================================================
InspectorManager::InspectorManager()
{
    m_headingRgba[0] = 1.0f;
    m_headingRgba[1] = 1.0f;
    m_headingRgba[2] = 1.0f;
    m_headingRgba[3] = 1.0f;
    m_textRgba[0] = 1.0f;
    m_textRgba[1] = 1.0f;
    m_textRgba[2] = 0.0f;
    m_textRgba[3] = 1.0f;
    m_currentRgba[0] = 1.0f;
    m_currentRgba[1] = 1.0f;
    m_currentRgba[2] = 0.0f;
    m_currentRgba[3] = 1.0f;
    m_KEY_INSPECTOR_ONOFF = 0;
    m_KEY_SELECT = 0;
    m_KEY_UP = 0;
    m_KEY_DOWN = 0;
    m_KEY_MENU_BACK = 0;
    m_KEY_LEFT = 0;
    m_KEY_LEFT_FAST = 0;
    m_KEY_LEFT_VERY_FAST = 0;
    m_KEY_RIGHT = 0;
    m_KEY_RIGHT_FAST = 0;
    m_KEY_RIGHT_VERY_FAST = 0;
    m_KEY_LEFT_DEBOUNCE = 0;
    m_KEY_LEFT_FAST_DEBOUNCE = 0;
    m_KEY_LEFT_VERY_FAST_DEBOUNCE = 0;
    m_KEY_RIGHT_DEBOUNCE = 0;
    m_KEY_RIGHT_FAST_DEBOUNCE = 0;
    m_KEY_RIGHT_VERY_FAST_DEBOUNCE = 0;
}

int InspectorManager::IsActive()
{
    return m_data.active;
}

// ============================================================================
// InspectorManager::ResetKeys - ea: 0x4EC050
// ============================================================================
void InspectorManager::ResetKeys()
{
    m_KEY_INSPECTOR_ONOFF = 0;
    m_KEY_SELECT = 0;
    m_KEY_UP = 0;
    m_KEY_DOWN = 0;
    m_KEY_MENU_BACK = 0;
    m_KEY_LEFT = 0;
    m_KEY_LEFT_FAST = 0;
    m_KEY_LEFT_VERY_FAST = 0;
    m_KEY_RIGHT = 0;
    m_KEY_RIGHT_FAST = 0;
    m_KEY_RIGHT_VERY_FAST = 0;
    m_KEY_LEFT_DEBOUNCE = 0;
    m_KEY_LEFT_FAST_DEBOUNCE = 0;
    m_KEY_LEFT_VERY_FAST_DEBOUNCE = 0;
    m_KEY_RIGHT_DEBOUNCE = 0;
    m_KEY_RIGHT_FAST_DEBOUNCE = 0;
    m_KEY_RIGHT_VERY_FAST_DEBOUNCE = 0;
}

// ============================================================================
// InspectorManager::ReadKeys - ea: 0x4EC090
// ============================================================================
void InspectorManager::ReadKeys()
{
    bool v1 = false;
    if (level.framenum >= 10)
    {
        m_KEY_SELECT = controller::inst()->button_pressed(
            (controller::ButtonIndex)(controller::SQUARE | controller::DOWNBUTTON),
            nullptr);
        m_KEY_UP = controller::inst()->button_pressed(controller::UPBUTTON, nullptr);
        m_KEY_DOWN = controller::inst()->button_pressed(controller::DOWNBUTTON, nullptr);
        m_KEY_MENU_BACK = controller::inst()->button_pressed(
            controller::TRIANGLE, nullptr);
        m_KEY_LEFT = controller::inst()->button_value(controller::LEFTBUTTON, nullptr);
        m_KEY_LEFT_FAST = controller::inst()->button_value(controller::L1, nullptr);
        m_KEY_LEFT_VERY_FAST = controller::inst()->button_value(controller::L2, nullptr);
        m_KEY_RIGHT = controller::inst()->button_value(controller::RIGHTBUTTON, nullptr);
        m_KEY_RIGHT_FAST = controller::inst()->button_value(controller::R1, nullptr);
        m_KEY_RIGHT_VERY_FAST = controller::inst()->button_value(controller::R2, nullptr);
        m_KEY_LEFT_DEBOUNCE = controller::inst()->button_pressed(
            controller::LEFTBUTTON, nullptr);
        m_KEY_LEFT_FAST_DEBOUNCE = controller::inst()->button_pressed(
            controller::L1, nullptr);
        m_KEY_LEFT_VERY_FAST_DEBOUNCE = controller::inst()->button_pressed(
            controller::L2, nullptr);
        m_KEY_RIGHT_DEBOUNCE = controller::inst()->button_pressed(
            controller::RIGHTBUTTON, nullptr);
        m_KEY_RIGHT_FAST_DEBOUNCE = controller::inst()->button_pressed(
            controller::R1, nullptr);
        m_KEY_RIGHT_VERY_FAST_DEBOUNCE = controller::inst()->button_pressed(
            controller::R2, nullptr);
        if (m_data.active != 0)
        {
            m_KEY_INSPECTOR_ONOFF = controller::inst()->button_pressed(
                controller::SELECT, nullptr);
        }
        else
        {
            if (controller::inst()->button_pressed(controller::SELECT, nullptr))
            {
                v1 = controller::inst()->button_value(controller::R3, nullptr) != 0;
            }
            m_KEY_INSPECTOR_ONOFF = v1;
        }
    }
    else
    {
        ResetKeys();
    }
}

// ============================================================================
// InspectorManager::AddSubMenu - ea: 0x4EC260
// ============================================================================
_INSPECTOR_MENU* InspectorManager::AddSubMenu(_INSPECTOR_MENU* parent,
                                              char* heading)
{
    if (parent == nullptr)
        parent = m_data.rootMenu;
    _INSPECTOR_MENU_ITEM* v4 =
        (_INSPECTOR_MENU_ITEM*)mem_heap_malloc(0x14u);
    m_data.memory += 20;
    _INSPECTOR_MENU* menu = (_INSPECTOR_MENU*)mem_heap_malloc(0x18u);
    m_data.memory += 24;
    memset(v4, 0, sizeof(*v4));
    v4->text = (char*)mem_heap_malloc(strlen(heading) + 1);
    m_data.memory += (int)strlen(heading) + 1;
    strcpy(v4->text, heading);
    v4->next = nullptr;
    v4->prev = nullptr;
    v4->value.vp = menu;
    menu->heading = (char*)mem_heap_malloc(strlen(heading) + 1);
    m_data.memory += (int)strlen(heading) + 1;
    strcpy(menu->heading, heading);
    menu->numItems = 0;
    menu->first = nullptr;
    menu->parentMenu = parent;
    ++parent->numItems;
    _INSPECTOR_MENU_ITEM* first = parent->first;
    if (first != nullptr)
    {
        for (; first->next != nullptr; first = first->next)
            ;
        first->next = v4;
        v4->prev = first;
    }
    else
    {
        parent->first = v4;
        parent->lastCurrentItem = v4;
        v4->prev = nullptr;
    }
    parent->last = v4;
    m_data.currentItem = m_data.currentMenu->first;
    return menu;
}

// ============================================================================
// InspectorManager::AddItem - ea: 0x4EC3B0
// ============================================================================
_INSPECTOR_MENU_ITEM* InspectorManager::AddItem(_INSPECTOR_MENU* parent,
                                                char* text, void* value, int type)
{
    if (parent == nullptr)
        parent = m_data.rootMenu;
    _INSPECTOR_MENU_ITEM* v6 =
        (_INSPECTOR_MENU_ITEM*)mem_heap_malloc(0x14u);
    m_data.memory += 20;
    v6->type = type;
    v6->text = (char*)mem_heap_malloc(strlen(text) + 1);
    m_data.memory += (int)strlen(text) + 1;
    strcpy(v6->text, text);
    v6->value.vp = value;
    v6->next = nullptr;
    ++parent->numItems;
    _INSPECTOR_MENU_ITEM* first = parent->first;
    if (first != nullptr)
    {
        for (; first->next != nullptr; first = first->next)
            ;
        first->next = v6;
        v6->prev = first;
    }
    else
    {
        parent->first = v6;
        parent->lastCurrentItem = v6;
        v6->prev = nullptr;
    }
    parent->last = v6;
    m_data.currentItem = m_data.currentMenu->first;
    return v6;
}

// ============================================================================
// InspectorManager::FreeSubMenu - ea: 0x4EC490
// ============================================================================
void InspectorManager::FreeSubMenu(_INSPECTOR_MENU* menu)
{
    _INSPECTOR_MENU_ITEM* first = menu->first;
    int n = 0;
    if (menu->numItems > 0)
    {
        do
        {
            if (first->type == 0)
                FreeSubMenu((_INSPECTOR_MENU*)first->value.vp);
            if (first->text != nullptr)
                mem_heap_free(first->text);
            _INSPECTOR_MENU_ITEM* next = first->next;
            mem_heap_free(first);
            ++n;
            first = next;
        } while (n < menu->numItems);
    }
    if (menu == m_data.rootMenu)
    {
        menu->first = nullptr;
        menu->numItems = 0;
    }
    else
    {
        if (menu->heading != nullptr)
            mem_heap_free(menu->heading);
        mem_heap_free(menu);
    }
}

// ============================================================================
// InspectorManager::UpdateInput - ea: 0x4EC530
// ============================================================================
void InspectorManager::UpdateInput()
{
    ReadKeys();
    if (m_KEY_INSPECTOR_ONOFF != 0)
        m_data.active ^= 1u;
}

// ============================================================================
// InspectorManager::ProfilerUpdate - ea: 0x4EC550
// ============================================================================
void InspectorManager::ProfilerUpdate()
{
}

// ============================================================================
// InspectorManager::RenderStart - ea: 0x4EC560
// ============================================================================
int InspectorManager::RenderStart()
{
    nglListBeginScene(NGLSCENE_PARENT);
    return 1;
}

// ============================================================================
// InspectorManager::RenderFinish - ea: 0x4EC570
// ============================================================================
void InspectorManager::RenderFinish()
{
    nglListEndScene();
}

// ============================================================================
// InspectorManager::RenderSetStates - ea: 0x4EC580
// ============================================================================
void InspectorManager::RenderSetStates()
{
}

// ============================================================================
// InspectorManager::RenderRestoreStates - ea: 0x4EC590
// ============================================================================
void InspectorManager::RenderRestoreStates()
{
}

// ============================================================================
// InspectorManager::SetFontColor - ea: 0x4EC5A0
// ============================================================================
void InspectorManager::SetFontColor(float r, float g, float b, float a)
{
    m_currentRgba[0] = r;
    m_currentRgba[1] = g;
    m_currentRgba[2] = b;
    m_currentRgba[3] = a;
}

// ============================================================================
// InspectorManager::Print - ea: 0x4EC5D0
// ============================================================================
void InspectorManager::Print(char* text, int x, int y, float scale)
{
    int black[4];
    memset(black, 0, 12);
    black[3] = 0x3F800000;  // 1.0f
    float ya = (float)y;
    float xa = (float)x;
    RE_Text_Paint(xa + 2.0f, ya + 2.0f, 5, scaleScalar * scale,
                  (const float*)black, text, 0, 0, 0);
    RE_Text_Paint(xa, ya, 5, scaleScalar * scale, m_currentRgba, text, 0, 0, 0);
}

// ============================================================================
// InspectorManager::GetTextSize - ea: 0x4EC690
// ============================================================================
void InspectorManager::GetTextSize(char* pString, int* xSize, int* ySize,
                                   float scale)
{
    if (xSize != nullptr)
        *xSize = RE_Text_Width(pString, 5, scale, 0.0f, 0);
    if (ySize != nullptr)
        *ySize = 17;
}

// ============================================================================
// InspectorManager::FreeAll - ea: 0x4F7180
// ============================================================================
void InspectorManager::FreeAll()
{
    if (m_data.rootMenu != nullptr)
    {
        FreeSubMenu(m_data.rootMenu);
        m_data.rootMenu->first = nullptr;
        m_data.rootMenu->last = nullptr;
        m_data.rootMenu->numItems = 0;
        m_data.rootMenu->parentMenu = nullptr;
        m_data.rootMenu->lastCurrentItem = nullptr;
        m_data.memory = 0;
    }
}

// ============================================================================
// InspectorManager::AddRenderMenus - ea: 0x4EC720
// ============================================================================
extern int g_disableVSync;
extern int g_showGPUTimers;
extern int gRenderWorld;
extern int gRenderEntities;
extern int gRenderInstanceGroups;
extern int gRenderLightGlows;
extern int gRenderFX;
extern int gRenderSky;
extern int gRenderLocalEntities;
extern int gRenderViewWeapon;
extern int gRenderCG_2D;
extern int gRenderStatusBar;
extern int gRenderMemGraph;
extern int gRenderDebug;
extern vmCvar_t cg_norender;
extern vmCvar_t cg_fov;
extern vmCvar_t cg_widescreen;

void InspectorManager::AddRenderMenus()
{
    _INSPECTOR_MENU* v2 = AddSubMenu(nullptr, "Render Options/Switching");
    AddItem(v2, "Disable VSync", &g_disableVSync, 2);
    AddItem(v2, "Show GPU timers", &g_showGPUTimers, 2);
    _INSPECTOR_MENU* v3 = AddSubMenu(v2, "Renderer Switching");
    AddItem(v3, "Render: World", &gRenderWorld, 2);
    AddItem(v3, "Render: Entities", &gRenderEntities, 2);
    AddItem(v3, "Render: Instance Groups", &gRenderInstanceGroups, 2);
    AddItem(v3, "Render: Light Glows", &gRenderLightGlows, 2);
    AddItem(v3, "Render: FX", &gRenderFX, 2);
    AddItem(v3, "Render: Sky", &gRenderSky, 2);
    AddItem(v3, "Render: Local Entities", &gRenderLocalEntities, 2);
    AddItem(v3, "Render: View Weapon", &gRenderViewWeapon, 2);
    AddItem(v3, "Render: CG 2D", &gRenderCG_2D, 2);
    AddItem(v3, "Render: Status Bar", &gRenderStatusBar, 2);
    AddItem(v3, "Render: Mem Graph", &gRenderMemGraph, 2);
    AddItem(v3, "Render: Debug", &gRenderDebug, 2);
    AddItem(v3, "Render: CG", &cg_norender.integer, 2);
    AddItem(v3, "Fov", &cg_fov.value, 6);
    AddItem(v3, "Widescreen", &cg_widescreen.integer, 2);
}

// ============================================================================
// InspectorManager::AddPhysicsMenus - ea: 0x4EC8A0
// ============================================================================
extern int g_useRagsOnNormalDeaths;

void InspectorManager::AddPhysicsMenus()
{
    _INSPECTOR_MENU* v2 = AddSubMenu(nullptr, "Physics / Nano");
    AddItem(v2, "Rag Dolls on normal deaths", &g_useRagsOnNormalDeaths, 2);
}

// ============================================================================
// Add*Menus builders - ea: 0x4F0C00 .. 0x50D840
// ============================================================================
extern cvar_t* Cvar_Get(const char* var_name, const char* var_value,
                        int flags);  // ?Cvar_Get@@YAPAUcvar_t@@PBD0H@Z
extern void* ToggleRenderGeom();     // ?ToggleRenderGeom@@YAPAXXZ (game.o)
extern void* ToggleGraph();          // ?ToggleGraph@@YAPAXXZ (game.o)
extern void* ToggleRenderPerf();     // ?ToggleRenderPerf@@YAPAXXZ (game.o)
extern void* ZoomIn();               // ?ZoomIn@@YAPAXXZ (game.o)
extern void* ZoomOut();              // ?ZoomOut@@YAPAXXZ (game.o)
extern void FN_SelectGodMode();      // game2.o
extern void FN_NoClip();             // game2.o
extern void FN_PakRender();          // game2.o
extern bool FN_NGLStatDisplay();     // game2.o
extern void FN_ControlConfigA();     // game2.o
extern void FN_ControlConfigB();
extern void FN_ControlConfigC();
extern void FN_ControlConfigD();
extern void FN_ControlSticksDefault();
extern void FN_ControlSticksSouthPaw();
extern void FN_ControlSticksLegacy();
extern void FN_ControlSticksLegacySouthPaw();
extern bool FN_ControlInvertAim();
extern void FN_ApplyEasyDifficultyChanges();
extern void FN_ApplyMediumDifficultyChanges();
extern void FN_ApplyHardDifficultyChanges();
extern int FnReverseOptions();       // ?FnReverseOptions@@YAHXZ (game2.o)
extern int PlayRumble();             // ?PlayRumble@@YAHXZ (game2.o)
extern void FN_DefaultShellshockTestFunction();
extern void FN_PainShellshockTestFunction();
extern void FN_DeathShellshockTestFunction();
extern void FN_CurgenMotionBlur();
extern void FN_Multiplayer_MapRestart();  // game2.o
extern Entity* FN_Multiplayer_Rank1();
extern Entity* FN_Multiplayer_Rank2();
extern Client* FN_Multiplayer_Rank3();
extern Entity* FN_DebugThread_Select_Player();  // game2.o
extern EntityManager* FN_DebugThread_Select_Level();
extern void FN_DebugThread_Select_Nearest();    // game2.o
extern void FN_DebugThread_Select_Nearest_Trigger();  // game2.o
extern void FN_DebugThread_Select_UniqueIndex();      // game2.o
extern void FN_DebugThread_Select_Nearest_Vehicle();  // game2.o
extern void FN_DebugThread_Select_Target();
extern void FN_DebugAnims_Select_Target();
extern void FN_DumpThreadsForTarget();
extern void FN_DumpThreadsForAll();
extern int FN_DebugEntity_BBoxes();

// Aim-assist / input tuning globals
extern cvar_t* bg_stickyAimRender;  // ?bg_stickyAimRender@@3PAUcvar_t@@A (game.o)
float gStickyBaseSlowFactorEasy;    // ?gStickyBaseSlowFactorEasy@@3MA (game.o)
float gStickyBaseSlowFactorNormal;  // ?gStickyBaseSlowFactorNormal@@3MA (game.o)
float gStickyBaseSlowFactorHard;    // ?gStickyBaseSlowFactorHard@@3MA (game.o)
extern float gStickyBoxScaleEasy;          // ?gStickyBoxScaleEasy@@3MA (game.o)
extern float gStickyBoxScaleNormal;        // ?gStickyBoxScaleNormal@@3MA (game.o)
extern float gStickyBoxScaleHard;          // ?gStickyBoxScaleHard@@3MA (game.o)
extern float gExtraDistanceSticky;         // ?gExtraDistanceSticky@@3MA (game.o)
float gExtraStrafeSticky;           // ?gExtraStrafeSticky@@3MA (game.o)
float gLookAccelRate;               // ?gLookAccelRate@@3MA (cl.o)
extern float gMaxTurnSpeed;                // ?gMaxTurnSpeed@@3MA (cl.o)

// Collision debug globals (g.o data)
extern int gPhysicsFinder;          // ?gPhysicsFinder@@3HA (g.o)
extern int gDebugTrace;             // ?gDebugTrace@@3HA (g.o)
extern int gDebugLocationalTrace;   // ?gDebugLocationalTrace@@3HA (g.o)

// Designer / FX debug globals (game2.o + render.o data)
extern int g_displayPlayerPosition;  // ?g_displayPlayerPosition@@3HA (game2.o)
extern int g_renderFPS;              // ?g_renderFPS@@3HA (game2.o)
extern int g_showPathNodeDensity;    // ?g_showPathNodeDensity@@3HA (game2.o)
extern int g_showCulledParticles;    // ?g_showCulledParticles@@3HA (game2.o)
extern int gThreadedParticles;       // ?gThreadedParticles@@3HA (game2.o)
extern int g_renderSphere;           // ?g_renderSphere@@3HA (game2.o)
extern int g_limitVisualRange;       // ?g_limitVisualRange@@3HA (game2.o)
extern int g_displayPlayerStats;     // ?g_displayPlayerStats@@3HA (game2.o)
extern int g_testInt;                // ?g_testInt@@3HA (game2.o)
extern int g_renderGameEntityStats;  // ?g_renderGameEntityStats@@3HA (game2.o)
float g_tankTracks;           // ?g_tankTracks@@3MA (render.o)
float g_tankWheels;           // ?g_tankWheels@@3MA (render.o)
float gNearLightRadius;       // ?gNearLightRadius@@3MA (render.o)
float gFarLightRadius;        // ?gFarLightRadius@@3MA (render.o)
float g_myBlurValue;          // ?g_myBlurValue@@3MA (game2.o)
float g_myRValue;             // ?g_myRValue@@3MA (game2.o)
float g_myGValue;             // ?g_myGValue@@3MA (game2.o)
float g_myBValue;             // ?g_myBValue@@3MA (game2.o)
extern int g_blendType;              // ?g_blendType@@3HA (game2.o)
float gEasyAccuracyMod;       // ?gEasyAccuracyMod@@3MA (mp_actors.o)
float gNormalAccuracyMod;     // ?gNormalAccuracyMod@@3MA (mp_actors.o)
float gHardAccuracyMod;       // ?gHardAccuracyMod@@3MA (mp_actors.o)
extern int gNewEasyMaxHealth;        // game2.o
extern int gNewMediumMaxHealth;      // game2.o
extern int gNewHardMaxHealth;        // game2.o
float gLowFreqDelay;          // ?gLowFreqDelay@@3MA (game2.o)
float gLowFreqRumbleIntensity;   // game2.o
float gLowFreqSteadyDuration;    // game2.o
float gLowFreqRampUpTime;        // game2.o
float gLowFreqRampDownTime;      // game2.o
float gHighFreqDelay;            // ?gHighFreqDelay@@3MA (game2.o)
float gHighFreqDuration;         // game2.o
float g_objectAmbientHelper;     // ?g_objectAmbientHelper@@3MA (cg.o)
float g_objectDiffuseHelper;     // ?g_objectDiffuseHelper@@3MA (cg.o)
extern float g_ShakeTestMag;            // ?g_ShakeTestMag@@3MA (game2.o)
extern float g_ShakeTestFreq;           // game2.o
extern float g_ShakeTestTime;           // game2.o
extern int g_ShakeTest2d;               // game2.o
extern vmCvar_t cg_camerashake;         // ?cg_camerashake@@3UvmCvar_t@@A (cg.o)
extern vmCvar_t cg_shellshockblur;      // ?cg_shellshockblur@@3UvmCvar_t@@A (cg.o)
extern vmCvar_t cg_forceCrosshair;      // ?cg_forceCrosshair@@3UvmCvar_t@@A (cg.o)
extern vmCvar_t g_debugBullets;         // ?g_debugBullets@@3UvmCvar_t@@A (g.o)
extern vmCvar_t g_debugGrenades;        // ?g_debugGrenades@@3UvmCvar_t@@A (g.o)
extern vmCvar_t bg_debugWeaponAnim;     // ?bg_debugWeaponAnim@@3UvmCvar_t@@A (game.o)
extern vmCvar_t bg_debugWeaponState;    // ?bg_debugWeaponState@@3UvmCvar_t@@A (game.o)
extern vmCvar_t bg_meleeassistrange;    // ?bg_meleeassistrange@@3UvmCvar_t@@A (game.o)
extern vmCvar_t bg_meleeassistaspeed;   // ?bg_meleeassistaspeed@@3UvmCvar_t@@A (game.o)
extern vmCvar_t bg_meleeassistfov;      // ?bg_meleeassistfov@@3UvmCvar_t@@A (game.o)
extern SoundOptions gSoundOptions;      // ?gSoundOptions@@3VSoundOptions@@A

// MP debug flags (mp.o statics)
// mp.o debug-tweak statics (MPPlayer/MPPeer classes in game/sv/sv_stubs.h)
struct MPVehicle {
    static int sDebugGeneral;          // ?sDebugGeneral@MPVehicle@@2HA
    static int sDebugNetworkUpdates;   // ?sDebugNetworkUpdates@MPVehicle@@2HA
    static int sPauseNetworkUpdates;   // ?sPauseNetworkUpdates@MPVehicle@@2HA
    static float sInterpolationTime;   // ?sInterpolationTime@MPVehicle@@2MA
    static float sInterpolationTimeLan;// ?sInterpolationTimeLan@MPVehicle@@2MA
};

int MPPlayer::sDebugNetworkUpdates = 0;
int MPPlayer::sPauseNetworkUpdates = 0;
int MPPeer::mRenderDataInfo = 1;
int MPPeer::mRenderPlayerInfo = 0;
int MPPeer::mRenderSessionInfo = 0;
int MPPeer::mRenderEntityBufferInfo = 0;
int MPVehicle::sDebugGeneral = 0;
int MPVehicle::sDebugNetworkUpdates = 0;
int MPVehicle::sPauseNetworkUpdates = 0;
float MPVehicle::sInterpolationTime = 0.1f;
float MPVehicle::sInterpolationTimeLan = 0.1f;

// Multiplayer / HUD cvars (g.o / cg.o vmCvar data)
extern vmCvar_t cg_thirdPerson;          // ?cg_thirdPerson@@3UvmCvar_t@@A (cg.o)
extern vmCvar_t cg_thirdPersonRange;     // ?cg_thirdPersonRange@@3UvmCvar_t@@A (cg.o)
extern vmCvar_t cg_thirdPersonAngle;     // ?cg_thirdPersonAngle@@3UvmCvar_t@@A (cg.o)
extern vmCvar_t cg_thirdPersonLock;      // ?cg_thirdPersonLock@@3UvmCvar_t@@A (cg.o)
extern vmCvar_t mp_headIconHeight;              // ?mp_headIconHeight@@3UvmCvar_t@@A (g.o)
extern vmCvar_t mp_headIconMinScreenSize;       // ?mp_headIconMinScreenSize@@3UvmCvar_t@@A (g.o)
extern vmCvar_t mp_headIconDistAbovePlayer;     // ?mp_headIconDistAbovePlayer@@3UvmCvar_t@@A (g.o)
extern vmCvar_t mp_headIconDistAboveVehicle;    // ?mp_headIconDistAboveVehicle@@3UvmCvar_t@@A (g.o)
extern vmCvar_t mp_headIconReviveMinAlphaDist;  // ?mp_headIconReviveMinAlphaDist@@3UvmCvar_t@@A (g.o)
extern vmCvar_t mp_headIconReviveMaxAlphaDist;  // ?mp_headIconReviveMaxAlphaDist@@3UvmCvar_t@@A (g.o)
extern vmCvar_t mp_itemIconHeight;              // ?mp_itemIconHeight@@3UvmCvar_t@@A (g.o)
extern vmCvar_t mp_itemIconMinScreenSize;       // ?mp_itemIconMinScreenSize@@3UvmCvar_t@@A (g.o)
extern vmCvar_t mp_itemIconDistAboveItem;       // ?mp_itemIconDistAboveItem@@3UvmCvar_t@@A (g.o)
extern vmCvar_t mp_itemIconMinAlphaDist;        // ?mp_itemIconMinAlphaDist@@3UvmCvar_t@@A (g.o)
extern vmCvar_t mp_itemIconMaxAlphaDist;        // ?mp_itemIconMaxAlphaDist@@3UvmCvar_t@@A (g.o)
extern vmCvar_t mp_objectiveSize;               // ?mp_objectiveSize@@3UvmCvar_t@@A (g.o)
extern vmCvar_t mp_objectiveMinSize;            // ?mp_objectiveMinSize@@3UvmCvar_t@@A (g.o)
extern vmCvar_t mp_objectiveMaxSize;            // ?mp_objectiveMaxSize@@3UvmCvar_t@@A (g.o)
extern vmCvar_t mp_objectiveNearAlpha;          // ?mp_objectiveNearAlpha@@3UvmCvar_t@@A (g.o)
extern vmCvar_t mp_objectiveNearAlphaDist;      // ?mp_objectiveNearAlphaDist@@3UvmCvar_t@@A (g.o)
extern vmCvar_t mp_objectiveFarAlpha;           // ?mp_objectiveFarAlpha@@3UvmCvar_t@@A (g.o)
extern vmCvar_t mp_objectiveFarAlphaDist;       // ?mp_objectiveFarAlphaDist@@3UvmCvar_t@@A (g.o)
extern vmCvar_t hud_healthOverlay_regenPauseTime;       // ?hud_healthOverlay_regenPauseTime@@3UvmCvar_t@@A (game2.o)
extern vmCvar_t hud_healthOverlay_pulseStart;           // ?hud_healthOverlay_pulseStart@@3UvmCvar_t@@A (cg.o)
extern vmCvar_t hud_healthOverlay_phaseOne_pulseDuration;      // ?hud_healthOverlay_phaseOne_pulseDuration@@3UvmCvar_t@@A (cg.o)
extern vmCvar_t hud_healthOverlay_phaseTwo_toAlphaMultiplier;  // ?hud_healthOverlay_phaseTwo_toAlphaMultiplier@@3UvmCvar_t@@A (cg.o)
extern vmCvar_t hud_healthOverlay_phaseTwo_pulseDuration;      // ?hud_healthOverlay_phaseTwo_pulseDuration@@3UvmCvar_t@@A (cg.o)
extern vmCvar_t hud_healthOverlay_phaseThree_toAlphaMultiplier;  // ?hud_healthOverlay_phaseThree_toAlphaMultiplier@@3UvmCvar_t@@A (cg.o)
extern vmCvar_t hud_healthOverlay_phaseThree_pulseDuration;      // ?hud_healthOverlay_phaseThree_pulseDuration@@3UvmCvar_t@@A (cg.o)
extern vmCvar_t hud_healthOverlay_phaseEnd_toAlpha;       // ?hud_healthOverlay_phaseEnd_toAlpha@@3UvmCvar_t@@A (cg.o)
extern vmCvar_t hud_healthOverlay_phaseEnd_pulseDuration;  // ?hud_healthOverlay_phaseEnd_pulseDuration@@3UvmCvar_t@@A (cg.o)

// Debugging / memory / AI cvars (game2.o / g.o vmCvar data)
extern vmCvar_t memory_showStatistics;             // ?memory_showStatistics@@3UvmCvar_t@@A (game2.o)
extern vmCvar_t memory_reportBrocPool;             // ?memory_reportBrocPool@@3UvmCvar_t@@A (game2.o)
extern vmCvar_t memory_reportBrocBackupStackPool;  // ?memory_reportBrocBackupStackPool@@3UvmCvar_t@@A (game2.o)
extern vmCvar_t memory_displayAepsStats;           // ?memory_displayAepsStats@@3UvmCvar_t@@A (game2.o)
extern vmCvar_t memory_reportAepsStats;            // ?memory_reportAepsStats@@3UvmCvar_t@@A (game2.o)
extern vmCvar_t memory_reportCommonPool;           // ?memory_reportCommonPool@@3UvmCvar_t@@A (game2.o)
extern vmCvar_t sound_showSoundStatForEntity;      // ?sound_showSoundStatForEntity@@3UvmCvar_t@@A (game2.o)
extern vmCvar_t sound_disableAllOtherSounds;       // ?sound_disableAllOtherSounds@@3UvmCvar_t@@A (game2.o)
extern vmCvar_t ai_showNearestNode;                // ?ai_showNearestNode@@3UvmCvar_t@@A (g.o)
extern vmCvar_t ai_showNodes;                      // ?ai_showNodes@@3UvmCvar_t@@A (g.o)
extern vmCvar_t ai_showNodesDist;                  // ?ai_showNodesDist@@3UvmCvar_t@@A (g.o)
extern vmCvar_t ai_showFriendlyChains;             // ?ai_showFriendlyChains@@3UvmCvar_t@@A (g.o)
extern int g_showNumBadPaths;           // ?g_showNumBadPaths@@3HA (game2.o)
extern int g_showLightGridDebugText;    // ?g_showLightGridDebugText@@3HA (game2.o)
extern int g_LightGridDecruftifier;     // ?g_LightGridDecruftifier@@3HA (render.o)
extern int g_showLightGridDistribution; // ?g_showLightGridDistribution@@3HA (game2.o)
extern int g_lightGridBlueErrors;       // ?g_lightGridBlueErrors@@3HA (game2.o)
extern int g_showWeaponRange;           // ?g_showWeaponRange@@3HA (game2.o)
// Sound menu globals
extern int gAIBattleChatterDebug;              // ?gAIBattleChatterDebug@@3HA (game2.o)
extern int g_displayCurrentSounds;              // ?g_displayCurrentSounds@@3HA (game2.o)
extern int g_displayCurrentPrioritySounds;      // ?g_displayCurrentPrioritySounds@@3HA (game2.o)
extern int g_displayCurrentSoundStreamsOnly;    // ?g_displayCurrentSoundStreamsOnly@@3HA (game2.o)
extern int g_useOnScreenSoundDebugging;         // ?g_useOnScreenSoundDebugging@@3HA (game2.o)
extern int g_useOnScreenSoundPosDebugging;      // ?g_useOnScreenSoundPosDebugging@@3HA (game2.o)
extern int g_displaySoundRamUsage;              // ?g_displaySoundRamUsage@@3HA (game2.o)
extern vmCvar_t sound_debug;                    // ?sound_debug@@3UvmCvar_t@@A (g.o)
int s_reverbPresetId;        // 0xF0497C (game2.o)
int s_reverbPresetDisplay;   // 0xF04974 (game2.o)
int s_lastReverbPresetId;    // 0xF04978 (game2.o)
const char* s_reverbPresetStr[26];  // ?s_reverbPresetStr (game2.o @ 0xDD9298)
float startx;  // 0xDEF1AC
float starty;  // 0xDEF1A8
float scale;   // 0xDEF1A4
float ystep;   // 0xDEF1A0
float xstep;   // 0xDEF19C
float xpos;    // 0xDEF198
float xinc;    // 0xDEF194
float yinc;    // 0xDEF190

// Entity stats renderer globals
struct apsEffectLocal {
    int mFlags;  // +0x08
};
struct ParticleEffectLocal {
    unsigned char _pad[0x1C];
    apsEffectLocal* mEffect;  // +0x1C
    unsigned short mFlags;    // +0x32
};
ae_vector<ParticleEffectLocal*> gParticleEffectList;  // ?gParticleEffectList@@3V?$ae_vector@PAVParticleEffect@@@@A (render.o @ 0x1346474)
extern int g_DOBJF_NOT_RENDERED_LAST_FRAME;  // ?g_DOBJF_NOT_RENDERED_LAST_FRAME@@3HA (core.o)
extern unsigned nslGetMaxNumVoices();      // ?nslGetMaxNumVoices@@YAIXZ (nslCompat.o)
extern unsigned nslGetNumVoices();         // ?nslGetNumVoices@@YAIXZ (nslCompat.o)
struct nslVoice;
extern nslVoice* nslGetVoice(unsigned int a);  // ?nslGetVoice@@YAPAUnslVoice@@I@Z (nslCompat.o)
extern nslSourceState nslGetSourceState(nslSourceID sid);  // ?nslGetSourceState@@YA?AW4nslSourceState@@W4nslSourceID@@@Z
struct nslSource;
extern nslSource* nslSourcePtr(nslSourceID sid);         // ?nslSourcePtr@@YAPAUnslSource@@W4nslSourceID@@@Z
extern const char* nslGetSourceName(nslSourceID sid);  // ?nslGetSourceName@@YAPBDW4nslSourceID@@@Z (nslSource.o)
extern const char* nslGetWaveName(nslWaveID a);       // ?nslGetWaveName@@YAPBDW4nslWaveID@@@Z (nslCompat.o)
extern float nslGetSourceParam(nslSourceID sid, int index, float defaultValue);  // nslSource.o
extern int nslIsWaveStreamed(nslWaveID a);            // ?nslIsWaveStreamed@@YAHW4nslWaveID@@@Z (nslCompat.o)
extern unsigned int nsl_aramFree;   // ?nsl_aramFree@@3IA (nslAram.o)
extern unsigned int nsl_aramSize;   // ?nsl_aramSize@@3IA (nslAram.o)

// Controller / settings cvars
extern cvar_t* in_stickSouthPaw;  // ?in_stickSouthPaw@@3PAUcvar_t@@A (game2.o)
extern cvar_t* in_stickLegacy;    // ?in_stickLegacy@@3PAUcvar_t@@A (game2.o)
extern cvar_t* cl_freeze;         // ?cl_freeze@@3PAUcvar_t@@A (cl.o)
extern cvar_t* r_showSkeletons;   // ?r_showSkeletons@@3PAUcvar_t@@A (render.o)
extern cvar_t* cg_debugSpawnPoints;  // ?cg_debugSpawnPoints@@3PAUcvar_t@@A (game2.o)
extern cvar_t* joy_threshold;     // ?joy_threshold@@3PAUcvar_t@@A (game2.o)
extern cvar_t* r_showLocationalDamage;  // ?r_showLocationalDamage@@3PAUcvar_t@@A (render.o)
extern int SetVehicleDebugRender(int onoff);  // ?SetVehicleDebugRender@@YAHH@Z (game2.o)
extern void IN_Init();            // ?IN_Init@@YAXXZ (game2.o)
extern int gTakeScreenshot;       // ?gTakeScreenshot@@3HA (game2.o)
extern void TakeCubeMapShot();    // ?TakeCubeMapShot@@YAXXZ (game2.o)
extern char* va(const char* fmt, ...);        // ?va@@YAPADPBDZZ
extern void Cvar_Set(const char* var_name, const char* value);  // ?Cvar_Set@@YAXPBD0@Z
extern int Sys_Milliseconds();    // ?Sys_Milliseconds@@YAHXZ
float g_losResetTime;      // ?g_losResetTime@@3MA (game2.o)
unsigned int g_previousSysTime;  // ?g_previousSysTime@@3IA (game2.o)
unsigned int g_previousMS;       // ?g_previousMS@@3IA (game2.o)
extern int g_fps;                 // ?g_fps@@3HA (game2.o)
extern int IM_RenderGameEntityStats();    // ?IM_RenderGameEntityStats@@YAHXZ (game2.o)
extern Entity* RenderPlayerStats();       // ?RenderPlayerStats@@YAPAVEntity@@XZ (game2.o)

// PathNode / zone / audio-tick helper views (opaque owners)
// BadPathManager defined in game/actor_types.h (mp_actors.o)
BadPathManager g_badPathManager;         // ?g_badPathManager@@3VBadPathManager@@A (mp_actors.o)
struct PathNodeLevelTOC {
    int mNodeCount;  // +0x00
};
extern PathNodes::PathNode* __fastcall Sentient_NearestNode(
    sentient_s* pSelf, float (*const vNormal)[2], float* const fDist,
    int iPlaneCount, int iCheckDontLink, float distanceThreshold,
    int ignoreNegotiationBegin);  // ?Sentient_NearestNode@@YIPAUPathNode@PathNodes@@PAUsentient_s@@QAY01MQAMHHMH@Z (mp_actors.o)

// Ocean shader debug globals (render_xboxr:cdOceanShaderDebug.o)
extern int g_oceanDebug_Enable;            // ?g_oceanDebug_Enable@@3HA
extern int g_oceanDebug_BankID;            // ?g_oceanDebug_BankID@@3HA
extern int g_oceanDebug_DumpSettings;      // ?g_oceanDebug_DumpSettings@@3HA
extern int g_oceanDebug_Layer2Enable;      // ?g_oceanDebug_Layer2Enable@@3HA
extern int g_oceanDebug_Layer3Enable;      // ?g_oceanDebug_Layer3Enable@@3HA
extern int g_oceanDebug_LightmapEnable;    // ?g_oceanDebug_LightmapEnable@@3HA
float g_oceanDebug_Layer2Alpha;     // ?g_oceanDebug_Layer2Alpha@@3MA
float g_oceanDebug_Layer3Alpha;     // ?g_oceanDebug_Layer3Alpha@@3MA
float g_oceanDebug_SeaLevel;        // ?g_oceanDebug_SeaLevel@@3MA
float g_oceanDebug_UVScale[8];      // ?g_oceanDebug_UVScale@@3PAY01MA (2 per layer)
float g_oceanDebug_UVScroll[8];     // ?g_oceanDebug_UVScroll@@3PAY01MA (2 per layer)
float g_oceanDebug_Origin[8];       // ?g_oceanDebug_Origin@@3PAY01MA (2 per wave)
float g_oceanDebug_Distance[4];     // ?g_oceanDebug_Distance@@3PAMA
float g_oceanDebug_Heading[4];      // ?g_oceanDebug_Heading@@3PAMA
float g_oceanDebug_Wavelength[4];   // ?g_oceanDebug_Wavelength@@3PAMA
float g_oceanDebug_Amplitude[4];    // ?g_oceanDebug_Amplitude@@3PAMA
float g_oceanDebug_Phase[4];        // ?g_oceanDebug_Phase@@3PAMA
float g_oceanDebug_Timescale[4];    // ?g_oceanDebug_Timescale@@3PAMA

// cdSimpleAlpha shader debug state (render_xboxr:cdSimpleAlphaDebug.o)
struct cdSimpleAlphaDebug {
    int mEnable;          // +0x00
    int mBlendMode;       // +0x04
    int mBackface;        // +0x08
    int mEnableZPass;     // +0x0C
    int mEnableTint;      // +0x10
    int mEnablePulsing;   // +0x14
    float mTint[4];       // +0x18
    float mAlphaCutoff;   // +0x28
    float mPulseRate;     // +0x2C
    float mMinTint;       // +0x30
    float mMaxTint;       // +0x34
    float mMinAlpha;      // +0x38
    float mMaxAlpha;      // +0x3C
};
static_assert(sizeof(cdSimpleAlphaDebug) == 0x40,
              "cdSimpleAlphaDebug size mismatch");
cdSimpleAlphaDebug g_cdSimpleAlphaDebug;  // ?g_cdSimpleAlphaDebug@@3UcdSimpleAlphaDebug@@A (render.o)

// ============================================================================
// InspectorManager::AddAimAssistMenus - ea: 0x4F0C00
// ============================================================================
void InspectorManager::AddAimAssistMenus(_INSPECTOR_MENU* parent)
{
    _INSPECTOR_MENU* v3 = AddSubMenu(parent, "Controls/Aim Assist");
    AddItem(v3, "Render Sicky Box", &bg_stickyAimRender->integer, 2);
    AddItem(v3, "Sticky Easy", &gStickyBaseSlowFactorEasy, 9);
    AddItem(v3, "Sticky Norm", &gStickyBaseSlowFactorNormal, 9);
    AddItem(v3, "Sticky Hard", &gStickyBaseSlowFactorHard, 9);
    AddItem(v3, "Sticky Scale Easy", &gStickyBoxScaleEasy, 0x40007);
    AddItem(v3, "Sticky Scale Norm", &gStickyBoxScaleNormal, 0x40007);
    AddItem(v3, "Sticky Scale Hard", &gStickyBoxScaleHard, 0x40007);
    AddItem(v3, "Extra Distance Sticky", &gExtraDistanceSticky, 9);
    AddItem(v3, "Extra Strafe Sticky", &gExtraStrafeSticky, 9);
    AddItem(v3, "Aim Accel Rate", &gLookAccelRate, 0x40003);
    AddItem(v3, "Max Turn Speed", &gMaxTurnSpeed, 0x40003);
    AddItem(v3, "Base Sensitivity Horizontal",
            &gSaveGameData[0].mStubData.mHorizontalSensitivity, 0x40001);
    AddItem(v3, "Base Sensitivity Vertical",
            &gSaveGameData[0].mStubData.mVerticalSensitivity, 0x40001);
}

// ============================================================================
// InspectorManager::AddCollisionMenus - ea: 0x4F0D40
// ============================================================================
void InspectorManager::AddCollisionMenus()
{
    _INSPECTOR_MENU* v2 = AddSubMenu(nullptr, "Collision");
    AddItem(v2, "Render Collision Geometry", (void*)ToggleRenderGeom, 17);
    AddItem(v2, "Render Performance Graph", (void*)ToggleGraph, 17);
    AddItem(v2, "Render Performance Geometry", (void*)ToggleRenderPerf, 17);
    AddItem(v2, "Render World", &gRenderWorld, 2);
    AddItem(v2, "Zoom In Perf Graph", (void*)ZoomIn, 17);
    AddItem(v2, "Zoom Out Perf Graph", (void*)ZoomOut, 17);
    AddItem(v2, "Physics Object Finder", &gPhysicsFinder, 2);
    AddItem(v2, "Trace", &gDebugTrace, 2);
    AddItem(v2, "Bullet Trace", &gDebugLocationalTrace, 2);
}

// ============================================================================
// AddSoundMenus - ea: 0x4F4860
// ============================================================================
void AddSoundMenus(InspectorManager* inspectorMan)
{
    unsigned char* mCurrentReverb = (unsigned char*)SoundDevice::sInst + 0x7890;
    _INSPECTOR_MENU* pMenu = inspectorMan->AddSubMenu(nullptr, "Sound");
    inspectorMan->AddItem(pMenu, "Debug Battlechatter", &gAIBattleChatterDebug, 2);
    inspectorMan->AddItem(pMenu, "Draw Sound Overlay", &g_displayCurrentSounds, 2);
    inspectorMan->AddItem(pMenu, "Draw Sound Priority Overlay",
                          &g_displayCurrentPrioritySounds, 2);
    inspectorMan->AddItem(pMenu, "Draw Streams Only",
                          &g_displayCurrentSoundStreamsOnly, 2);
    inspectorMan->AddItem(pMenu, "On screen sound debugging",
                          &g_useOnScreenSoundDebugging, 2);
    inspectorMan->AddItem(pMenu, "Positional sound debugging",
                          &g_useOnScreenSoundPosDebugging, 2);
    inspectorMan->AddItem(pMenu, "Script sound debugging", &sound_debug.integer, 2);
    inspectorMan->AddItem(pMenu, "RAM Usage", &g_displaySoundRamUsage, 2);
    _INSPECTOR_MENU* v2 = inspectorMan->AddSubMenu(pMenu, "Reverb Parameters");
    inspectorMan->AddItem(v2, "Use Preset", &s_reverbPresetId, 0x20001);
    inspectorMan->AddItem(v2, "Room               ", mCurrentReverb, 1);
    inspectorMan->AddItem(v2, "Room HF            ", mCurrentReverb + 4, 1);
    inspectorMan->AddItem(v2, "Room Rolloff Factor", mCurrentReverb + 8, 7);
    inspectorMan->AddItem(v2, "Decay HF Ratio     ", mCurrentReverb + 16, 7);
    inspectorMan->AddItem(v2, "Reflections        ", mCurrentReverb + 20, 1);
    inspectorMan->AddItem(v2, "Reflections Delay  ", mCurrentReverb + 24, 7);
    inspectorMan->AddItem(v2, "Reverb             ", mCurrentReverb + 28, 1);
    inspectorMan->AddItem(v2, "Reverb Delay       ", mCurrentReverb + 32, 7);
    inspectorMan->AddItem(v2, "Diffusion          ", mCurrentReverb + 36, 4);
    inspectorMan->AddItem(v2, "Density            ", mCurrentReverb + 40, 4);
    inspectorMan->AddItem(v2, "HF Reference       ", mCurrentReverb + 44, 4);
    inspectorMan->AddSubMenu(pMenu, "Veh Parameters");
}

// ============================================================================
// SoundDebugRender - ea: 0x4F4A40
// ============================================================================
void SoundDebugRender(InspectorManager* inspectorMan)
{
    char tmpstr[512];
    int v1 = s_reverbPresetId;
    if (s_reverbPresetId >= 26)
    {
        if (s_reverbPresetId >= 0)
        {
            v1 = 25;
            goto preset_clamped;
        }
    preset_underflow:
        v1 = 0;
    preset_clamped:
        s_reverbPresetId = v1;
        goto preset_done;
    }
    if (s_reverbPresetId < 0)
        goto preset_underflow;
    if (s_reverbPresetDisplay != 0)
    {
        sprintf(tmpstr, "Reverb = %s", s_reverbPresetStr[s_reverbPresetId]);
        inspectorMan->Print(tmpstr, 300, 400, 0.55f);
        if (s_reverbPresetDisplay == 90)
            SoundDevice::sInst->SetReverb(
                s_reverbPresetStr[s_reverbPresetId], true);
        --s_reverbPresetDisplay;
        v1 = s_reverbPresetId;
    }
preset_done:
    if (s_lastReverbPresetId != v1)
    {
        s_lastReverbPresetId = v1;
        s_reverbPresetDisplay = 90;
    }
    if (g_displaySoundRamUsage != 0)
    {
        sprintf(tmpstr, "RAM Free %d\nRAM Used %d\nTotal RAM %d",
                nsl_aramFree, nsl_aramSize - nsl_aramFree, nsl_aramSize);
        inspectorMan->Print(tmpstr, 52, 400, 0.55f);
    }
    if (g_displayCurrentSounds != 0)
    {
        int xPos = (int)startx;
        int yPos = (int)starty;
        int count = 0;
        nslGetMaxNumVoices();
        unsigned int NumVoices = nslGetNumVoices();
        unsigned int currentNumVoices = NumVoices;
        unsigned int voicesPlaying = 0;
        sprintf(tmpstr, "Sound Overlay (SoundDevice-AudioSlots)");
        inspectorMan->Print(tmpstr, (int)startx, 30, 0.55f);
        inspectorMan->m_currentRgba[0] = 1.0f;
        inspectorMan->m_currentRgba[1] = 1.0f;
        inspectorMan->m_currentRgba[2] = 1.0f;
        inspectorMan->m_currentRgba[3] = 1.0f;
        unsigned int i = 0;
        if (NumVoices != 0)
        {
            do
            {
                unsigned char* Voice = (unsigned char*)nslGetVoice(i);
                if (*(Voice + 0x108) != 0)
                {
                    nslSourceID v5 = *(nslSourceID*)(Voice + 0x114);
                    nslWaveID v6 = *(nslWaveID*)(Voice + 0x110);
                    nslSourceState state = nslGetSourceState(v5);
                    nslSourcePtr(v5);
                    const char* sourceName = nslGetSourceName(v5);
                    nslGetWaveName(v6);
                    nslGetSourceParam(v5, 0, -1.0f);
                    bool v7 = nslIsWaveStreamed(v6) != 0;
                    if (g_displayCurrentSoundStreamsOnly == 0 || v7)
                    {
                        if (state != NSL_SOURCE_STATE_INVALID)
                        {
                            sprintf(tmpstr, "%d)%s", i, sourceName);
                            int v8 = yPos;
                            inspectorMan->Print(tmpstr, xPos, yPos, scale);
                            ++voicesPlaying;
                            yPos = (int)ystep + v8;
                            ++count;
                        }
                        else
                        {
                            sprintf(tmpstr, "%d) empty", i);
                        }
                        if (count == 24)
                        {
                            yPos = (int)starty;
                            xPos += (int)xstep;
                        }
                    }
                }
                i++;
            } while (i < currentNumVoices);
        }
        sprintf(tmpstr, "NUMBER OF VOICES PLAYING [%d]", voicesPlaying);
        float v11[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
        int v9 = (int)startx;
        RE_Text_Paint(startx + 2.0f, 22.0f, 5, scaleScalar * 0.55f, v11,
                      tmpstr, 0, 0, 0);
        RE_Text_Paint((float)v9, 20.0f, 5, scaleScalar * 0.55f,
                      inspectorMan->m_currentRgba, tmpstr, 0, 0, 0);
    }
}

// ============================================================================
// InspectorManager::AddPlayerMenus - ea: 0x4F7880
// ============================================================================
void InspectorManager::AddPlayerMenus()
{
    _INSPECTOR_MENU* v2 = AddSubMenu(nullptr, "Player Settings");
    AddItem(v2, "God Mode", (void*)FN_SelectGodMode, 17);
    AddItem(v2, "NoClip", (void*)FN_NoClip, 17);
    AddItem(v2, "Player Speed", &g_speed.integer, 1);
    AddItem(v2, "Draw Player Position", &g_displayPlayerPosition, 2);
}

// ============================================================================
// InspectorManager::AddDesignerMenus - ea: 0x4F78F0
// ============================================================================
void InspectorManager::AddDesignerMenus()
{
    cvar_t* v2 = Cvar_Get("timescale", "1.0", 256);
    _INSPECTOR_MENU* v3 = AddSubMenu(nullptr, "Designer Support");
    AddItem(v3, "Draw FPS", &g_renderFPS, 2);
    AddItem(v3, "Time Scaling", &v2->value, 0x40007);
    AddItem(v3, "Show Path Node Number", &g_showPathNodeDensity, 2);
    AddItem(v3, "Entity Stats", &g_renderGameEntityStats, 2);
    AddItem(v3, "PFX Stats", &g_renderPFXStats, 2);
    AddItem(v3, "show culled particles", &g_showCulledParticles, 2);
    AddItem(v3, "Pak Render", (void*)FN_PakRender, 17);
    AddItem(v3, "NGL Stat Display", (void*)FN_NGLStatDisplay, 17);
    AddItem(v3, "Render ALL Los Calls", &g_drawDebugLos, 2);
    AddItem(v3, "Thread Particle system", &gThreadedParticles, 2);
}

// ============================================================================
// InspectorManager::AddFXMenus - ea: 0x4F79F0
// ============================================================================
void InspectorManager::AddFXMenus()
{
    _INSPECTOR_MENU* v2 = AddSubMenu(nullptr, "FX Settings/Switching");
    _INSPECTOR_MENU* v3 = AddSubMenu(v2, "FX on/off");
    AddItem(v3, "Don't Play Foot Steps", &gSoundOptions.mFxDontPlayFootSteps, 2);
    AddItem(v3, "Don't Play Gear Rattle",
            &gSoundOptions.mFxDontPlayGearRattle, 2);
    AddItem(v3, "Don't Play Landing", &gSoundOptions.mFxDontPlayLanding, 2);
    AddItem(v3, "Don't Play Script Call",
            &gSoundOptions.mFxDontPlayScriptCall, 2);
    AddItem(v3, "Don't Play Script Call(Dir)",
            &gSoundOptions.mFxDontPlayScriptCall_Dir, 2);
    AddItem(v3, "Don't Play Weapon", &gSoundOptions.mFxDontPlayWeapon, 2);
    AddItem(v3, "Don't Play Bullet Hit",
            &gSoundOptions.mFxDontPlayBulletHit, 2);
    AddItem(v3, "Don't Play Grenade Bounce",
            &gSoundOptions.mFxDontPlayGrenadeBounce, 2);
    AddItem(v3, "Don't Play Proj Explode",
            &gSoundOptions.mFxDontPlayProjExplode, 2);
    AddItem(v3, "Don't Play Vehicle", &gSoundOptions.mFxDontPlayVehicle, 2);
    AddItem(v3, "Don't Play Turret", &gSoundOptions.mFxDontPlayTurret, 2);
    AddItem(v3, "Don't Play Vehicle Wheel",
            &gSoundOptions.mFxDontPlayVehicleWheel, 2);
    AddItem(v3, "Don't Play Light Flash",
            &gSoundOptions.mFxDontPlayLightFlash, 2);
    AddItem(v3, "Don't Play Music", &gSoundOptions.mFxDontPlayMusic, 2);
    AddItem(v3, "Reverse Options", (void*)FnReverseOptions, 17);
    _INSPECTOR_MENU* v4 = AddSubMenu(v2, "CG Settings");
    AddItem(v4, "Camera Shake", &cg_camerashake.integer, 2);
    AddItem(v4, "ShellShock Blur", &cg_shellshockblur.integer, 2);
    _INSPECTOR_MENU* v5 = AddSubMenu(v2, "Motion Blur");
    AddItem(v5, "MOTION BLUR Alpha VALUE", &g_myBlurValue, 7);
    AddItem(v5, "MOTION BLUR Red VALUE", &g_myRValue, 7);
    AddItem(v5, "MOTION BLUR Green VALUE", &g_myGValue, 7);
    AddItem(v5, "MOTION BLUR Blue VALUE", &g_myBValue, 7);
    AddItem(v5, "MOTION BLUR Blend Method", &g_blendType, 0x20001);
    AddItem(v5, "Far Near Radius", &gNearLightRadius, 0x40007);
    AddItem(v5, "Far Light Radius", &gFarLightRadius, 0x40007);
    _INSPECTOR_MENU* v6 = AddSubMenu(
        v2, "Sky Bloom and Jesus Rays (level.skybloom.csv)");
    AddItem(v6, "Glow ON/OFF", &g_GlowEnable, 2);
    AddItem(v6, "Glow God Rays ON/OFF", &g_GlowGodRaysEnable, 2);
    AddItem(v6, "Glow Intensity", &g_GlowIntensity, 10);
    AddItem(v6, "Glow Expansion", &g_GlowExpansion, 10);
    AddItem(v6, "Glow Brightness", &g_GlowBrightness, 10);
    AddItem(v6, "Glow Passes", &g_GlowPasses, 0x60001);
    _INSPECTOR_MENU* v7 = AddSubMenu(v2, "Marcus");
    AddItem(v7, "Render Spheres", &g_renderSphere, 2);
    AddItem(v7, "Limit Visual Range", &g_limitVisualRange, 2);
    AddItem(v7, "Tank Tracks", &g_tankTracks, 0x40007);
    AddItem(v7, "Tank Wheels", &g_tankWheels, 0x40007);
}

// ============================================================================
// InspectorManager::AddVehicleMenus - ea: 0x4F7D60
// ============================================================================
void InspectorManager::AddVehicleMenus(_INSPECTOR_MENU* parent)
{
    _INSPECTOR_MENU* v3 = AddSubMenu(parent, "Vehicle Settings");
    AddItem(v3, "Debug Render All", &rb_vehicle::sRenderAllVehicles, 2);
    AddItem(v3, "Debug Render", (void*)SetVehicleDebugRender, 22);
    AddItem(v3, "Debug Render Entry Points",
            &scr_vehicle_t::sRenderEntryPoints, 2);
    AddItem(v3, "Debug Vehicle Anims", &scr_vehicle_t::sDebugAnims, 2);
    AddItem(v3, "Debug Vehicle Mantle", &scr_vehicle_t::sDebugMantle, 2);
    _INSPECTOR_MENU* v4 = AddSubMenu(v3, "Vehicle Physics");
    AddItem(v4, "speed_max", (void*)speed_max_Function, 24);
    AddItem(v4, "accel_max", (void*)accel_max_Function, 24);
    AddItem(v4, "reverse_scale", (void*)reverse_scale_Function, 24);
    AddItem(v4, "steer_angle_max", (void*)steer_angle_max_Function, 24);
    AddItem(v4, "steer_speed", (void*)steer_speed_Function, 24);
    AddItem(v4, "wheel_radius", (void*)wheel_radius_Function, 24);
    AddItem(v4, "susp_spring_k", (void*)susp_spring_k_Function, 24);
    AddItem(v4, "susp_damp_k", (void*)susp_damp_k_Function, 24);
    AddItem(v4, "susp_adj", (void*)susp_adj_Function, 24);
    AddItem(v4, "susp_hard_limit", (void*)susp_hard_limit_Function, 24);
    AddItem(v4, "tire_fric_fwd", (void*)tire_fric_fwd_Function, 24);
    AddItem(v4, "tire_fric_side", (void*)tire_fric_side_Function, 24);
    AddItem(v4, "tire_fric_brake", (void*)tire_fric_brake_Function, 24);
    AddItem(v4, "tire_fric_hand_brake", (void*)tire_fric_hand_brake_Function, 24);
    AddItem(v4, "body_mass", (void*)body_mass_Function, 24);
    AddItem(v4, "mass_center_delta_x", (void*)mass_center_delta_x_Function, 24);
    AddItem(v4, "mass_center_delta_y", (void*)mass_center_delta_y_Function, 24);
    AddItem(v4, "mass_center_delta_z", (void*)mass_center_delta_z_Function, 24);
    AddItem(v4, "roll_stability", (void*)roll_stability_Function, 24);
    AddItem(v4, "roll_resistance", (void*)roll_resistance_Function, 24);
    AddItem(v4, "upright_strength", (void*)upright_strength_Function, 24);
    AddItem(v4, "tilt_fakey", (void*)tilt_fakey_Function, 24);
    AddItem(v4, "peel_out_max_speed", (void*)peel_out_max_speed_Function, 24);
    AddItem(v4, "inertia_scale_x", (void*)inertia_scale_x_Function, 24);
    AddItem(v4, "tire_damp_coast", (void*)tire_damp_coast_Function, 24);
    AddItem(v4, "tire_damp_brake", (void*)tire_damp_brake_Function, 24);
    AddItem(v4, "tire_damp_hand", (void*)tire_damp_hand_Function, 24);
    _INSPECTOR_MENU* v5 = AddSubMenu(v3, "Vehicle Physics2");
    AddItem(v5, "BBoxMinX", (void*)SetVehicleInertiaBoxMinX, 24);
    AddItem(v5, "BBoxMinY", (void*)SetVehicleInertiaBoxMinY, 24);
    AddItem(v5, "BBoxMinZ", (void*)SetVehicleInertiaBoxMinZ, 24);
    AddItem(v5, "BBoxMaxX", (void*)SetVehicleInertiaBoxMaxX, 24);
    AddItem(v5, "BBoxMaxY", (void*)SetVehicleInertiaBoxMaxY, 24);
    AddItem(v5, "BBoxMaxZ", (void*)SetVehicleInertiaBoxMaxZ, 24);
    _INSPECTOR_MENU* v6 = AddSubMenu(v3, "Vehicle GDE");
    _INSPECTOR_MENU* v7 = AddSubMenu(v6, "Misc Settings");
    _INSPECTOR_MENU* movementMenu = AddSubMenu(v6, "Movement Settings");
    _INSPECTOR_MENU* turretMenu = AddSubMenu(v6, "Turret Settings");
    _INSPECTOR_MENU* v8 = AddSubMenu(v6, "Camera Settings");
    AddItem(v7, "steerWheels", (void*)vehicleFuncs::steerWheels_Function, 23);
    AddItem(v7, "quadBarrel", (void*)vehicleFuncs::quadBarrel_Function, 23);
    AddItem(v7, "bulletDamage", (void*)vehicleFuncs::bulletDamage_Function, 24);
    AddItem(v7, "grenadeDamage", (void*)vehicleFuncs::grenadeDamage_Function, 24);
    AddItem(v7, "mineDamage", (void*)vehicleFuncs::mineDamage_Function, 24);
    AddItem(v7, "projectileDamage", (void*)vehicleFuncs::projectileDamage_Function, 24);
    AddItem(v7, "spClientSeat", (void*)vehicleFuncs::spClientSeat_Function, 22);
    AddItem(v7, "hudIndex", (void*)vehicleFuncs::hudIndex_Function, 22);
    AddItem(v7, "numSeats", (void*)vehicleFuncs::numSeats_Function, 22);
    AddItem(v7, "health", (void*)vehicleFuncs::health_Function, 22);
    AddItem(v7, "texureScroll", (void*)vehicleFuncs::texureScroll_Function, 24);
    AddItem(v7, "texureScrollScale", (void*)vehicleFuncs::texureScrollScale_Function, 24);
    AddItem(v7, "engineSndSpeed", (void*)vehicleFuncs::engineSndSpeed_Function, 24);
    AddItem(movementMenu, "maxSpeed", (void*)vehicleFuncs::maxSpeed_Function, 24);
    AddItem(movementMenu, "accel", (void*)vehicleFuncs::accel_Function, 24);
    AddItem(movementMenu, "rotRate", (void*)vehicleFuncs::rotRate_Function, 24);
    AddItem(movementMenu, "rotAccel", (void*)vehicleFuncs::rotAccel_Function, 24);
    AddItem(movementMenu, "collisionDamage", (void*)vehicleFuncs::collisionDamage_Function, 24);
    AddItem(movementMenu, "collisionSpeed", (void*)vehicleFuncs::collisionSpeed_Function, 24);
    AddItem(movementMenu, "suspensionTravel", (void*)vehicleFuncs::suspensionTravel_Function, 24);
    AddItem(movementMenu, "maxBodyPitch", (void*)vehicleFuncs::maxBodyPitch_Function, 24);
    AddItem(movementMenu, "maxBodyRoll", (void*)vehicleFuncs::maxBodyRoll_Function, 24);
    AddItem(movementMenu, "boundsRadius", (void*)vehicleFuncs::boundsRadius_Function, 24);
    AddItem(movementMenu, "boundsHeight", (void*)vehicleFuncs::boundsHeight_Function, 24);
    AddItem(movementMenu, "boundsLength", (void*)vehicleFuncs::boundsLength_Function, 24);
    AddItem(turretMenu, "turretHorizSpanLeft", (void*)vehicleFuncs::turretHorizSpanLeft_Function, 24);
    AddItem(turretMenu, "turretHorizSpanRight", (void*)vehicleFuncs::turretHorizSpanRight_Function, 24);
    AddItem(turretMenu, "turretVertSpanUp", (void*)vehicleFuncs::turretVertSpanUp_Function, 24);
    AddItem(turretMenu, "turretVertSpanDown", (void*)vehicleFuncs::turretVertSpanDown_Function, 24);
    AddItem(turretMenu, "turretRotRate", (void*)vehicleFuncs::turretRotRate_Function, 24);
    AddItem(turretMenu, "turretSwirlLerpRate", (void*)vehicleFuncs::turretSwirlLerpRate_Function, 24);
    AddItem(turretMenu, "turretSwirlPitchFactor", (void*)vehicleFuncs::turretSwirlPitchFactor_Function, 24);
    AddItem(turretMenu, "turretGunnerVertSpanUp", (void*)vehicleFuncs::turretGunnerVertSpanUp_Function, 24);
    AddItem(turretMenu, "turretGunnerVertSpanDown", (void*)vehicleFuncs::turretGunnerVertSpanDown_Function, 24);
    AddItem(v8, "cameraZOffset", (void*)vehicleFuncs::cameraZOffset_Function, 24);
    AddItem(v8, "cameraFPHeightOffset", (void*)vehicleFuncs::cameraFPHeightOffset_Function, 24);
    AddItem(v8, "cameraFPFwdOffset", (void*)vehicleFuncs::cameraFPFwdOffset_Function, 24);
    AddItem(v8, "cameraFPHeightLerp", (void*)vehicleFuncs::cameraFPHeightLerp_Function, 24);
    AddItem(v8, "cameraChaseOffsetX", (void*)vehicleFuncs::cameraChaseOffsetX_Function, 24);
    AddItem(v8, "cameraChaseOffsetY", (void*)vehicleFuncs::cameraChaseOffsetY_Function, 24);
    AddItem(v8, "cameraChaseOffsetZ", (void*)vehicleFuncs::cameraChaseOffsetZ_Function, 24);
    AddItem(v8, "cameraChaseRadiusInner", (void*)vehicleFuncs::cameraChaseRadiusInner_Function, 24);
    AddItem(v8, "cameraChaseRadiusOuter", (void*)vehicleFuncs::cameraChaseRadiusOuter_Function, 24);
    AddItem(v8, "cameraVehViewRadius", (void*)vehicleFuncs::cameraVehViewRadius_Function, 24);
    AddItem(v8, "cameraVehViewMaxPitch", (void*)vehicleFuncs::cameraVehViewMaxPitch_Function, 24);
    AddItem(v8, "cameraVehViewMaxPitchDistAdj", (void*)vehicleFuncs::cameraVehViewMaxPitchDistAdj_Function, 24);
    AddItem(v8, "cameraVehViewFwdBackRatio", (void*)vehicleFuncs::cameraVehViewFwdBackRatio_Function, 24);
    AddItem(v8, "cameraVehViewMoveInPitch", (void*)vehicleFuncs::cameraVehViewMoveInPitch_Function, 24);
    AddItem(v8, "camLinkedPitchFactor", (void*)vehicleFuncs::camLinkedPitchFactor_Function, 24);
    AddItem(v8, "pitchBasedCamOffsetX", (void*)vehicleFuncs::pitchBasedCamOffsetX_Function, 24);
    AddItem(v8, "pitchBasedCamOffsetZ", (void*)vehicleFuncs::pitchBasedCamOffsetZ_Function, 24);
    AddItem(v7, "vehicleAnimMatrixColumn", (void*)vehicleFuncs::vehicleAnimMatrixColumn_Function, 22);
    AddItem(v8, "hatchOpenAngleRight", (void*)vehicleFuncs::hatchOpenAngleRight_Function, 24);
    AddItem(v8, "hatchOpenAngleLeft", (void*)vehicleFuncs::hatchOpenAngleLeft_Function, 24);
    AddItem(v7, "inactiveBlowupSeconds", (void*)vehicleFuncs::inactiveBlowupSeconds_Function, 22);
}

// ============================================================================
// InspectorManager::AddMultiplayerMenus - ea: 0x4F8570
// ============================================================================
void InspectorManager::AddMultiplayerMenus()
{
    _INSPECTOR_MENU* v2 = AddSubMenu(nullptr, "Multi-Player");
    _INSPECTOR_MENU* v3 = AddSubMenu(v2, "Debug Render");
    AddItem(v3, "General Vehicle", &MPVehicle::sDebugGeneral, 2);
    AddItem(v3, "Vehicle Network Updates", &MPVehicle::sDebugNetworkUpdates, 2);
    AddItem(v3, "Pause Vehicle Network Updates", &MPVehicle::sPauseNetworkUpdates, 2);
    AddItem(v3, "Player Network Updates", &MPPlayer::sDebugNetworkUpdates, 2);
    AddItem(v3, "Pause Player Network Updates", &MPPlayer::sPauseNetworkUpdates, 2);
    AddItem(v3, "Session Status", &MPPeer::mRenderSessionInfo, 2);
    AddItem(v3, "Player Status", &MPPeer::mRenderPlayerInfo, 2);
    AddItem(v3, "Data Status", &MPPeer::mRenderDataInfo, 2);
    AddItem(v3, "Debug Anim Entity", &cg_mpDebugAnimEntity, 0x20001);
    AddItem(v3, "Third Person Render", &cg_thirdPerson.integer, 2);
    AddItem(v3, "Third Person Range", &cg_thirdPersonRange.value, 0x40003);
    AddItem(v3, "Third Person Angles", &cg_thirdPersonAngle.value, 13);
    AddItem(v3, "Third Person Lock", &cg_thirdPersonLock.integer, 2);
    r_showSkeletons = Cvar_Get("r_showSkeletons", "0", 512);
    AddItem(v3, "Draw Skeleton", &r_showSkeletons->integer, 2);
    cvar_t* v4 = Cvar_Get("mp_debugrender", "0", 256);
    AddItem(v3, "Debug Render General", &v4->integer, 2);
    cvar_t* v5 = Cvar_Get("mp_debugrenderspawnpoints", "0", 256);
    AddItem(v3, "Debug Spawn Points", &v5->integer, 2);
    AddItem(v3, "Debug Stats", &g_displayPlayerStats, 2);
    AddItem(v2, "Map Restart", (void*)FN_Multiplayer_MapRestart, 17);
    AddItem(v2, "Rank 1", (void*)FN_Multiplayer_Rank1, 17);
    AddItem(v2, "Rank 2", (void*)FN_Multiplayer_Rank2, 17);
    AddItem(v2, "Rank 3", (void*)FN_Multiplayer_Rank3, 17);
    cvar_t* v6 = Cvar_Get("mp_debug", "0", 256);
    AddItem(v2, "Debug Script", &v6->integer, 2);
    cg_debugSpawnPoints = Cvar_Get("cg_debugSpawnPoints", "0", 512);
    AddItem(v2, "Test Spawn\tPoints", &cg_debugSpawnPoints->integer, 2);
    cvar_t* v7 = Cvar_Get("ik_ADS", "0", 256);
    AddItem(v2, "Enable IK ADS", &v7->integer, 2);
    _INSPECTOR_MENU* v8 = AddSubMenu(v2, "Head Icons");
    AddItem(v8, "Icon Height", &mp_headIconHeight.integer, 0x40001);
    AddItem(v8, "Min Screen Size", &mp_headIconMinScreenSize.integer, 0x40001);
    AddItem(v8, "Dist. Above Player", &mp_headIconDistAbovePlayer.integer, 0x40001);
    AddItem(v8, "Dist. Above Vehicle", &mp_headIconDistAboveVehicle.integer, 0x40001);
    AddItem(v8, "Min Alpha Distance", &mp_headIconReviveMinAlphaDist.integer, 0x40001);
    AddItem(v8, "Max Alpha Distance", &mp_headIconReviveMaxAlphaDist.integer, 0x40001);
    _INSPECTOR_MENU* v9 = AddSubMenu(v2, "Ammo Icons");
    AddItem(v9, "Icon Height", &mp_itemIconHeight.integer, 0x40001);
    AddItem(v9, "Min Screen Size", &mp_itemIconMinScreenSize.integer, 0x40001);
    AddItem(v9, "Dist. Above Item", &mp_itemIconDistAboveItem.integer, 0x40001);
    AddItem(v9, "Min Alpha Distance", &mp_itemIconMinAlphaDist.integer, 0x40001);
    AddItem(v9, "Max Alpha Distance", &mp_itemIconMaxAlphaDist.integer, 0x40001);
    _INSPECTOR_MENU* v10 = AddSubMenu(v2, "3D Objective Indicator");
    AddItem(v10, "World Size", &mp_objectiveSize.integer, 0x40001);
    AddItem(v10, "Min Screen Size", &mp_objectiveMinSize.integer, 0x40001);
    AddItem(v10, "Max Screen Size", &mp_objectiveMaxSize.integer, 0x40001);
    AddItem(v10, "Near Alpha", &mp_objectiveNearAlpha.integer, 0x40001);
    AddItem(v10, "Near Alpha Dist", &mp_objectiveNearAlphaDist.integer, 0x40001);
    AddItem(v10, "Far Alpha", &mp_objectiveFarAlpha.integer, 0x40001);
    AddItem(v10, "Far Alpha Dist", &mp_objectiveFarAlphaDist.integer, 0x40001);
    _INSPECTOR_MENU* v11 = AddSubMenu(v2, "Damage Overlay");
    AddItem(v11, "Regen Pause Time", &hud_healthOverlay_regenPauseTime.integer, 0x40001);
    AddItem(v11, "Pulse Start", &hud_healthOverlay_pulseStart.value, 9);
    AddItem(v11, "Pulse One Duration", &hud_healthOverlay_phaseOne_pulseDuration.integer, 0x40001);
    AddItem(v11, "Pulse Two Alpha Mult.", &hud_healthOverlay_phaseTwo_toAlphaMultiplier.value, 9);
    AddItem(v11, "Pulse Two Duration", &hud_healthOverlay_phaseTwo_pulseDuration.integer, 0x40001);
    AddItem(v11, "Pulse Three Alpha Mult.", &hud_healthOverlay_phaseThree_toAlphaMultiplier.value, 9);
    AddItem(v11, "Pulse Three Duration", &hud_healthOverlay_phaseThree_pulseDuration.integer, 0x40001);
    AddItem(v11, "End Alpha", &hud_healthOverlay_phaseEnd_toAlpha.value, 9);
    AddItem(v11, "End Pulse Duration", &hud_healthOverlay_phaseEnd_pulseDuration.integer, 0x40001);
}

// ============================================================================
// InspectorManager::AddDebuggingMenus - ea: 0x50C420
// ============================================================================
void InspectorManager::AddDebuggingMenus()
{
    _INSPECTOR_MENU* v2 = AddSubMenu(nullptr, "Debug Stuff");
    AddItem(v2, "Test Int", &g_testInt, 2);
    AddItem(v2, "Test Int Scroll", &g_testInt, 0x20001);
    AddItem(v2, "NGL Stat Display", &nglDebug, 0x20010);
    AddItem(v2, "Lock PVS", &gLockMeshList, 2);
    AddItem(v2, "Lock PVS Flash", &gEnableMeshFlash, 2);
    _INSPECTOR_MENU* v3 = AddSubMenu(v2, "Memory");
    AddItem(v3, "Display Statistics", &memory_showStatistics.integer, 2);
    AddItem(v3, "Report gBrocPool", &memory_reportBrocPool.integer, 2);
    AddItem(v3, "Report BackupStackPool",
            &memory_reportBrocBackupStackPool.integer, 2);
    AddItem(v3, "Display Aeps Stats", &memory_displayAepsStats.integer, 2);
    AddItem(v3, "Report Aeps Stats", &memory_reportAepsStats.integer, 2);
    AddItem(v3, "Report gCommonPool", &memory_reportCommonPool.integer, 2);
    _INSPECTOR_MENU* v4 = AddSubMenu(v2, "Debug Entity");
    AddItem(v4, "Debug Player", (void*)FN_DebugThread_Select_Player, 17);
    AddItem(v4, "Debug Level", (void*)FN_DebugThread_Select_Level, 17);
    AddItem(v4, "Debug Nearest Ent", (void*)FN_DebugThread_Select_Nearest, 17);
    AddItem(v4, "Debug Nearest Trigger",
            (void*)FN_DebugThread_Select_Nearest_Trigger, 17);
    AddItem(v4, "Debug Entity UniqueIndex",
            (void*)FN_DebugThread_Select_UniqueIndex, 17);
    AddItem(v4, "Debug Nearest Vehicle",
            (void*)FN_DebugThread_Select_Nearest_Vehicle, 17);
    AddItem(v4, "Debug Target Ent", (void*)FN_DebugThread_Select_Target, 17);
    AddItem(v4, "Debug Anims Target Ent",
            (void*)FN_DebugAnims_Select_Target, 17);
    AddItem(v4, "Toggle Display", &g_debugThread.m_active, 2);
    AddItem(v4, "Toggle Threads", &g_debugThread.m_displayThreads, 2);
    AddItem(v4, "Dump Current Threads", (void*)FN_DumpThreadsForTarget, 17);
    AddItem(v4, "Dump All Threads", (void*)FN_DumpThreadsForAll, 17);
    AddItem(v4, "Toggle Sound Stat", &sound_showSoundStatForEntity.integer, 2);
    AddItem(v4, "Disable All Other Sounds", &sound_disableAllOtherSounds.integer, 2);
    AddItem(v4, "Menu Scroll Start Index", &g_debugThread.m_menuScrollStartIndex, 0x60001);
    AddItem(v4, "Debug Ent BBoxes", (void*)FN_DebugEntity_BBoxes, 17);
    AddItem(v4, "Render Debug Entity Los", &g_drawDebugEntityLos, 2);
    _INSPECTOR_MENU* v5 = AddSubMenu(v2, "Path Finding Debugging");
    AddItem(v5, "Show Nearest Node", &ai_showNearestNode.integer, 0x40001);
    AddItem(v5, "Show Nodes", &ai_showNodes.integer, 0x60001);
    AddItem(v5, "Show Nodes Distance", &ai_showNodesDist.value, 0x40003);
    AddItem(v5, "Show Friendly Chains", &ai_showFriendlyChains.integer, 0x60001);
    AddItem(v5, "Show Num Bad Paths", &g_showNumBadPaths, 2);
    _INSPECTOR_MENU* v6 = AddSubMenu(v2, "Light Grid Debugging");
    AddItem(v6, "Display Debug Text", &g_showLightGridDebugText, 2);
    AddItem(v6, "Light Grid Decruftifier", &g_LightGridDecruftifier, 2);
    AddItem(v6, "Display Grid Distribution", &g_showLightGridDistribution, 2);
    AddItem(v6, "Errors Have Blue Light", &g_lightGridBlueErrors, 2);
    _INSPECTOR_MENU* v7 = AddSubMenu(v2, "Accuracy Debugging");
    AddItem(v7, "Display Weapon Range", &g_showWeaponRange, 2);
}

// ============================================================================
// InspectorManager::AddWeaponMenus - ea: 0x50C790
// ============================================================================
void InspectorManager::AddWeaponMenus(_INSPECTOR_MENU* parent)
{
    _INSPECTOR_MENU* parentMenu = AddSubMenu(parent, "Weapons Settings");
    _INSPECTOR_MENU* aimMenu = AddSubMenu(parentMenu, "Aim Settings");
    _INSPECTOR_MENU* miscMenu = AddSubMenu(parentMenu, "Misc Settings");
    _INSPECTOR_MENU* adsMenu = AddSubMenu(parentMenu, "ADS Settings");
    _INSPECTOR_MENU* kickMenu = AddSubMenu(parentMenu, "Kick Settings");
    _INSPECTOR_MENU* adsKickMenu = AddSubMenu(parentMenu, "ADS Kick Settings");
    _INSPECTOR_MENU* swayMenu = AddSubMenu(parentMenu, "Sway Settings");
    _INSPECTOR_MENU* timingMenu = AddSubMenu(parentMenu, "Anim Timings Settings");
    _INSPECTOR_MENU* projMenu = AddSubMenu(parentMenu, "Projectile Settings");
    _INSPECTOR_MENU* animIKMenu = AddSubMenu(parentMenu, "AnimIK Settings");
    _INSPECTOR_MENU* v3 = AddSubMenu(parentMenu, "Offset Settings");
    _INSPECTOR_MENU* v4 = AddSubMenu(parentMenu, "MP Specific Settings");
    AddItem(miscMenu, "reticleCenterSize", (void*)weaponFuncs::reticleCenterSize_Function, 22);
    AddItem(miscMenu, "reticleSideSize", (void*)weaponFuncs::reticleSideSize_Function, 22);
    AddItem(miscMenu, "reticleMinOfs", (void*)weaponFuncs::reticleMinOfs_Function, 22);
    AddItem(v3, "duckedOfsF", (void*)weaponFuncs::duckedOfsF_Function, 24);
    AddItem(v3, "duckedOfsR", (void*)weaponFuncs::duckedOfsR_Function, 24);
    AddItem(v3, "duckedOfsU", (void*)weaponFuncs::duckedOfsU_Function, 24);
    AddItem(v3, "proneOfsF", (void*)weaponFuncs::proneOfsF_Function, 24);
    AddItem(v3, "proneOfsR", (void*)weaponFuncs::proneOfsR_Function, 24);
    AddItem(v3, "proneOfsU", (void*)weaponFuncs::proneOfsU_Function, 24);
    AddItem(v3, "standMoveF", (void*)weaponFuncs::standMoveF_Function, 24);
    AddItem(v3, "standMoveR", (void*)weaponFuncs::standMoveR_Function, 24);
    AddItem(v3, "standMoveU", (void*)weaponFuncs::standMoveU_Function, 24);
    AddItem(v3, "duckedMoveF", (void*)weaponFuncs::duckedMoveF_Function, 24);
    AddItem(v3, "duckedMoveR", (void*)weaponFuncs::duckedMoveR_Function, 24);
    AddItem(v3, "duckedMoveU", (void*)weaponFuncs::duckedMoveU_Function, 24);
    AddItem(v3, "proneMoveF", (void*)weaponFuncs::proneMoveF_Function, 24);
    AddItem(v3, "proneMoveR", (void*)weaponFuncs::proneMoveR_Function, 24);
    AddItem(v3, "proneMoveU", (void*)weaponFuncs::proneMoveU_Function, 24);
    AddItem(v3, "proneRotP", (void*)weaponFuncs::proneRotP_Function, 24);
    AddItem(v3, "proneRotY", (void*)weaponFuncs::proneRotY_Function, 24);
    AddItem(v3, "proneRotR", (void*)weaponFuncs::proneRotR_Function, 24);
    AddItem(v3, "standMoveMinSpeed", (void*)weaponFuncs::standMoveMinSpeed_Function, 24);
    AddItem(v3, "duckedMoveMinSpeed", (void*)weaponFuncs::duckedMoveMinSpeed_Function, 24);
    AddItem(v3, "proneMoveMinSpeed", (void*)weaponFuncs::proneMoveMinSpeed_Function, 24);
    AddItem(v3, "posProneRotRate", (void*)weaponFuncs::posProneRotRate_Function, 24);
    AddItem(v3, "proneRotMinSpeed", (void*)weaponFuncs::proneRotMinSpeed_Function, 24);
    AddItem(miscMenu, "damage", (void*)weaponFuncs::damage_Function, 22);
    AddItem(miscMenu, "meleeDamage", (void*)weaponFuncs::meleeDamage_Function, 22);
    AddItem(miscMenu, "sensitivityScale", (void*)weaponFuncs::sensitivityScale_Function, 24);
    AddItem(miscMenu, "damageInnerRadius", (void*)weaponFuncs::damageInnerRadius_Function, 22);
    AddItem(miscMenu, "damageOuterRadius", (void*)weaponFuncs::damageOuterRadius_Function, 22);
    AddItem(miscMenu, "minDamagePercent", (void*)weaponFuncs::minDamagePercent_Function, 22);
    AddItem(timingMenu, "fireDelay", (void*)weaponFuncs::fireDelay_Function, 22);
    AddItem(timingMenu, "meleeDelay", (void*)weaponFuncs::meleeDelay_Function, 22);
    AddItem(timingMenu, "fireTime", (void*)weaponFuncs::fireTime_Function, 22);
    AddItem(timingMenu, "rechamberTime", (void*)weaponFuncs::rechamberTime_Function, 22);
    AddItem(timingMenu, "rechamberBoltTime", (void*)weaponFuncs::rechamberBoltTime_Function, 22);
    AddItem(timingMenu, "holdFireTime", (void*)weaponFuncs::holdFireTime_Function, 22);
    AddItem(timingMenu, "meleeTime", (void*)weaponFuncs::meleeTime_Function, 22);
    AddItem(timingMenu, "reloadTime", (void*)weaponFuncs::reloadTime_Function, 22);
    AddItem(timingMenu, "reloadEmptyTime", (void*)weaponFuncs::reloadEmptyTime_Function, 22);
    AddItem(timingMenu, "reloadAddTime", (void*)weaponFuncs::reloadAddTime_Function, 22);
    AddItem(timingMenu, "reloadStartTime", (void*)weaponFuncs::reloadStartTime_Function, 22);
    AddItem(timingMenu, "reloadStartAddTime", (void*)weaponFuncs::reloadStartAddTime_Function, 22);
    AddItem(timingMenu, "reloadEndTime", (void*)weaponFuncs::reloadEndTime_Function, 22);
    AddItem(timingMenu, "dropTime", (void*)weaponFuncs::dropTime_Function, 22);
    AddItem(timingMenu, "raiseTime", (void*)weaponFuncs::raiseTime_Function, 22);
    AddItem(timingMenu, "altDropTime", (void*)weaponFuncs::altDropTime_Function, 22);
    AddItem(timingMenu, "altRaiseTime", (void*)weaponFuncs::altRaiseTime_Function, 22);
    AddItem(timingMenu, "fuseTime", (void*)weaponFuncs::fuseTime_Function, 22);
    AddItem(miscMenu, "moveSpeedScale", (void*)weaponFuncs::moveSpeedScale_Function, 24);
    AddItem(kickMenu, "gunMaxPitch", (void*)weaponFuncs::gunMaxPitch_Function, 24);
    AddItem(kickMenu, "gunMaxYaw", (void*)weaponFuncs::gunMaxYaw_Function, 24);
    AddItem(swayMenu, "swayMaxAngle", (void*)weaponFuncs::swayMaxAngle_Function, 24);
    AddItem(swayMenu, "swayLerpSpeed", (void*)weaponFuncs::swayLerpSpeed_Function, 24);
    AddItem(swayMenu, "swayPitchScale", (void*)weaponFuncs::swayPitchScale_Function, 24);
    AddItem(swayMenu, "swayYawScale", (void*)weaponFuncs::swayYawScale_Function, 24);
    AddItem(swayMenu, "swayHorizScale", (void*)weaponFuncs::swayHorizScale_Function, 24);
    AddItem(swayMenu, "swayVertScale", (void*)weaponFuncs::swayVertScale_Function, 24);
    AddItem(swayMenu, "swayShellShockScale", (void*)weaponFuncs::swayShellShockScale_Function, 24);
    AddItem(swayMenu, "adsSwayMaxAngle", (void*)weaponFuncs::adsSwayMaxAngle_Function, 24);
    AddItem(swayMenu, "adsSwayLerpSpeed", (void*)weaponFuncs::adsSwayLerpSpeed_Function, 24);
    AddItem(swayMenu, "adsSwayPitchScale", (void*)weaponFuncs::adsSwayPitchScale_Function, 24);
    AddItem(swayMenu, "adsSwayYawScale", (void*)weaponFuncs::adsSwayYawScale_Function, 24);
    AddItem(swayMenu, "adsSwayHorizScale", (void*)weaponFuncs::adsSwayHorizScale_Function, 24);
    AddItem(swayMenu, "adsSwayVertScale", (void*)weaponFuncs::adsSwayVertScale_Function, 24);
    AddItem(projMenu, "takedamage", (void*)weaponFuncs::takedamage_Function, 22);
    AddItem(projMenu, "explosionRadius", (void*)weaponFuncs::explosionRadius_Function, 22);
    AddItem(projMenu, "explosionInnerDamage", (void*)weaponFuncs::explosionInnerDamage_Function, 22);
    AddItem(projMenu, "explosionOuterDamage", (void*)weaponFuncs::explosionOuterDamage_Function, 22);
    AddItem(projMenu, "projectileSpeed", (void*)weaponFuncs::projectileSpeed_Function, 22);
    AddItem(projMenu, "projectileSpeedUp", (void*)weaponFuncs::projectileSpeedUp_Function, 22);
    AddItem(projMenu, "triggerRadius", (void*)weaponFuncs::triggerRadius_Function, 22);
    AddItem(adsMenu, "adsTransInTime", (void*)weaponFuncs::adsTransInTime_Function, 22);
    AddItem(adsMenu, "adsTransOutTime", (void*)weaponFuncs::adsTransOutTime_Function, 22);
    AddItem(adsMenu, "adsIdleAmount", (void*)weaponFuncs::adsIdleAmount_Function, 24);
    AddItem(adsMenu, "adsZoomFov", (void*)weaponFuncs::adsZoomFov_Function, 24);
    AddItem(adsMenu, "adsSensitivityScale", (void*)weaponFuncs::adsSensitivityScale_Function, 24);
    AddItem(adsMenu, "adsZoomInFrac", (void*)weaponFuncs::adsZoomInFrac_Function, 24);
    AddItem(adsMenu, "adsZoomOutFrac", (void*)weaponFuncs::adsZoomOutFrac_Function, 24);
    AddItem(adsMenu, "adsOverlayWidth", (void*)weaponFuncs::adsOverlayWidth_Function, 24);
    AddItem(adsMenu, "adsOverlayHeight", (void*)weaponFuncs::adsOverlayHeight_Function, 24);
    AddItem(adsMenu, "adsBobFactor", (void*)weaponFuncs::adsBobFactor_Function, 24);
    AddItem(adsMenu, "adsViewBobMult", (void*)weaponFuncs::adsViewBobMult_Function, 24);
    AddItem(adsMenu, "adsAimPitch", (void*)weaponFuncs::adsAimPitch_Function, 24);
    AddItem(adsMenu, "adsCrosshairInFrac", (void*)weaponFuncs::adsCrosshairInFrac_Function, 24);
    AddItem(adsMenu, "adsCrosshairOutFrac", (void*)weaponFuncs::adsCrosshairOutFrac_Function, 24);
    AddItem(timingMenu, "adsReloadTransTime", (void*)weaponFuncs::adsReloadTransTime_Function, 22);
    AddItem(timingMenu, "adsTransBlendTime", (void*)weaponFuncs::adsTransBlendTime_Function, 22);
    AddItem(adsKickMenu, "adsGunKickPitchMin", (void*)weaponFuncs::adsGunKickPitchMin_Function, 24);
    AddItem(adsKickMenu, "adsGunKickPitchMax", (void*)weaponFuncs::adsGunKickPitchMax_Function, 24);
    AddItem(adsKickMenu, "adsGunKickYawMin", (void*)weaponFuncs::adsGunKickYawMin_Function, 24);
    AddItem(adsKickMenu, "adsGunKickYawMax", (void*)weaponFuncs::adsGunKickYawMax_Function, 24);
    AddItem(adsKickMenu, "adsGunKickAccel", (void*)weaponFuncs::adsGunKickAccel_Function, 24);
    AddItem(adsKickMenu, "adsGunKickSpeedMax", (void*)weaponFuncs::adsGunKickSpeedMax_Function, 24);
    AddItem(adsKickMenu, "adsGunKickSpeedDecay", (void*)weaponFuncs::adsGunKickSpeedDecay_Function, 24);
    AddItem(adsKickMenu, "adsGunKickStaticDecay", (void*)weaponFuncs::adsGunKickStaticDecay_Function, 24);
    AddItem(adsKickMenu, "adsViewKickPitchMin", (void*)weaponFuncs::adsViewKickPitchMin_Function, 24);
    AddItem(adsKickMenu, "adsViewKickPitchMax", (void*)weaponFuncs::adsViewKickPitchMax_Function, 24);
    AddItem(adsKickMenu, "adsViewKickYawMin", (void*)weaponFuncs::adsViewKickYawMin_Function, 24);
    AddItem(adsKickMenu, "adsViewKickYawMax", (void*)weaponFuncs::adsViewKickYawMax_Function, 24);
    AddItem(adsKickMenu, "adsViewKickCenterSpeed", (void*)weaponFuncs::adsViewKickCenterSpeed_Function, 24);
    AddItem(aimMenu, "adsSpread", (void*)weaponFuncs::adsSpread_Function, 24);
    AddItem(aimMenu, "adsSpreadDucked", (void*)weaponFuncs::adsSpreadDucked_Function, 24);
    AddItem(aimMenu, "adsSpreadProne", (void*)weaponFuncs::adsSpreadProne_Function, 24);
    AddItem(aimMenu, "hipSpreadStandMin", (void*)weaponFuncs::hipSpreadStandMin_Function, 24);
    AddItem(aimMenu, "hipSpreadDuckedMin", (void*)weaponFuncs::hipSpreadDuckedMin_Function, 24);
    AddItem(aimMenu, "hipSpreadProneMin", (void*)weaponFuncs::hipSpreadProneMin_Function, 24);
    AddItem(aimMenu, "hipSpreadMax", (void*)weaponFuncs::hipSpreadMax_Function, 24);
    AddItem(aimMenu, "hipSpreadDecayRate", (void*)weaponFuncs::hipSpreadDecayRate_Function, 24);
    AddItem(aimMenu, "hipSpreadFireAdd", (void*)weaponFuncs::hipSpreadFireAdd_Function, 24);
    AddItem(aimMenu, "hipSpreadTurnAdd", (void*)weaponFuncs::hipSpreadTurnAdd_Function, 24);
    AddItem(aimMenu, "hipSpreadMoveAdd", (void*)weaponFuncs::hipSpreadMoveAdd_Function, 24);
    AddItem(aimMenu, "hipSpreadDuckedDecay", (void*)weaponFuncs::hipSpreadDuckedDecay_Function, 24);
    AddItem(aimMenu, "hipSpreadProneDecay", (void*)weaponFuncs::hipSpreadProneDecay_Function, 24);
    AddItem(aimMenu, "hipReticleSidePos", (void*)weaponFuncs::hipReticleSidePos_Function, 24);
    AddItem(aimMenu, "hipIdleAmount", (void*)weaponFuncs::hipIdleAmount_Function, 24);
    AddItem(kickMenu, "hipGunKickPitchMin", (void*)weaponFuncs::hipGunKickPitchMin_Function, 24);
    AddItem(kickMenu, "hipGunKickPitchMax", (void*)weaponFuncs::hipGunKickPitchMax_Function, 24);
    AddItem(kickMenu, "hipGunKickYawMin", (void*)weaponFuncs::hipGunKickYawMin_Function, 24);
    AddItem(kickMenu, "hipGunKickYawMax", (void*)weaponFuncs::hipGunKickYawMax_Function, 24);
    AddItem(kickMenu, "hipGunKickAccel", (void*)weaponFuncs::hipGunKickAccel_Function, 24);
    AddItem(kickMenu, "hipGunKickSpeedMax", (void*)weaponFuncs::hipGunKickSpeedMax_Function, 24);
    AddItem(kickMenu, "hipGunKickSpeedDecay", (void*)weaponFuncs::hipGunKickSpeedDecay_Function, 24);
    AddItem(kickMenu, "hipGunKickStaticDecay", (void*)weaponFuncs::hipGunKickStaticDecay_Function, 24);
    AddItem(kickMenu, "hipViewKickPitchMin", (void*)weaponFuncs::hipViewKickPitchMin_Function, 24);
    AddItem(kickMenu, "hipViewKickPitchMax", (void*)weaponFuncs::hipViewKickPitchMax_Function, 24);
    AddItem(kickMenu, "hipViewKickYawMin", (void*)weaponFuncs::hipViewKickYawMin_Function, 24);
    AddItem(kickMenu, "hipViewKickYawMax", (void*)weaponFuncs::hipViewKickYawMax_Function, 24);
    AddItem(kickMenu, "hipViewKickCenterSpeed", (void*)weaponFuncs::hipViewKickCenterSpeed_Function, 24);
    AddItem(aimMenu, "aiEffectiveRange", (void*)weaponFuncs::aiEffectiveRange_Function, 24);
    AddItem(aimMenu, "aiMissRange", (void*)weaponFuncs::aiMissRange_Function, 24);
    AddItem(aimMenu, "aiDamageMod", (void*)weaponFuncs::aiDamageMod_Function, 24);
    AddItem(aimMenu, "bulletConeAngle", (void*)weaponFuncs::bulletConeAngle_Function, 24);
    AddItem(aimMenu, "adsBulletConeAngle", (void*)weaponFuncs::adsBulletConeAngle_Function, 24);
    AddItem(animIKMenu, "animIKOffsetTime", (void*)weaponFuncs::animIKOffsetTime_Function, 24);
    AddItem(animIKMenu, "animIKOffsetForce", (void*)weaponFuncs::animIKOffsetForce_Function, 24);
    AddItem(animIKMenu, "animIKOffsetDist", (void*)weaponFuncs::animIKOffsetDist_Function, 24);
    AddItem(animIKMenu, "animIKPitchTime", (void*)weaponFuncs::animIKPitchTime_Function, 24);
    AddItem(animIKMenu, "animIKPitchForce", (void*)weaponFuncs::animIKPitchForce_Function, 24);
    AddItem(animIKMenu, "animIKPitchAngle", (void*)weaponFuncs::animIKPitchAngle_Function, 24);
    AddItem(animIKMenu, "animIKTorsoRecoilPitchTime", (void*)weaponFuncs::animIKTorsoRecoilPitchTime_Function, 24);
    AddItem(animIKMenu, "animIKTorsoRecoilPitchForce", (void*)weaponFuncs::animIKTorsoRecoilPitchForce_Function, 24);
    AddItem(animIKMenu, "animIKTorsoRecoilPitchAngle", (void*)weaponFuncs::animIKTorsoRecoilPitchAngle_Function, 24);
    AddItem(v4, "damageMP", (void*)weaponFuncs::damageMP_Function, 22);
    AddItem(v4, "meleeDamageMP", (void*)weaponFuncs::meleeDamageMP_Function, 22);
    AddItem(v4, "damageInnerRadiusMP", (void*)weaponFuncs::damageInnerRadiusMP_Function, 22);
    AddItem(v4, "damageOuterRadiusMP", (void*)weaponFuncs::damageOuterRadiusMP_Function, 22);
    AddItem(v4, "minDamagePercentMP", (void*)weaponFuncs::minDamagePercentMP_Function, 22);
    AddItem(v4, "fireDelayMP", (void*)weaponFuncs::fireDelayMP_Function, 22);
    AddItem(v4, "meleeDelayMP", (void*)weaponFuncs::meleeDelayMP_Function, 22);
    AddItem(v4, "fireTimeMP", (void*)weaponFuncs::fireTimeMP_Function, 22);
    AddItem(v4, "meleeTimeMP", (void*)weaponFuncs::meleeTimeMP_Function, 22);
    AddItem(v4, "explosionRadiusMP", (void*)weaponFuncs::explosionRadiusMP_Function, 22);
    AddItem(v4, "explosionInnerDamageMP", (void*)weaponFuncs::explosionInnerDamageMP_Function, 22);
    AddItem(v4, "explosionOuterDamageMP", (void*)weaponFuncs::explosionOuterDamageMP_Function, 22);
    AddItem(v4, "maxAmmoMP", (void*)weaponFuncs::maxAmmoMP_Function, 22);
    AddItem(v4, "sensitivityScaleMP", (void*)weaponFuncs::sensitivityScaleMP_Function, 24);
    AddItem(v4, "adsZoomFovMP", (void*)weaponFuncs::adsZoomFovMP_Function, 24);
    AddItem(v4, "adsTransInTimeMP", (void*)weaponFuncs::adsTransInTimeMP_Function, 22);
    AddItem(v4, "adsTransOutTimeMP", (void*)weaponFuncs::adsTransOutTimeMP_Function, 22);
    AddItem(v4, "adsSensitivityScaleMP", (void*)weaponFuncs::adsSensitivityScaleMP_Function, 24);
    AddItem(v4, "adsSpreadMP", (void*)weaponFuncs::adsSpreadMP_Function, 24);
    AddItem(v4, "adsSpreadDuckedMP", (void*)weaponFuncs::adsSpreadDuckedMP_Function, 24);
    AddItem(v4, "adsSpreadProneMP", (void*)weaponFuncs::adsSpreadProneMP_Function, 24);
    AddItem(v4, "hipSpreadStandMinMP", (void*)weaponFuncs::hipSpreadStandMinMP_Function, 24);
    AddItem(v4, "hipSpreadDuckedMinMP", (void*)weaponFuncs::hipSpreadDuckedMinMP_Function, 24);
    AddItem(v4, "hipSpreadProneMinMP", (void*)weaponFuncs::hipSpreadProneMinMP_Function, 24);
    AddItem(v4, "hipSpreadMaxMP", (void*)weaponFuncs::hipSpreadMaxMP_Function, 24);
    AddItem(v4, "hipSpreadDecayRateMP", (void*)weaponFuncs::hipSpreadDecayRateMP_Function, 24);
    AddItem(v4, "hipSpreadFireAddMP", (void*)weaponFuncs::hipSpreadFireAddMP_Function, 24);
    AddItem(v4, "hipSpreadTurnAddMP", (void*)weaponFuncs::hipSpreadTurnAddMP_Function, 24);
    AddItem(v4, "hipSpreadMoveAddMP", (void*)weaponFuncs::hipSpreadMoveAddMP_Function, 24);
    AddItem(v4, "hipSpreadDuckedDecayMP", (void*)weaponFuncs::hipSpreadDuckedDecayMP_Function, 24);
    AddItem(v4, "hipSpreadProneDecayMP", (void*)weaponFuncs::hipSpreadProneDecayMP_Function, 24);
    AddItem(parentMenu, "Draw Crosshair", &cg_forceCrosshair.integer, 2);
    AddItem(parentMenu, "Draw Bullets", &g_debugBullets.integer, 0x60001);
    r_showLocationalDamage = Cvar_Get("r_showLocationalDamage", "0", 512);
    AddItem(parentMenu, "Draw Hit Boxes", &r_showLocationalDamage->integer, 0x60001);
    bg_stickyAimRender = Cvar_Get("bg_stickyAimRender", "0", 512);
    IN_Init();
    joy_threshold = Cvar_Get("joy_threshold", "40", 1);
    AddItem(parentMenu, "Draw StickyAim", &bg_stickyAimRender->integer, 0x20001);
    AddItem(parentMenu, "Draw Weap Anim", &bg_debugWeaponAnim.integer, 2);
    AddItem(parentMenu, "Draw Weap State", &bg_debugWeaponState.integer, 2);
    AddItem(parentMenu, "Draw Grenades", &g_debugGrenades.integer, 2);
    AddItem(parentMenu, "Joystick Threshold", &joy_threshold->integer, 1);
    AddItem(parentMenu, "Melee Assist Range", &bg_meleeassistrange.integer, 0x40001);
    AddItem(parentMenu, "Melee Assist Angular Speed",
            &bg_meleeassistaspeed.integer, 0x40001);
    AddItem(parentMenu, "Melee Assist FOV Min Dot Prod",
            &bg_meleeassistfov.value, 9);
}

// ============================================================================
// InspectorManager::AddSettingsMenus - ea: 0x50D840
// ============================================================================
void InspectorManager::AddSettingsMenus()
{
    _INSPECTOR_MENU* v2 = AddSubMenu(nullptr, "Game Settings");
    _INSPECTOR_MENU* v3 = AddSubMenu(nullptr, "Controller");
    AddItem(v3, "Config A", (void*)FN_ControlConfigA, 17);
    AddItem(v3, "Config B", (void*)FN_ControlConfigB, 17);
    AddItem(v3, "Config C", (void*)FN_ControlConfigC, 17);
    AddItem(v3, "Config D", (void*)FN_ControlConfigD, 17);
    AddItem(v3, "Sticks Default", (void*)FN_ControlSticksDefault, 17);
    AddItem(v3, "Sticks South Paw", (void*)FN_ControlSticksSouthPaw, 17);
    AddItem(v3, "Sticks Legacy", (void*)FN_ControlSticksLegacy, 17);
    AddItem(v3, "Sticks Legacy South Paw", (void*)FN_ControlSticksLegacySouthPaw, 17);
    in_stickSouthPaw = Cvar_Get("in_stickSouthPaw", "0", 0);
    in_stickLegacy = Cvar_Get("in_stickLegacy", "0", 0);
    cl_freeze = Cvar_Get("cl_freeze", "0", 0);
    AddItem(v3, "Freeze Input", &cl_freeze->integer, 2);
    AddItem(v3, "Invert Aim", (void*)FN_ControlInvertAim, 17);
    _INSPECTOR_MENU* v4 = AddSubMenu(v2, "Difficulty Settings");
    AddItem(v4, "Easy Accuracy Mod %", &gEasyAccuracyMod, 0x40007);
    AddItem(v4, "Normal Accuracy Mod %", &gNormalAccuracyMod, 0x40007);
    AddItem(v4, "Hard Accuracy Mod %", &gHardAccuracyMod, 0x40007);
    AddItem(v4, "New Easy Max Health", &gNewEasyMaxHealth, 1);
    AddItem(v4, "New Medium Max Health", &gNewMediumMaxHealth, 1);
    AddItem(v4, "New Hard Max Health", &gNewHardMaxHealth, 1);
    AddItem(v4, "Change to Easy Difficulty", (void*)FN_ApplyEasyDifficultyChanges, 17);
    AddItem(v4, "Change to Medium Difficulty", (void*)FN_ApplyMediumDifficultyChanges, 17);
    AddItem(v4, "Change to Hard Difficulty", (void*)FN_ApplyHardDifficultyChanges, 17);
    _INSPECTOR_MENU* v5 = AddSubMenu(v2, "Shake and Rumble");
    AddItem(v5, "Low Frequency Delay", &gLowFreqDelay, 0x40007);
    AddItem(v5, "Low Frequency Steady Duration", &gLowFreqSteadyDuration, 0x40007);
    AddItem(v5, "Low Frequency Ramp Up Time", &gLowFreqRampUpTime, 0x40007);
    AddItem(v5, "Low Frequency Ramp Down Time", &gLowFreqRampDownTime, 0x40007);
    AddItem(v5, "Low Frequency Intensity", &gLowFreqRumbleIntensity, 0x40007);
    AddItem(v5, "High Frequency Delay", &gHighFreqDelay, 0x40007);
    AddItem(v5, "High Frequency Duration", &gHighFreqDuration, 0x40007);
    AddItem(v5, "RUN SHAKE AND RUMBLE!!!", (void*)PlayRumble, 17);
    AddWeaponMenus(v2);
    AddVehicleMenus(v2);
    AddAimAssistMenus(v2);
    _INSPECTOR_MENU* v6 = AddSubMenu(v2, "Fog Values (Set these in script)");
    AddItem(v6, "Fog ON/OFF", &g_FogEnable, 2);
    AddItem(v6, "Fog Red", &g_FogRed, 9);
    AddItem(v6, "Fog Green", &g_FogGreen, 9);
    AddItem(v6, "Fog Blue", &g_FogBlue, 9);
    AddItem(v6, "Fog Near", &g_FogNear, 0x40004);
    AddItem(v6, "Fog Far", &g_FogFar, 0x40004);
    AddItem(v6, "Fog Start", &g_FogStart, 9);
    AddItem(v6, "Fog End", &g_FogEnd, 9);
    cvar_t* v7 = Cvar_Get("r_LightScale", "1.0", 256);
    AddItem(v2, "Object Ambient Colour", &g_objectAmbientHelper, 7);
    AddItem(v2, "Object Diffuse Colour", &g_objectDiffuseHelper, 7);
    AddItem(v2, "Entity Light Scaler", &v7->value, 0x40007);
    _INSPECTOR_MENU* v8 = AddSubMenu(v2, "Camera Shake Testing");
    AddItem(v8, "Shake Magnitude", &g_ShakeTestMag, 8);
    AddItem(v8, "Shake Frequency", &g_ShakeTestFreq, 8);
    AddItem(v8, "Shake Time", &g_ShakeTestTime, 8);
    AddItem(v8, "Shake 2D Effect", &g_ShakeTest2d, 2);
    AddItem(v8, "Start Shake Now", (void*)FN_ShakeTestFunction, 17);
    AddItem(v8, "Enable Default Shellshock Effect",
            (void*)FN_DefaultShellshockTestFunction, 17);
    AddItem(v8, "Enable Pain Shellshock Effect",
            (void*)FN_PainShellshockTestFunction, 17);
    AddItem(v8, "Enable Death Shellshock Effect",
            (void*)FN_DeathShellshockTestFunction, 17);
    _INSPECTOR_MENU* v9 = AddSubMenu(v2, "blur effect testing");
    AddItem(v9, "Curgen motion blur", (void*)FN_CurgenMotionBlur, 17);
    _INSPECTOR_MENU* v10 = AddSubMenu(v2, "Ocean Config");
    AddItem(v10, "Enable", &g_oceanDebug_Enable, 2);
    AddItem(v10, "Bank Number", &g_oceanDebug_BankID, 0x40001);
    AddItem(v10, "Dump Settings", &g_oceanDebug_DumpSettings, 2);
    AddItem(v10, "Layer 2 Enable", &g_oceanDebug_Layer2Enable, 2);
    AddItem(v10, "Layer 3 Enable", &g_oceanDebug_Layer3Enable, 2);
    AddItem(v10, "Lightmap Enable", &g_oceanDebug_LightmapEnable, 2);
    _INSPECTOR_MENU* pLayer = AddSubMenu(v10, "Diffuse Settings");
    AddItem(pLayer, "U Scale", &g_oceanDebug_UVScale[0], 0x40006);
    AddItem(pLayer, "V Scale", &g_oceanDebug_UVScale[1], 0x40006);
    AddItem(pLayer, "U Scroll", &g_oceanDebug_UVScroll[0], 7);
    AddItem(pLayer, "V Scroll", &g_oceanDebug_UVScroll[1], 7);
    pLayer = AddSubMenu(v10, "Layer 2 Settings");
    AddItem(pLayer, "U Scale", &g_oceanDebug_UVScale[2], 0x40006);
    AddItem(pLayer, "V Scale", &g_oceanDebug_UVScale[3], 0x40006);
    AddItem(pLayer, "U Scroll", &g_oceanDebug_UVScroll[2], 7);
    AddItem(pLayer, "V Scroll", &g_oceanDebug_UVScroll[3], 7);
    AddItem(pLayer, "Alpha", &g_oceanDebug_Layer2Alpha, 9);
    pLayer = AddSubMenu(v10, "Layer 3 Settings");
    AddItem(pLayer, "U Scale", &g_oceanDebug_UVScale[4], 0x40006);
    AddItem(pLayer, "V Scale", &g_oceanDebug_UVScale[5], 0x40006);
    AddItem(pLayer, "U Scroll", &g_oceanDebug_UVScroll[4], 7);
    AddItem(pLayer, "V Scroll", &g_oceanDebug_UVScroll[5], 7);
    AddItem(pLayer, "Alpha", &g_oceanDebug_Layer3Alpha, 9);
    AddItem(v10, "Sea Level", &g_oceanDebug_SeaLevel, 6);
    const char* waveMenuNames[4] = { "Wave 1", "Wave 2", "Wave 3", "Wave 4" };
    for (int wave = 0; wave < 4; ++wave)
    {
        _INSPECTOR_MENU* pWave = AddSubMenu(v10, (char*)waveMenuNames[wave]);
        AddItem(pWave, "Origin X", &g_oceanDebug_Origin[wave * 2], 4);
        AddItem(pWave, "Origin Y", &g_oceanDebug_Origin[wave * 2 + 1], 4);
        AddItem(pWave, "Distance", &g_oceanDebug_Distance[wave], 0x40004);
        AddItem(pWave, "Heading", &g_oceanDebug_Heading[wave], 0x40006);
        AddItem(pWave, "Wavelength", &g_oceanDebug_Wavelength[wave], 0x40006);
        AddItem(pWave, "Amplitude", &g_oceanDebug_Amplitude[wave], 6);
        AddItem(pWave, "Phase", &g_oceanDebug_Phase[wave], 9);
        AddItem(pWave, "Timescale", &g_oceanDebug_Timescale[wave], 7);
    }
    _INSPECTOR_MENU* v11 = AddSubMenu(v2, "cdSimpleAlpha Config");
    AddItem(v11, "Enable", &g_cdSimpleAlphaDebug.mEnable, 2);
    AddItem(v11, "Blend Mode (*10)", &g_cdSimpleAlphaDebug.mBlendMode, 0x40001);
    AddItem(v11, "Backface Culling", &g_cdSimpleAlphaDebug.mBackface, 2);
    AddItem(v11, "Enable Sorting", &g_cdSimpleAlphaDebug.mEnableZPass, 2);
    AddItem(v11, "Enable Tint", &g_cdSimpleAlphaDebug.mEnableTint, 2);
    AddItem(v11, "Enable Pulsing", &g_cdSimpleAlphaDebug.mEnablePulsing, 2);
    AddItem(v11, "Tint R", &g_cdSimpleAlphaDebug.mTint[0], 10);
    AddItem(v11, "Tint G", &g_cdSimpleAlphaDebug.mTint[1], 10);
    AddItem(v11, "Tint B", &g_cdSimpleAlphaDebug.mTint[2], 10);
    AddItem(v11, "Alpha", &g_cdSimpleAlphaDebug.mTint[3], 9);
    AddItem(v11, "Alpha Cutoff", &g_cdSimpleAlphaDebug.mAlphaCutoff, 9);
    AddItem(v11, "Pulse Rate", &g_cdSimpleAlphaDebug.mPulseRate, 0x40007);
    AddItem(v11, "Min Tint", &g_cdSimpleAlphaDebug.mMinTint, 9);
    AddItem(v11, "Max Tint", &g_cdSimpleAlphaDebug.mMaxTint, 9);
    AddItem(v11, "Min Alpha", &g_cdSimpleAlphaDebug.mMinAlpha, 9);
    AddItem(v11, "Max Alpha", &g_cdSimpleAlphaDebug.mMaxAlpha, 9);
}

// ============================================================================
// InspectorManager::SetupUserMenus - ea: 0x50E1B0
// ============================================================================
void InspectorManager::SetupUserMenus()
{
    AddMultiplayerMenus();
    AddPlayerMenus();
    AddDesignerMenus();
    AddSettingsMenus();
    AddRenderMenus();
    AddFXMenus();
    AddCollisionMenus();
    AddSoundMenus(this);
    AddDebuggingMenus();
    _INSPECTOR_MENU* v2 = AddSubMenu(nullptr, "Screen Shot");
    AddItem(v2, "ScreenShot", &gTakeScreenshot, 2);
    AddItem(v2, "Cubemap ScreenShot", (void*)TakeCubeMapShot, 17);
    _INSPECTOR_MENU* v3 = AddSubMenu(nullptr, "Physics / Nano");
    AddItem(v3, "Rag Dolls on normal deaths", &g_useRagsOnNormalDeaths, 2);
}

// ============================================================================
// RenderPlayerStats - ea: 0x4F0E10
// Stats table renderer: 7 columns (per player slot) x rows (stat categories).
// ============================================================================
Entity* RenderPlayerStats()
{
    Entity* v1 = EntityManager::sInst->GetPlayer(currCl);
    if (v1 == nullptr || v1->client == nullptr)
        return v1;
    static const int s_statCol[25] = {
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 11, 12, 13, 14, 15, 16, 17,
        18, 19, 20, 21, 22, 23, 24, 25
    };
    static const char* s_label[25] = {
        "Time", "Wins", "Losses", "Kills", "Deaths", "VehiclesDestroyed",
        "Suicides", "TeamKills", "Mantle", "GrenadeKills", "Supply",
        "ArtilleryKills", "MineKills", "RifleGrenadeKills", "CTFFlagPickup",
        "CTFFlagCapture", "SCFFlagPickup", "SCFFlagCapture", "WARAreaCapture",
        "HQDestroy", "Spots", "WARAreaDefend", "WARAreaAssist", "SCFFlagDefend",
        "CTFFlagDefend"
    };
    char tmpstr[128];
    float white[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
    int y = 50;
    for (int r = 0; r < 25; ++r)
    {
        g_inspectorManager.Print((char*)s_label[r], (int)xpos, y, 0.55f);
        int xx = 300;
        for (int c = 0; c < 7; ++c)
        {
            sprintf(tmpstr, "%d", v1->client->pers.mStats[c][s_statCol[r]]);
            RE_Text_Paint((float)(xx + 2), (float)(y + 2), 5,
                          scaleScalar * 0.55f, white, tmpstr, 0, 0, 0);
            RE_Text_Paint((float)xx, (float)y, 5, scaleScalar * 0.55f,
                          g_inspectorManager.m_currentRgba, tmpstr, 0, 0, 0);
            xx += (int)xinc;
        }
        y += (int)yinc;
    }
    // CTFFlagReturn (stat col 27) + per-slot row
    g_inspectorManager.Print((char*)"CTFFlagReturn", (int)xpos, y, 0.55f);
    int xx = 300;
    for (int c = 0; c < 7; ++c)
    {
        sprintf(tmpstr, "%d", v1->client->pers.mStats[c][27]);
        RE_Text_Paint((float)(xx + 2), (float)(y + 2), 5, scaleScalar * 0.55f,
                      white, tmpstr, 0, 0, 0);
        RE_Text_Paint((float)xx, (float)y, 5, scaleScalar * 0.55f,
                      g_inspectorManager.m_currentRgba, tmpstr, 0, 0, 0);
        xx += (int)xinc;
    }
    // Total row (stat col 28)
    y += (int)yinc;
    xx = 300;
    for (int c = 0; c < 7; ++c)
    {
        sprintf(tmpstr, "%d", v1->client->pers.mStats[c][28]);
        RE_Text_Paint((float)(xx + 2), (float)(y + 2), 5, scaleScalar * 0.55f,
                      white, tmpstr, 0, 0, 0);
        RE_Text_Paint((float)xx, (float)y, 5, scaleScalar * 0.55f,
                      g_inspectorManager.m_currentRgba, tmpstr, 0, 0, 0);
        xx += (int)xinc;
    }
    return v1;
}

// ============================================================================
// IM_RenderGameEntityStats - ea: 0x502150
// ============================================================================
int IM_RenderGameEntityStats()
{
    char tmpstr[128];
    float white[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
    int numParticles = 0;
    int numVis = 0;
    for (int i = 0; i < gParticleEffectList.mSize; ++i)
    {
        ParticleEffectLocal* pe = gParticleEffectList.mElements[i];
        ++numParticles;
        if (pe != nullptr && pe->mEffect != nullptr
            && (pe->mEffect->mFlags & 2) != 0)
            ++numVis;
    }
    sprintf(tmpstr, "Num Particle Effects: %d   Vis: %d", numParticles,
            numVis);
    RE_Text_Paint(416.0f, 42.0f, 5, scaleScalar * 0.55f, white, tmpstr,
                  0, 0, 0);
    RE_Text_Paint(408.0f, 40.0f, 5, scaleScalar * 0.55f,
                  g_inspectorManager.m_currentRgba, tmpstr, 0, 0, 0);

    int numEntities = 0;
    int numEntitiesVisible = 0;
    int numDrones = 0;
    int numEntGeneral = 0;
    int numEntActor = 0;
    int numEntSpawner = 0;
    int numEntCorpse = 0;
    int numEntItem = 0;
    int numEntMissile = 0;
    int numEntMover = 0;
    int numEntPortal = 0;
    int numEntInvisible = 0;
    int numEntScriptMover = 0;
    int numEntSound = 0;
    int numEntLoopFX = 0;
    int numEntMG42 = 0;
    int numEntVehicle = 0;
    int numEntVehicleCorpse = 0;
    int numEntVehicleCollmap = 0;
    int numTriggerMultiple = 0;
    int numTriggerOnce = 0;
    int numTriggerLookAt = 0;
    int numTriggerFriendlyChain = 0;
    int numTriggerDamage = 0;
    int numTriggerUse = 0;
    int unknown = 0;

    for (unsigned int idx = 0; idx < 0x540; ++idx)
    {
        Entity* mObject = EntityHandleDb::sInst.mElements[idx].mObject;
        ++numEntities;
        if (mObject == nullptr)
            continue;
        if ((mObject->flags & 0x2000000) != 0)
            ++numDrones;
        if (mObject->mDObj != nullptr
            && (g_DOBJF_NOT_RENDERED_LAST_FRAME & mObject->mDObj->mFlags) == 0)
            ++numEntitiesVisible;
        switch (mObject->s.eType)
        {
        case 0:
        {
            ++numEntGeneral;
            if (mObject->mClassName.mBlock != nullptr
                && mObject->mClassName.mBlock->mLength != 0)
            {
                const char* name = (const char*)&mObject->mClassName.mBlock[1];
                if (strcmp(name, "trigger_multiple") == 0)
                    ++numTriggerMultiple;
                else if (strcmp(name, "trigger_once") == 0)
                    ++numTriggerOnce;
                else if (strcmp(name, "trigger_lookat") == 0)
                    ++numTriggerLookAt;
                else if (strcmp(name, "trigger_friendlychain") == 0)
                    ++numTriggerFriendlyChain;
                else if (strcmp(name, "trigger_damage") == 0)
                    ++numTriggerDamage;
                else if (strcmp(name, "trigger_use") == 0)
                    ++numTriggerUse;
            }
            break;
        }
        case 1:
        case 11:
            ++numEntActor;
            break;
        case 2:
            ++numEntItem;
            break;
        case 3:
            ++numEntMissile;
            break;
        case 4:
            ++numEntMover;
            break;
        case 5:
            ++numEntPortal;
            break;
        case 6:
            ++numEntInvisible;
            break;
        case 7:
            ++numEntScriptMover;
            break;
        case 8:
            ++numEntSound;
            break;
        case 9:
            ++numEntLoopFX;
            break;
        case 10:
            ++numEntMG42;
            break;
        case 12:
            ++numEntSpawner;
            break;
        case 13:
            ++numEntCorpse;
            break;
        case 14:
            ++numEntVehicle;
            break;
        case 15:
            ++numEntVehicleCorpse;
            break;
        case 16:
            ++numEntVehicleCollmap;
            break;
        default:
            ++unknown;
            break;
        }
    }

    struct Row { const char* fmt; int val; int y; };
    Row rows[] = {
        { "Num Entities: %d   Vis: %d", numEntities, 70 },
        { "Num Drones: %d", numDrones, 84 },
        { "General: %d", numEntGeneral, 112 },
        { "Actor: %d", numEntActor, 126 },
        { "Spawner: %d", numEntSpawner, 140 },
        { "Corpse: %d", numEntCorpse, 154 },
        { "Item: %d", numEntItem, 168 },
        { "Missile: %d", numEntMissile, 182 },
        { "Mover: %d", numEntMover, 196 },
        { "Portal: %d", numEntPortal, 210 },
        { "Invisible: %d", numEntInvisible, 224 },
        { "ScriptMover: %d", numEntScriptMover, 238 },
        { "Sound: %d", numEntSound, 252 },
        { "LoopFX: %d", numEntLoopFX, 266 },
        { "MG42: %d", numEntMG42, 280 },
        { "Vehicle: %d", numEntVehicle, 294 },
        { "Vehicle Corpse: %d", numEntVehicleCorpse, 308 },
        { "Vehicle Collmap: %d", numEntVehicleCollmap, 322 },
        { "Unknown: %d", unknown, 336 },
    };
    for (int r = 0; r < 19; ++r)
    {
        sprintf(tmpstr, rows[r].fmt, rows[r].val);
        RE_Text_Paint(416.0f, (float)(rows[r].y + 2), 5, scaleScalar * 0.55f,
                      white, tmpstr, 0, 0, 0);
        RE_Text_Paint(408.0f, (float)rows[r].y, 5, scaleScalar * 0.55f,
                      g_inspectorManager.m_currentRgba, tmpstr, 0, 0, 0);
    }
    const char* trig[] = {
        "Triggers:", "->  Multiple %d", "->  Once %d", "->  LookAt %d",
        "->  FriendlyChain %d", "->  Damage %d", "->  Use %d",
    };
    int trigVals[] = {
        0, numTriggerMultiple, numTriggerOnce, numTriggerLookAt,
        numTriggerFriendlyChain, numTriggerDamage, numTriggerUse,
    };
    int y = 350;
    for (int r = 0; r < 7; ++r)
    {
        if (r == 0)
            strcpy(tmpstr, trig[0]);
        else
            sprintf(tmpstr, trig[r], trigVals[r]);
        RE_Text_Paint(416.0f, (float)(y + 2), 5, scaleScalar * 0.55f, white,
                      tmpstr, 0, 0, 0);
        RE_Text_Paint(408.0f, (float)y, 5, scaleScalar * 0.55f,
                      g_inspectorManager.m_currentRgba, tmpstr, 0, 0, 0);
        y += 14;
    }
    return 0;
}

// ============================================================================
// InspectorManager::UserRenderHook - ea: 0x509CA0
// ============================================================================
void InspectorManager::UserRenderHook()
{
    char string[128];
    char tmpstr[128];
    Cvar_Set("battlechatter_debug", va("%i", gAIBattleChatterDebug));
    SoundDebugRender(this);
    int v3 = 0;
    if (g_displayPlayerPosition)
    {
        Entity* Player = EntityManager::sInst->GetPlayer(currCl);
        float playerPos[3];
        Sentient_GetOrigin(Player->sentient, playerPos);
        sprintf(tmpstr, "Pos: %.1f  %.1f  %.1f", playerPos[0],
                playerPos[1], playerPos[2]);
        Print(tmpstr, 320, 50, 0.55f);
    }
    if (g_drawDebugLos || g_drawDebugEntityLos)
    {
        sprintf(tmpstr, "LOS Total: %d  LOS Hits: %d",
                g_numLosHits + g_numLosMisses, g_numLosHits);
        Print(tmpstr, 360, 35, 0.55f);
        float v5 = g_losResetTime - ServerTime::sInst.mTickDelta;
        g_losResetTime = g_losResetTime - ServerTime::sInst.mTickDelta;
        if (g_losResetTime <= 0.0f)
        {
            g_losResetTime = v5 + 1.0f;
            g_numLosHits = 0;
            g_numLosMisses = 0;
        }
    }
    int v6 = Sys_Milliseconds();
    unsigned int v8 = v6 - g_previousSysTime;
    bool v7 = v6 == g_previousSysTime;
    g_previousMS = v6 - g_previousSysTime;
    g_previousSysTime = v6;
    if (!v7)
    {
        g_fps = 0x3E8 / v8;
        if (g_renderFPS)
        {
            sprintf(tmpstr, "FPS: %d", 0x3E8 / v8);
            Print(tmpstr, 450, 45, 0.55f);
        }
    }
    if (g_showNumBadPaths)
    {
        sprintf(tmpstr, "Bad Paths: %d %d", g_badPathManager.mNumBadPaths,
                g_badPathManager.mTotalNum);
        Print(tmpstr, 450, 45, 0.55f);
    }
    if (g_showPathNodeDensity)
    {
        int mNodeCount = 0;
        Entity* v10 = EntityManager::sInst->GetPlayer(currCl);
        if (Sentient_NearestNode(v10->sentient, nullptr, nullptr, 0, 1,
                                 192.0f, 0))
        {
            mNodeCount = (*(PathNodeLevelTOC**)PathNodeMgr::sInst)->mNodeCount;
        }
        int NumZones = StreamZoneManager::sInst->GetNumZones();
        if (NumZones > 0)
            v3 = NumZones * (*(PathNodeLevelTOC**)PathNodeMgr::sInst)->mNodeCount;
        sprintf(string, "Num Nodes in Level: %d", v3);
        float color[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
        RE_Text_Paint(416.0f, 412.0f, 5, scaleScalar * 0.55f, color, string,
                      0, 0, 0);
        RE_Text_Paint(408.0f, 410.0f, 5, scaleScalar * 0.55f, m_currentRgba,
                      string, 0, 0, 0);
        sprintf(string, "Num Nodes in Zone: %d", mNodeCount);
        color[0] = 0.0f;
        color[1] = 0.0f;
        color[2] = 0.0f;
        color[3] = 1.0f;
        RE_Text_Paint(416.0f, 426.0f, 5, scaleScalar * 0.55f, color, string,
                      0, 0, 0);
        RE_Text_Paint(408.0f, 424.0f, 5, scaleScalar * 0.55f, m_currentRgba,
                      string, 0, 0, 0);
    }
    if (g_renderGameEntityStats)
        IM_RenderGameEntityStats();
    if (g_displayPlayerStats)
        RenderPlayerStats();
}

// ============================================================================
// InspectorManager::Update - ea: 0x4F71C0
// Menu navigation + per-item value adjustment.
// ============================================================================
void InspectorManager::Update()
{
    if (!m_data.active)
        return;
    g_GlowIntensity = ShaderCommon::gGlowIntensity;
    g_GlowExpansion = ShaderCommon::gGlowExpansion;
    g_GlowEnable = ShaderCommon::gGlowEnable;
    g_GlowPasses = ShaderCommon::gGlowPasses;
    g_GlowGodRaysEnable = ShaderCommon::gGlowGodRays ? 1 : 0;
    g_GlowBrightness = ShaderCommon::gGlowBrighten;
    FogConfig::GetEnabled(g_FogEnable);
    FogConfig::GetColor(g_FogRed, g_FogGreen, g_FogBlue);
    FogConfig::GetRange(g_FogNear, g_FogFar);
    FogConfig::GetVal(g_FogStart, g_FogEnd);

    _INSPECTOR_MENU_ITEM* currentItem = m_data.currentItem;
    if (currentItem)
    {
        if (m_KEY_UP)
        {
            _INSPECTOR_MENU_ITEM* prev = currentItem->prev;
            if (!prev)
                prev = m_data.currentMenu->last;
            m_data.currentItem = prev;
        }
        if (m_KEY_DOWN)
        {
            _INSPECTOR_MENU_ITEM* next = m_data.currentItem->next;
            if (!next)
                next = m_data.currentMenu->first;
            m_data.currentItem = next;
        }
        _INSPECTOR_MENU_ITEM* v5 = m_data.currentItem;
        float v6;
        float angle;
        int veryFast = 0;
        if ((0x20000 & v5->type) != 0)
        {
            if (m_KEY_LEFT_DEBOUNCE || m_KEY_LEFT_FAST_DEBOUNCE
                || m_KEY_LEFT_VERY_FAST_DEBOUNCE)
            {
                v6 = -1.0f;
                angle = -1.0f;
                if (m_KEY_LEFT_FAST_DEBOUNCE)
                {
                    v6 = -12.0f;
                    angle = -12.0f;
                }
                veryFast = m_KEY_LEFT_VERY_FAST_DEBOUNCE;
            }
            else if (!m_KEY_RIGHT_DEBOUNCE && !m_KEY_RIGHT_FAST_DEBOUNCE
                     && !m_KEY_RIGHT_VERY_FAST_DEBOUNCE)
            {
                v6 = 0.0f;
                angle = v6;
                goto input_done;
            }
            else
            {
                v6 = 1.0f;
                angle = 1.0f;
                if (m_KEY_RIGHT_FAST_DEBOUNCE)
                {
                    v6 = 12.0f;
                    angle = 12.0f;
                }
                veryFast = m_KEY_RIGHT_VERY_FAST_DEBOUNCE;
            }
        }
        else if (m_KEY_LEFT || m_KEY_LEFT_FAST || m_KEY_LEFT_VERY_FAST)
        {
            v6 = -1.0f;
            angle = -1.0f;
            if (m_KEY_LEFT_FAST)
            {
                v6 = -12.0f;
                angle = -12.0f;
            }
            veryFast = m_KEY_LEFT_VERY_FAST;
        }
        else if (!m_KEY_RIGHT && !m_KEY_RIGHT_FAST && !m_KEY_RIGHT_VERY_FAST)
        {
            v6 = 0.0f;
            angle = v6;
            goto input_done;
        }
        else
        {
            v6 = 1.0f;
            angle = 1.0f;
            if (m_KEY_RIGHT_FAST)
            {
                v6 = 12.0f;
                angle = 12.0f;
            }
            veryFast = m_KEY_RIGHT_VERY_FAST;
        }
        if (veryFast)
            v6 = v6 * 80.0f;
        angle = v6;
    input_done:
        int v8 = (int)v6;
        if ((0x10000 & v5->type) == 0)
        {
            switch (v5->type & 0xFFFF)
            {
            case 0u:  // submenu
                if (m_KEY_SELECT)
                {
                    m_data.currentMenu->lastCurrentItem = v5;
                    _INSPECTOR_MENU* vp = (_INSPECTOR_MENU*)v5->value.vp;
                    m_data.currentMenu = vp;
                    m_data.currentItem = vp->lastCurrentItem;
                }
                break;
            case 1u:  // int adjust
                *v5->value.ip += v8;
                if ((0x40000 & v5->type) != 0)
                {
                    if (*v5->value.ip < 0)
                        *v5->value.ip = 0;
                }
                if ((0x80000 & v5->type) != 0)
                {
                    if (*v5->value.ip > 0)
                        *v5->value.ip = 1;
                }
                break;
            case 2u:  // int toggle
                if (m_KEY_SELECT)
                    *v5->value.ip ^= 1u;
                break;
            case 3u:
                *v5->value.fp = *v5->value.fp + v6;
                goto clamp_float;
            case 4u:
                *v5->value.fp = (v6 * 5.0f) + *v5->value.fp;
                goto clamp_float;
            case 5u:
                *v5->value.fp = (v6 * 1000.0f) + *v5->value.fp;
                goto clamp_float;
            case 6u:
                *v5->value.fp = (v6 * 0.05f) + *v5->value.fp;
                goto clamp_float;
            case 7u:
                *v5->value.fp = (v6 * 0.005f) + *v5->value.fp;
                goto clamp_float;
            case 8u:
                *v5->value.fp = (v6 * 0.0002f) + *v5->value.fp;
            clamp_float:
                if ((0x40000 & v5->type) != 0)
                {
                    if (*v5->value.fp < 0.0f)
                        *v5->value.fp = 0.0f;
                }
                if ((0x80000 & v5->type) != 0)
                {
                    if (*v5->value.fp > 1.0f)
                        *v5->value.fp = 1.0f;
                }
                break;
            case 9u:
                *v5->value.fp = (v6 * 0.005f) + *v5->value.fp;
                if (*v5->value.fp < 0.0f)
                    *v5->value.fp = 0.0f;
                if (*v5->value.fp > 1.0f)
                    *v5->value.fp = 1.0f;
                break;
            case 0xAu:
                *v5->value.fp = (v6 * 0.01f) + *v5->value.fp;
                if (*v5->value.fp < 0.0f)
                    *v5->value.fp = 0.0f;
                if (*v5->value.fp > 2.0f)
                    *v5->value.fp = 2.0f;
                break;
            case 0xBu:
                *v5->value.fp = *v5->value.fp + v6;
                if (*v5->value.fp < 0.0f)
                    *v5->value.fp = 0.0f;
                if (*v5->value.fp > 255.0f)
                    *v5->value.fp = 255.0f;
                break;
            case 0xCu:  // angle (radians)
            {
                float* v10 = v5->value.fp;
                float v11 = (float)fmod(angle * 0.017453292f + *v10
                                        + 251.32741f, 6.283185482025146f);
                float v12 = v11;
                if (v11 >= 3.1415927f)
                    v12 = v11 - 6.2831855f;
                *v10 = v12;
                break;
            }
            case 0xDu:  // angle (degrees)
            {
                float* v13 = v5->value.fp;
                float v14 = (float)fmod(*v13 * 3.1415927f * 0.0055555557f
                                        + angle * 0.017453292f + 251.32741f,
                                        6.283185482025146f);
                float v15 = v14;
                if (v14 >= 3.1415927f)
                    v15 = v14 - 6.2831855f;
                *v13 = (v15 * 180.0f) * 0.31830987f;
                break;
            }
            case 0xEu:
                if (m_KEY_SELECT)
                    *v5->value.ip = 1;
                break;
            case 0xFu:
                *v5->value.ucp += v8;
                break;
            case 0x10u:
                if (m_KEY_SELECT)
                    *v5->value.ucp ^= 1u;
                break;
            case 0x11u:
                if (m_KEY_SELECT)
                    v5->value.fn();
                break;
            case 0x12u:
                if (m_KEY_SELECT)
                {
                    _INSPECTOR_MENU* currentMenu = m_data.currentMenu;
                    _INSPECTOR_MENU_ITEM* first = currentMenu->first;
                    int v31 = 0;
                    if (first != currentMenu->last)
                    {
                        do
                        {
                            if (first == v5)
                                break;
                            first = first->next;
                            ++v31;
                        } while (first != m_data.currentMenu->last);
                    }
                    ((void(*)(int))v5->value.vp)(v31);
                }
                break;
            case 0x16u:
            case 0x17u:
                ((void(*)(float))v5->value.vp)(v6);
                break;
            case 0x18u:
                ((void(*)(float))v5->value.vp)(v6 * 0.005f);
                break;
            default:
                break;
            }
        }
    }
    if (m_KEY_MENU_BACK)
    {
        _INSPECTOR_MENU* v32 = m_data.currentMenu;
        if (v32->parentMenu)
        {
            v32->lastCurrentItem = m_data.currentItem;
            _INSPECTOR_MENU* parentMenu = m_data.currentMenu->parentMenu;
            m_data.currentMenu = parentMenu;
            m_data.currentItem = parentMenu->lastCurrentItem;
        }
        else
        {
            m_data.active = 0;
        }
    }
    ShaderCommon::gGlowIntensity = g_GlowIntensity;
    ShaderCommon::gGlowExpansion = g_GlowExpansion;
    ShaderCommon::gGlowBrighten = g_GlowBrightness;
    ShaderCommon::gGlowPasses = g_GlowPasses;
    ShaderCommon::gGlowGodRays = g_GlowGodRaysEnable != 0;
    ShaderCommon::gGlowEnable = g_GlowEnable;
    if (g_FogNear > g_FogFar)
        g_FogNear = g_FogFar;
    if (g_FogStart > g_FogEnd)
        g_FogStart = g_FogEnd;
    FogConfig::SetColor(g_FogRed, g_FogGreen, g_FogBlue);
    FogConfig::SetRange(g_FogNear, g_FogFar);
    FogConfig::SetVal(g_FogStart, g_FogEnd);
    FogConfig::SetEnabled(g_FogEnable);
}

// ============================================================================
// InspectorManager::Initialise - ea: 0x50E250
// ============================================================================
void InspectorManager::Initialise()
{
    m_data.active = 0;
    m_data.rootMenu = &g_inspectorRootMenu;
    m_data.currentMenu = &g_inspectorRootMenu;
    m_data.currentItem = nullptr;
    SetupUserMenus();
}

// ============================================================================
// InspectorManager::Render - ea: 0x50BED0
// ============================================================================
void InspectorManager::Render()
{
    if (gScreenshotInProgress != 0
        || cgGlobal.cubemapShot != CUBEMAPSHOT_NONE)
        return;
    nglListBeginScene(NGLSCENE_PARENT);
    g_debugThread.Render();
    UserRenderHook();
    if (m_data.active == 0)
    {
        Print((char*)sBuildId, 15, 40, 0.55f);
        nglListEndScene();
        return;
    }
    if (m_data.currentItem == nullptr)
    {
        Print((char*)"Empty Menu", 40, 40, 0.55f);
        return;
    }
    m_currentRgba[0] = m_headingRgba[0];
    m_currentRgba[1] = m_headingRgba[1];
    m_currentRgba[2] = m_headingRgba[2];
    m_currentRgba[3] = m_headingRgba[3];
    char insp_s[208];
    char insp_val[208];
    char selected[4];
    _INSPECTOR_MENU* currentMenu = m_data.currentMenu;
    sprintf(insp_s, "---- %s ----", currentMenu->heading);
    Print(insp_s, 40, 40, 0.55f);
    m_currentRgba[0] = m_textRgba[0];
    m_currentRgba[1] = m_textRgba[1];
    m_currentRgba[2] = m_textRgba[2];
    m_currentRgba[3] = m_textRgba[3];
    _INSPECTOR_MENU_ITEM* first = m_data.currentMenu->first;
    int v11 = 57;
    if (first == nullptr)
    {
        nglListEndScene();
        return;
    }
    do
    {
        *selected = first != m_data.currentItem ? 32 : 62;
        switch (first->type & 0xFFFF)
        {
        case 0u:
            sprintf(insp_s, "%s %s", selected, first->text);
            sprintf(insp_val, "->");
            break;
        case 1u:
            sprintf(insp_s, "%s %s", selected, first->text);
            sprintf(insp_val, "%d", *first->value.ip);
            break;
        case 2u:
            sprintf(insp_s, "%s %s", selected, first->text);
            sprintf(insp_val, "%s", *first->value.ip == 0 ? "OFF" : "ON");
            break;
        case 3u:
        case 4u:
        case 5u:
        case 6u:
        case 7u:
        case 8u:
        case 0xBu:
            sprintf(insp_s, "%s %s", selected, first->text);
            sprintf(insp_val, "%g", *first->value.fp);
            break;
        case 9u:
        case 0xAu:
            sprintf(insp_s, "%s %s", selected, first->text);
            sprintf(insp_val, "%1.3f", *first->value.fp);
            break;
        case 0xCu:
        {
            float v = (float)fmod(*first->value.fp + 251.32741f,
                                  6.283185482025146f) * 180.0f * 0.31830987f;
            if (v > 180.0f)
                v = v - 360.0f;
            sprintf(insp_s, "%s %s", selected, first->text);
            sprintf(insp_val, "%4.2f", v);
            break;
        }
        case 0xDu:
        {
            float v = (float)fmod(*first->value.fp * 3.1415927f * 0.0055555557f
                                  + 251.32741f,
                                  6.283185482025146f) * 180.0f * 0.31830987f;
            if (v > 180.0f)
                v = v - 360.0f;
            sprintf(insp_s, "%s %s", selected, first->text);
            sprintf(insp_val, "%4.2f", v);
            break;
        }
        case 0xEu:
            sprintf(insp_s, "%s %s", selected, first->text);
            sprintf(insp_val, "%s", *first->value.ip == 0 ? "-" : "SELECTED");
            break;
        case 0xFu:
            sprintf(insp_s, "%s %s", selected, first->text);
            sprintf(insp_val, "%d", *first->value.ucp);
            break;
        case 0x10u:
            sprintf(insp_s, "%s %s", selected, first->text);
            sprintf(insp_val, "%s", *first->value.ucp == 0 ? "OFF" : "ON");
            break;
        case 0x11u:
        case 0x12u:
            sprintf(insp_s, "%s %s", selected, first->text);
            insp_val[0] = 0;
            break;
        case 0x16u:
        case 0x17u:
            sprintf(insp_s, "%s %s", selected, first->text);
            sprintf(insp_val, "%d", first->value.ifn(0));
            break;
        case 0x18u:
            sprintf(insp_s, "%s %s", selected, first->text);
            sprintf(insp_val, "%g", first->value.ffn(0));
            break;
        default:
            break;
        }
        Print(insp_s, 40, v11, 0.55f);
        if (insp_val[0] != 0)
            Print(insp_val, 290, v11, 0.55f);
        first = first->next;
        v11 += 17;
    } while (first != nullptr);
    nglListEndScene();
}
