// ============================================================================
// g_scr_vehicle.cpp - scripted vehicle implementation (g.o family)
// ============================================================================

#include "game/logic/g_local.h"

#include <math.h>
#include <new>
#include <stdio.h>
#include <string.h>

extern void DObjGetBounds(const DObj* obj, math::Position3& mins,
                          math::Position3& maxs);

struct vehicle_backup_s_local
{
    vehicle_pathpos_t pathPos;
    scr_vehicle_t::vehicle_physic_t phys;
};
static vehicle_backup_s_local s_backup; // @ 0xEE60A0 (bss, g_scr_vehicle.cpp local)

extern math::Position3 kVehSaftyMaxs;
extern math::Position3 kVehSaftyMins;
extern vmCvar_t g_vehicleTexScrollScale;

// ea: 0x705010 (physics.o)
rb_vehicle* GetPlayerRBVehicle()
{
    unsigned int mVal = EntityManager::sInst->GetPlayer(currCl)
                            ->client->ps.mViewLockedEntity.mHandle.mVal;
    unsigned int v1 = mVal & 0xFFF;
    if (v1 < 0x540
        && (mVal >> 12)
               == (unsigned int)EntityHandleDb::sInst.mElements[v1].mKey)
    {
        Entity* mObject = EntityHandleDb::sInst.mElements[v1].mObject;
        if (mObject != nullptr && mObject->scr_vehicle != nullptr)
            return (rb_vehicle*)mObject->scr_vehicle->mRBVeh;
    }
    return nullptr;
}
void rb_vehicle_debug_render_all()
{
    rb_vehicle::debug_render_all();
}
void rb_vehicle_unpause_physics(rb_vehicle* self)
{
    if (self != nullptr)
        self->unpause_physics();
}
void rb_vehicle_update_from_network(rb_vehicle* self,
                                    math::Position3* position,
                                    math::Position3* angles, math::Dir3* vel,
                                    math::Dir3* aVel)
{
    if (self != nullptr && position != nullptr && angles != nullptr
        && vel != nullptr && aVel != nullptr)
        self->update_from_network(*position, *angles, *vel, *aVel);
}

static float VEH_LerpAngle(float targetAngle, float currentAngle, float rate);

extern void VEH_UpdatePO(Entity* ent, char* move, int msec);
extern void VEH_GroundTrace(Entity* ent);
extern void VEH_GroundMove(Entity* ent, int msec);
extern int VEH_Slide(Entity* ent, int gravity, int msec, int move,
                    int allowHit);
extern void VEH_UpdateSoundLerps(Entity* ent, int msec);
extern void VEH_FireGunnerWeapon(Entity* ent, int msec);
extern bool AttachCurveVehicle(unsigned int entityHandleVal, char* filename,
                               float topSpeed, float topSpeedReverse);
extern void TraceSphereFull(const proximity_data_t* proximity_data,
                            trace_t* results, const math::Position3* start,
                            const math::Position3* mins,
                            const math::Position3* maxs,
                            const math::Position3* end,
                            const collision_context_t* context);
extern float veh_radius;
extern float vehicleDeadZone;
extern float delta_yaw_vel;
extern unsigned char unk_F6A294[4 * 3208];
extern const float AngleNormalize180Accurate(float angle);
float decay = 5.0f; // IDA global @ 0xDD8250

// Minimal controller view (controller_xboxr). The function-pointer block is
// retained so locked_port/is_locked keep their IDA-verified offsets.
class controller {
public:
    enum ButtonIndex {
        L3 = 13,
    };
    static controller* inst();
    int button_value(int controller, ButtonIndex button);
    void (*button_value_fn)(int*);
    void (*button_released_fn)(int*);
    void (*button_released_clear_fn)(int*);
    void (*button_pressed_fn)(int*);
    void (*button_pressed_clear_fn)(int*);
    void (*stick_value_fn)(int*, int*);
    int locked_port;
    bool is_locked;
    bool accepting_input_from_controller[4];
};

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
        ae_sized_array<ae_sized_array<EPadAliasButton, 16>, 4> mButtonAlias;
        ae_sized_array<ae_sized_array<EPadAliasStick, 2>, 4> mStickAlias;
        int GetButtonValue(int ctrlNum, EPadAliasButton buttonAlias);
    };
    Context mCtx[3];
    static PadAliasMgr* sInst;
};

extern int dword_F6A28C[4 * 802];
extern int g_vehicle_button_threshold;

// ea: 0x0048FA30
void VEH_FireAltWeapon(Entity* ent)
{
    scr_vehicle_t* scr_vehicle = ent->scr_vehicle;
    if (!scr_vehicle->seats[0].gunMounted)
        return;

    weaponFileInfo_t* info = BG_GetInfoForWeapon(scr_vehicle->altWeapon);
    scr_vehicle->altFireTime = info->iFireTime;

    weaponParms wp;
    memset(&wp, 0, sizeof(wp));
    wp.pWeapInfo = info;

    int coax = scr_vehicle->boneIndex.coax;
    if (coax < 0)
        coax = scr_vehicle->boneIndex.flash[0];

    DObjSkelMat flashMtx;
    G_DObjGetWorldBoneIndexMatrix(ent, coax, &flashMtx);

    unsigned int ownerValue = ent->r.mOwner.mHandle.mVal;
    unsigned int ownerIndex = ownerValue & 0xFFF;
    Entity* owner = nullptr;
    if (ownerIndex < 0x540
        && (ownerValue >> 12)
               == (unsigned int)EntityHandleDb::sInst.mElements[ownerIndex].mKey)
        owner = EntityHandleDb::sInst.mElements[ownerIndex].mObject;

    float muzzleOrigin[3] = {
        flashMtx.origin[0], flashMtx.origin[1], flashMtx.origin[2]
    };
    float gunAngles[3] = {
        flashMtx.axis[0][0], flashMtx.axis[0][1], flashMtx.axis[0][2]
    };
    float muzzleAngles[3];
    vectoangles(gunAngles, muzzleAngles);
    if (owner != nullptr && scr_vehicle->barrelBlocked == 0
        && (scr_vehicle->targetOrigin[0] != 0.0f
            || scr_vehicle->targetOrigin[1] != 0.0f
            || scr_vehicle->targetOrigin[2] != 0.0f))
    {
        wp.muzzleTrace[0] = scr_vehicle->targetOrigin[0] - muzzleOrigin[0];
        wp.muzzleTrace[1] = scr_vehicle->targetOrigin[1] - muzzleOrigin[1];
        wp.muzzleTrace[2] = scr_vehicle->targetOrigin[2] - muzzleOrigin[2];
        VectorNormalize(wp.muzzleTrace);

        float targetAngles[3];
        vectoangles(wp.muzzleTrace, targetAngles);
        math::Position3 muzzlePos = native_to_cdl_pos3(muzzleAngles);
        math::Position3 targetPos = native_to_cdl_pos3(targetAngles);
        math::Position3 angleDelta;
        AnglesSubtract(muzzlePos, targetPos, angleDelta);
        float pitch = angleDelta.v.m128_f32[2];
        if (pitch < -10.0f)
            pitch = -10.0f;
        else if (pitch > 10.0f)
            pitch = 10.0f;
        angleDelta.v.m128_f32[2] = pitch;

        math::Position3 adjustedAngles;
        AnglesSubtract(muzzlePos, angleDelta, adjustedAngles);
        AnglesToForward(adjustedAngles.v.m128_f32, wp.muzzleTrace);
        gunAngles[0] = wp.muzzleTrace[0];
        gunAngles[1] = wp.muzzleTrace[1];
        gunAngles[2] = wp.muzzleTrace[2];
    }

    wp.forward[0] = flashMtx.axis[1][0];
    wp.forward[1] = flashMtx.axis[1][1];
    wp.forward[2] = flashMtx.axis[1][2];
    wp.right[0] = flashMtx.axis[2][0];
    wp.right[1] = flashMtx.axis[2][1];
    wp.right[2] = flashMtx.axis[2][2];
    if (scr_vehicle->barrelBlocked != 0)
    {
        wp.up[0] = (0.0f - scr_vehicle->barrelOffset) * gunAngles[0]
                 + muzzleOrigin[0];
        wp.up[1] = (0.0f - scr_vehicle->barrelOffset) * gunAngles[1]
                 + muzzleOrigin[1];
        wp.up[2] = (0.0f - scr_vehicle->barrelOffset) * gunAngles[2]
                 + muzzleOrigin[2];
    }
    else
    {
        wp.up[0] = muzzleOrigin[0];
        wp.up[1] = muzzleOrigin[1];
        wp.up[2] = muzzleOrigin[2];
    }

    unsigned int occupantValue = scr_vehicle->seats[0].occupant.mHandle.mVal;
    unsigned int occupantIndex = occupantValue & 0xFFF;
    if (occupantIndex >= 0x540
        || (occupantValue >> 12)
               != (unsigned int)EntityHandleDb::sInst.mElements[occupantIndex].mKey
        || EntityHandleDb::sInst.mElements[occupantIndex].mObject == nullptr)
        return;

    Entity* occupant = EntityHandleDb::sInst.mElements[occupantIndex].mObject;
    if (info->type != WEAPTYPE_BULLET)
    {
        Weapon_RocketLauncher_Fire(ent, 0.0f, &wp, 10.0f, true);
        return;
    }

    float spread = (info->accuracy - 1.0f) * 10.0f;
    int damage = info->iDamage;
    if (EntityManager::sInst->IsLocalPlayer(occupant))
        Bullet_Fire(occupant, spread, damage, &wp, ent, 0.0f);
    else
        Bullet_Fire_Fake(occupant, 0.0f, damage, &wp, ent, 0.0f);

    CG_FireWeapon(occupant, &occupant->s, 187, 0);
    scr_vehicle->seats[0].heat +=
        (info->iFireTime * info->fFireHeat) * 0.00075000001f;

    for (int i = 0; i < 11; ++i)
    {
        unsigned int value = scr_vehicle->seats[i].occupant.mHandle.mVal;
        unsigned int index = value & 0xFFF;
        if (index >= 0x540
            || (value >> 12)
                   != (unsigned int)EntityHandleDb::sInst.mElements[index].mKey)
            continue;
        Entity* passenger = EntityHandleDb::sInst.mElements[index].mObject;
        if (passenger == nullptr
            || !EntityManager::sInst->IsLocalPlayer(passenger))
            continue;

        int client = passenger->client->mServerClientIndex;
        void* rumble = RumbleManager_Inst(client);
        if (rumble == nullptr)
            continue;

        RumbleEffect effect;
        effect.mRumbleDataArray[0].enabled = true;
        effect.mRumbleDataArray[0].delay = 0.5f;
        effect.mRumbleDataArray[1].delay = 0.2f;
        effect.mRumbleDataArray[1].enabled = false;
        effect.mRumbleDataArray[1].intensity = 0.0f;
        RumbleEffect_SetIntensity(&effect, kRumbleLEFT, 0.5f);
        float intensity;
        if (i != 0)
        {
            RumbleEffect_SetIntensity(&effect, kRumbleRIGHT, 0.2f);
            intensity = 0.5f;
        }
        else
        {
            RumbleEffect_SetIntensity(&effect, kRumbleRIGHT, 0.5f);
            intensity = 1.0f;
        }
        RumbleManager_Play(rumble, &effect, intensity);
    }
}

// VEH_* free artifacts (g.o)
struct scr_vehicle_t;
void VEH_InitEntity(Entity* ent, scr_vehicle_t* veh, short a)
{
    vehicle_info_t* info = s_vehicleInfos[a];
    ent->die = 4;
    ent->touch = 4;
    ent->pain = 3;
    ent->use = 3;
    ent->controller = 2;
    ent->entinfo = 2;
    ent->think = THINK__Scr_Vehicle_Init;
    ent->r.svFlags = 16;
    ent->r.contents = byte_A00000;
    ent->s.eType = 14;
    ent->s.eFlags = 0;
    ent->s.pos.trType = TR_INTERPOLATE;
    ent->s.apos.trType = TR_INTERPOLATE;
    ent->s.loopSound = 0;
    ent->s.weapon = BG_GetWeaponIndexForName(info->turretWeapon);

    veh->next.mSteeringAngle = 0.0f;
    veh->next.mTurretAngles.v.m128_f32[0] = 0.0f;
    veh->next.mTurretAngles.v.m128_f32[1] = 0.0f;
    veh->next.mTurretAngles.v.m128_f32[2] = 0.0f;
    veh->next.mBodyPosition.v.m128_f32[0] = 0.0f;
    veh->next.mBodyPosition.v.m128_f32[1] = 0.0f;
    veh->next.mBodyPosition.v.m128_f32[2] = 0.0f;
    veh->current.mSteeringAngle = 0.0f;
    veh->current.mTurretAngles.v.m128_f32[0] = 0.0f;
    veh->current.mTurretAngles.v.m128_f32[1] = 0.0f;
    veh->current.mTurretAngles.v.m128_f32[2] = 0.0f;
    veh->current.mBodyPosition.v.m128_f32[0] = 0.0f;
    veh->current.mBodyPosition.v.m128_f32[1] = 0.0f;
    veh->current.mBodyPosition.v.m128_f32[2] = 0.0f;
    veh->numWaitNotify = 0;
    veh->treadTime = 0.0f;
    veh->treadTime2 = 0.0f;
    veh->mUseRadius = 128;
    veh->mHasEntryPoints = false;

    int flags = ent->flags;
    proximity_data_t* proximity_data = ent->proximity_data;
    ent->scr_vehicle = veh;
    ent->nextthink = level.time + 1;
    ent->takedamage = 1;
    ent->speed = 0.0f;
    ent->active = 0;
    ent->clipmask = 0x810251;
    ent->flags = flags | 0x1000;

    if (proximity_data == nullptr)
    {
        TPakId pakId = (TPakId)ent->mPakId;
        if (pakId == PAK_ID_INVALID)
            pakId = CurPakId();
        proximity_data = (proximity_data_t*)proximity_data_t::operator new(
            0x1850u, pakId);
        if (proximity_data != nullptr)
            ::new (proximity_data) proximity_data_t();
        ent->proximity_data = proximity_data;
    }

    ent->health = info->health;
    ent->maxHealth = info->health;
    veh->animMap = vehicleAnimMaps[info->type];
    G_DObjUpdate(ent, false);
    if (ent->mDObj == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile =
            "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 1415;
        AeAssert::gCurrentExpr = "ent->GetDObj()";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Vehicle entity does not have an object - bad!"))
            __debugbreak();
    }
    DObj* mDObj = ent->mDObj;
    if (mDObj != nullptr)
        DObjGetBounds(mDObj, ent->r.mins, ent->r.maxs);
    g_LinkEntity(ent);
    RegisterItem(ent->s.weapon, 1);
}
// ea: 0x0044D7C0
void VEH_InitPhysics(Entity* ent)
{
    scr_vehicle_t* veh = ent->scr_vehicle;
    for (int i = 0; i < 3; ++i)
    {
        veh->phys.origin.v.m128_f32[i] = ent->r.currentOrigin.v.m128_f32[i];
        veh->phys.prevOrigin.v.m128_f32[i] = ent->r.currentOrigin.v.m128_f32[i];
        veh->phys.angles.v.m128_f32[i] = ent->r.currentAngles.v.m128_f32[i];
        veh->phys.prevAngles.v.m128_f32[i] = ent->r.currentAngles.v.m128_f32[i];
        veh->phys.vel.v.m128_f32[i] = 0.0f;
        veh->phys.rotVel.v.m128_f32[i] = 0.0f;
    }
    for (int i = 0; i < 6; ++i)
    {
        veh->phys.wheelZVel[i] = 0.0f;
        veh->phys.wheelZPos[i] = 0.0f;
        veh->phys.wheelSurfType[i] = 0;
        veh->wheel_polies[i * 0x50 + 0x44] = 0;
    }
}

// ea: 0x00487C30
void VEH_InitVehicle(Entity* ent, scr_vehicle_t* veh, int16_t infoIdx)
{
    vehicle_info_t* info = s_vehicleInfos[infoIdx];

    G_VehInitPathPos(&veh->pathPos);
    VEH_InitPhysics(ent);
    veh->mEntity.mHandle.mVal = ent->mHandle.mHandle.mVal;
    veh->infoIdx = infoIdx;
    veh->waitSpeed = -1.0f;
    veh->waitNode = -1;
    veh->fireTime = 0;
    veh->fireBarrel = 0;
    veh->turretState = 0;
    veh->drawOnCompass = 0;
    veh->drawAsEnemy = 0;
    veh->barrelOffset = 0.0f;
    veh->barrelBlocked = 0;
    veh->altWeapon = 0;
    veh->gunnerWeapon = 0;
    veh->shooter = 0;
    veh->noEntryTime = 0;

    if (info->vehicleCurveFile[0] != 0)
    {
        AttachCurveVehicle(ent->mHandle.mHandle.mVal, info->vehicleCurveFile,
                           info->vehicleSndTopSpeed,
                           info->vehicleSndTopSpeedReverse);
    }

    veh->mHatchOpen = true;
    if (info->type == 2)
    {
        veh->current.mHatchAngleRight = info->hatchOpenAngleRight;
        veh->current.mHatchAngleLeft = info->hatchOpenAngleLeft;
    }

    int seatIndex = 0;
    for (int i = 0; i < 11; ++i)
    {
        vehicleSeat_t& seat = veh->seats[i];
        seat.occupant.mHandle.mVal = 0;

        int boneIndex = -1;
        if (seatIndex < info->numSeats)
        {
            boneIndex = SV_DObjGetBoneIndex(ent, s_seatTagHashes[i]);
        }
        else if (i >= 8)
        {
            const int previousIndex = i - 1;
            if (previousIndex < 0)
            {
                AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
                AeAssert::gCurrentFile =
                    "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
                AeAssert::gCurrentLine = 1516;
                AeAssert::gCurrentExpr = "(j ) >= 0";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert("Invalid seat index"))
                    __debugbreak();
            }
            boneIndex = SV_DObjGetBoneIndex(ent, s_seatTagHashes[previousIndex]);
        }

        seat.boneIndex = boneIndex;
        seat.gunMounted = false;
        seat.weapon = 0;
        seat.heat = 0.0f;
        seat.overheating = false;
        seat.flags = 0;
        seat.firing = false;
        if (boneIndex >= 0)
            ++seatIndex;
    }

    veh->altWeapon = 0;
    veh->gunnerWeapon = 0;
    if (veh->seats[0].boneIndex < 0)
        veh->seats[0].boneIndex = 0;
    veh->spotTime = 0;
    veh->mLastSpotter.mHandle.mVal = 0;
    veh->mMantleTime = 0;
    veh->mMantleEntity.mHandle.mVal = 0;
    veh->mLastRequestedOwnershipTime = 0;

    if (info->turretAltWeapon[0] != 0)
    {
        unsigned char weapon = BG_GetWeaponIndexForName(info->turretAltWeapon);
        veh->altWeapon = weapon;
        veh->seats[0].weapon = weapon;
    }

    G_EntDetachAll(ent);
    if (info->turretAltModel[0] != 0)
    {
        TPakId pakId = (TPakId)ent->mPakId;
        if (pakId == PAK_ID_INVALID)
            pakId = CurPakId();
        if (G_EntAttach(ent, info->turretAltModel, "tag_guncoax", 1, pakId))
            veh->seats[0].gunMounted = true;
    }
    if (info->turretGunnerModel[0] != 0)
    {
        TPakId pakId = (TPakId)ent->mPakId;
        if (pakId == PAK_ID_INVALID)
            pakId = CurPakId();
        if (G_EntAttach(ent, info->turretGunnerModel,
                        "tag_gunner_barrel", 1, pakId))
            veh->seats[1].gunMounted = true;
    }
    if (info->turretGunnerBaseModel[0] != 0)
    {
        TPakId pakId = (TPakId)ent->mPakId;
        if (pakId == PAK_ID_INVALID)
            pakId = CurPakId();
        G_EntAttach(ent, info->turretGunnerBaseModel,
                    "tag_gunner_turret", 1, pakId);
    }

    veh->manualMode = 0;
    veh->hasTarget = 0;
    veh->manualSpeed = 0.0f;
    veh->manualAccel = 0.0f;
    veh->manualTime = 0.0f;
    veh->wheelRadius = 15.0f;
    veh->mTargetEnt.mHandle.mVal = 0;
    for (int i = 0; i < 3; ++i)
    {
        veh->targetOrigin[i] = 0.0f;
        veh->targetOffset[i] = 0.0f;
    }
    veh->joltDir[0] = 0.0f;
    veh->joltDir[1] = 0.0f;
    veh->joltTime = 0.0f;
    veh->joltWave = 0.0f;
    veh->mIdleSndEnt.mHandle.mVal = 0;
    veh->mEngineSndEnt.mHandle.mVal = 0;
    veh->turretHitNum = 0;
    veh->idleSndLerp = 0.0f;
    veh->engineSndLerp = 0.0f;
    veh->brakeSndLerp = 0.0f;
    for (int i = 0; i < 6; ++i)
    {
        veh->mWheel_ParticleEffectHandle[i].mVal = 0;
        veh->mSoundEffectHandle[i].mVal = 0;
    }
    veh->mPhysicsOwner.mHandle.mVal = 0;
    veh->playersAttached = 0;
    veh->mRumbleEffectHandle.mVal = 0;

    math::Position3 zeroVelocity = {};
    VEH_SetPosition(ent, ent->r.currentOrigin, ent->r.currentAngles,
                    zeroVelocity);

    static unsigned int wheelFrontLeftHash = 0;
    static bool wheelFrontLeftHashInitialized = false;
    if (!wheelFrontLeftHashInitialized)
    {
        wheelFrontLeftHash = HashString::CalcHash("tag_wheel_front_left");
        wheelFrontLeftHashInitialized = true;
    }
    if (ent->mDObj != nullptr && ent->mDObj->skel != nullptr)
    {
        int boneIndex = DObjGetBoneIndex(ent->mDObj, wheelFrontLeftHash);
        if (boneIndex >= 0)
        {
            DObjSkelMat* matrixArray = DObjGetMatrixArray(ent->mDObj, 0);
            if (matrixArray != nullptr)
                veh->wheelRadius = matrixArray[boneIndex].origin[2];
        }
    }

    if (veh->mRBVeh != nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::JRS;
        AeAssert::gCurrentFile =
            "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 1640;
        AeAssert::gCurrentExpr = "!veh->mRBVeh";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Vehicle never had physics removed. Tell James."))
            __debugbreak();
    }

    veh->mRBVeh = nullptr;
    if (info->vehiclePhysicsParms[0] != 0)
    {
        rb_vehicle* rbVehicle = rb_vehicle::add_vehicle();
        veh->mRBVeh = rbVehicle;
        if (rbVehicle != nullptr)
        {
            vehicle_rb_parameter* parms =
                vehicle_rb_parameter::GetRBVehParameter(info->vehiclePhysicsParms);
            if (parms == nullptr)
            {
                AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
                AeAssert::gCurrentFile =
                    "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
                AeAssert::gCurrentLine = 1650;
                AeAssert::gCurrentExpr = "parms";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert("Failed to find vehicle physics settings for %s",
                                        info->vehiclePhysicsParms))
                    __debugbreak();
            }
            rbVehicle->init(ent, parms);
            TPakId pakId = (TPakId)ent->mPakId;
            if (pakId == PAK_ID_INVALID)
                pakId = CurPakId();
            IVPointer<Destructible> destructible =
                DestructibleBankManager::sInst->GetDestructible(pakId, "vehicle");
            ent->mDestructible = destructible;
            ent->takedamage = 1;
        }
    }

    if (info->turretGunnerWeapon[0] != 0)
    {
        unsigned char weapon = BG_GetWeaponIndexForName(info->turretGunnerWeapon);
        veh->gunnerWeapon = weapon;
        veh->seats[1].weapon = weapon;
    }
    veh->follow = nullptr;
}
// ea: 0x0047D940
void VEH_UpdateAim(Entity* ent)
{
    scr_vehicle_t* scr_vehicle = ent->scr_vehicle;
    vehicle_info_t* info = s_vehicleInfos[scr_vehicle->infoIdx];
    if (info->type == 1)
        return;

    Entity* owner = HandleDbToEnt(ent->r.mOwner);
    if (scr_vehicle->hasTarget == 0 || ent->health <= 0 || owner == nullptr
        || !IsPlayerFullySeatedInVehicle(owner) || owner->client == nullptr
        || owner->client->mVehicleAnimPauseRemoteAngles
        || owner->client->ps.vehPos != 0)
    {
        if (scr_vehicle->turretState == 2)
            scr_vehicle->turretState = 1;
        else if (scr_vehicle->turretState == 1)
            scr_vehicle->turretState = 0;
        return;
    }

    Entity* tgtEnt = HandleDbToEnt(scr_vehicle->mTargetEnt);
    float targetPos[3];
    if (tgtEnt != nullptr)
    {
        targetPos[0] = tgtEnt->r.currentOrigin.v.m128_f32[0]
                     + scr_vehicle->targetOffset[0];
        targetPos[1] = tgtEnt->r.currentOrigin.v.m128_f32[1]
                     + scr_vehicle->targetOffset[1];
        targetPos[2] = tgtEnt->r.currentOrigin.v.m128_f32[2]
                     + scr_vehicle->targetOffset[2];
    }
    else
    {
        targetPos[0] = scr_vehicle->targetOrigin[0];
        targetPos[1] = scr_vehicle->targetOrigin[1];
        targetPos[2] = scr_vehicle->targetOrigin[2];
    }

    if (scr_vehicle->boneIndex.barrel < 0)
        return;

    DObjSkelMat barrelMtx;
    G_DObjGetWorldBoneIndexMatrix(ent, scr_vehicle->boneIndex.barrel,
                                  &barrelMtx);

    float vehicleAngles[3] = {
        ent->r.currentAngles.v.m128_f32[0],
        ent->r.currentAngles.v.m128_f32[1],
        ent->r.currentAngles.v.m128_f32[2]
    };
    if ((owner->client->ps.eFlags & 0x200000) == 0
        && info->spClientSeat == 0)
    {
        vehicleAngles[1] = owner->client->ps.viewangles[1];
    }

    float ownerAngles[3] = {
        owner->client->ps.viewangles[0],
        owner->client->ps.viewangles[1],
        0.0f
    };
    float ownerAxis[3][3];
    float vehicleAxis[3][3];
    float inverseVehicleAxis[3][3];
    float relativeAxis[3][3];
    float targetAngles[3];
    AnglesToAxis(ownerAngles, ownerAxis);
    AnglesToAxis(vehicleAngles, vehicleAxis);
    MatrixTranspose(vehicleAxis, inverseVehicleAxis);
    MatrixMultiply(ownerAxis, inverseVehicleAxis, relativeAxis);
    AxisToAngles(relativeAxis, targetAngles);

    scr_vehicle->current.mTurretAngles.v.m128_f32[0] =
        scr_vehicle->next.mTurretAngles.v.m128_f32[0];
    scr_vehicle->current.mTurretAngles.v.m128_f32[1] =
        AngleNormalize180(scr_vehicle->next.mTurretAngles.v.m128_f32[1]);
    scr_vehicle->current.mTurretAngles.v.m128_f32[2] = 0.0f;

    math::Position3 targetAnglePos = native_to_cdl_pos3(targetAngles);
    math::Position3 deltaAngles;
    AnglesSubtract(targetAnglePos, scr_vehicle->current.mTurretAngles,
                   deltaAngles);
    const float absPitch = fabsf(deltaAngles.v.m128_f32[0]);
    const float absYaw = fabsf(deltaAngles.v.m128_f32[1]);

    scr_vehicle->next.mTurretAngles.v.m128_f32[0] = VEH_LerpAngle(
        targetAngles[0], scr_vehicle->current.mTurretAngles.v.m128_f32[0],
        info->turretRotRate);
    scr_vehicle->next.mTurretAngles.v.m128_f32[1] = VEH_LerpAngle(
        targetAngles[1], scr_vehicle->current.mTurretAngles.v.m128_f32[1],
        info->turretRotRate);

    float pitch = -info->turretVertSpanDown;
    if (pitch <= scr_vehicle->next.mTurretAngles.v.m128_f32[0])
    {
        pitch = scr_vehicle->next.mTurretAngles.v.m128_f32[0];
        if (pitch > info->turretVertSpanUp)
            pitch = info->turretVertSpanUp;
    }
    scr_vehicle->next.mTurretAngles.v.m128_f32[0] = pitch;

    float yaw = scr_vehicle->next.mTurretAngles.v.m128_f32[1];
    if (yaw < -info->turretHorizSpanRight)
        yaw = -info->turretHorizSpanRight;
    else if (yaw > info->turretHorizSpanLeft)
        yaw = info->turretHorizSpanLeft;
    scr_vehicle->next.mTurretAngles.v.m128_f32[1] = yaw;

    if (info->type == 5)
    {
        float* viewAngles = owner->client->ps.viewangles;
        float viewPitch = viewAngles[0];
        if (viewPitch < -info->turretVertSpanDown)
            viewPitch = -info->turretVertSpanDown;
        else if (viewPitch > info->turretVertSpanUp)
            viewPitch = info->turretVertSpanUp;
        if (viewPitch != viewAngles[0])
        {
            viewAngles[0] = viewPitch;
            SetClientViewAngle(owner, viewAngles);
        }

        float deltaYaw = AngleNormalize180(
            viewAngles[1] - AngleNormalize360(vehicleAngles[1]));
        if (fabsf(deltaYaw) > info->turretHorizSpanLeft)
        {
            viewAngles[1] = deltaYaw <= 0.0f
                              ? AngleNormalize360(vehicleAngles[1]
                                                  - info->turretHorizSpanLeft)
                              : AngleNormalize360(vehicleAngles[1]
                                                  + info->turretHorizSpanLeft);
            SetClientViewAngle(owner, viewAngles);
        }
    }

    if (fabsf(deltaAngles.v.m128_f32[0]) >= 0.3f
            && scr_vehicle->next.mTurretAngles.v.m128_f32[0] == 0.0f
        || fabsf(deltaAngles.v.m128_f32[1]) >= 0.3f
            && scr_vehicle->next.mTurretAngles.v.m128_f32[1] == 0.0f)
    {
        scr_vehicle->turretState = 2;
    }
    else if (scr_vehicle->turretState == 2)
    {
        scr_vehicle->turretState = 1;
    }
    else if (scr_vehicle->turretState == 1)
    {
        scr_vehicle->turretState = 0;
    }

    if (scr_vehicle->hasTarget != 0 && absPitch < 1.0f && absYaw < 1.0f)
    {
        Scr_Notify(ent, hash_const.turret_on_target, 0);
        if (tgtEnt != nullptr && ((com_frameNumber + ((int)ent >> 5)) & 3) != 0)
        {
            collision_context_t context(ent->mHandle, tgtEnt->mHandle, 1);
            math::Position3 zeroMins;
            math::Position3 zeroMaxs;
            zeroMins.v = _mm_setzero_ps();
            zeroMaxs.v = _mm_setzero_ps();
            math::Position3 start = native_to_cdl_pos3(barrelMtx.origin);
            math::Position3 end = native_to_cdl_pos3(targetPos);
            g_SightTrace(&scr_vehicle->turretHitNum, start, zeroMins,
                         zeroMaxs, end, context);
            if (scr_vehicle->turretHitNum == 0)
                Scr_Notify(ent, hash_const.turret_on_vistarget, 0);
        }
    }
}

// ea: 0x004904B0
void VEH_UpdateAltWeapon(Entity* ent, int msec)
{
    scr_vehicle_t* scr_vehicle = ent->scr_vehicle;
    if (scr_vehicle->altWeapon == 0 || !scr_vehicle->seats[0].gunMounted)
        return;

    unsigned int mVal = ent->r.mOwner.mHandle.mVal;
    unsigned int index = mVal & 0xFFF;
    Entity* owner = nullptr;
    if (index < 0x540
        && (mVal >> 12)
               == (unsigned int)EntityHandleDb::sInst.mElements[index].mKey)
        owner = EntityHandleDb::sInst.mElements[index].mObject;

    if (owner == nullptr)
    {
        scr_vehicle->seats[0].firing = false;
        goto update_alt_fire;
    }
    if (!EntityManager::sInst->IsLocalPlayer(owner))
        goto update_alt_fire;

    Client* client = owner->client;
    if (client == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile =
            "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 4752;
        AeAssert::gCurrentExpr = "client";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (client->ps.vehPos == 0)
    {
        int locked_port = dword_F6A28C[802 * owner->GetPlayerIndex()];
        controller* input = controller::inst();
        if (input->is_locked)
            locked_port = input->locked_port;
        scr_vehicle->seats[0].firing =
            PadAliasMgr::sInst->mCtx[1].GetButtonValue(
                locked_port, kPadAliasButtonFireCoax)
            > g_vehicle_button_threshold;
        if ((owner->client->ps.pm_flags & 0x4000) != 0)
            scr_vehicle->seats[0].firing = false;
        if (!IsPlayerFullySeatedInVehicle(owner))
            scr_vehicle->seats[0].firing = false;
        if (GamePause::IsGamePaused(owner->GetPlayerIndex()))
        {
            scr_vehicle->seats[0].firing = false;
            goto update_alt_fire;
        }
    }

update_alt_fire:
    {
        int remaining = scr_vehicle->altFireTime - msec;
        bool reached = scr_vehicle->altFireTime == msec;
        scr_vehicle->altFireTime = remaining;
        if (remaining < 0 || reached)
        {
            bool firing = scr_vehicle->seats[0].firing;
            scr_vehicle->altFireTime = 0;
            if (firing && !scr_vehicle->seats[0].overheating)
                VEH_FireAltWeapon(ent);
        }
    }
}

// ea: 0x0046D8D0
void VEH_UpdateClient(Entity* ent, int msec)
{
    scr_vehicle_t* scr_vehicle = ent->scr_vehicle;
    vehicle_info_t* info = s_vehicleInfos[scr_vehicle->infoIdx];
    scr_vehicle_t::vehicle_physic_t* phys = &scr_vehicle->phys;
    char move[3] = { 0, 0, 0 };

    Entity* owner = HandleDbToEnt(ent->r.mOwner);
    if (owner != nullptr)
    {
        if (owner->client == nullptr)
        {
            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
            AeAssert::gCurrentFile =
                "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
            AeAssert::gCurrentLine = 5377;
            AeAssert::gCurrentExpr = "player->client";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
                __debugbreak();
        }
        if (IsPlayerFullySeatedInVehicle(owner))
        {
            Entity* physicsOwner = HandleDbToEnt(scr_vehicle->mPhysicsOwner);
            if (EntityManager::sInst->IsLocalPlayer(physicsOwner))
            {
                owner->client->ps.eFlags |= 0x200000;
                Client* client = owner->client;
                if ((client->ps.eFlags & 0x400000) == 0
                    && (client->ps.pm_flags & 0x4000) == 0)
                {
                    move[0] = client->pers.cmd.forwardmove;
                    move[1] = client->pers.cmd.rightmove;
                    move[2] = client->pers.cmd.upmove;
                    if (move[2] > 0)
                        client->ps.eFlags &= ~0x200000u;
                }

                Entity* player = HandleDbToEnt(scr_vehicle->mPhysicsOwner);
                if (GamePause::IsGamePaused(player->GetPlayerIndex()))
                {
                    move[0] = 0;
                    move[1] = 0;
                    move[2] = 0;
                }
                VEH_UpdatePO(ent, move, msec);
            }
        }
    }

    VEH_GroundTrace(ent);
    Entity* physicsOwner = HandleDbToEnt(scr_vehicle->mPhysicsOwner);
    if (EntityManager::sInst->IsLocalPlayer(physicsOwner))
        VEH_GroundMove(ent, msec);

    if (HandleDbToEnt(ent->r.mOwner) != nullptr)
    {
        ent->speed = sqrtf(phys->vel.v.m128_f32[0]
                           * phys->vel.v.m128_f32[0]
                           + phys->vel.v.m128_f32[1]
                                 * phys->vel.v.m128_f32[1]
                           + phys->vel.v.m128_f32[2]
                                 * phys->vel.v.m128_f32[2]);
    }
    else
    {
        phys->vel.v.m128_f32[2] = 0.0f;
        phys->vel.v.m128_f32[1] = 0.0f;
        phys->vel.v.m128_f32[0] = 0.0f;
        ent->speed = 0.0f;
    }

    if (ent->speed < 0.0f)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile =
            "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 5440;
        AeAssert::gCurrentExpr = "ent->speed >= 0.0f";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }

    if (g_vehicleDebug.integer != 0)
        VEH_DebugCapsule(phys->origin.v.m128_f32, 1.0f,
                         info->mins.v.m128_f32[0],
                         info->maxs.v.m128_f32[0], 1.0f, 0.0f);
}

// ea: 0x00463420
void VEH_GroundTrace(Entity* ent)
{
    scr_vehicle_t::vehicle_physic_t* phys = &ent->scr_vehicle->phys;
    static bool initialized = false;
    static TouchEntityData entities;
    if (!initialized)
    {
        initialized = true;
        memset(entities.touch, 0, sizeof(entities.touch));
    }

    proximity_data_t proximity;
    math::Position3 start;
    start.v.m128_f32[0] = phys->origin.v.m128_f32[0];
    start.v.m128_f32[1] = phys->origin.v.m128_f32[1];
    start.v.m128_f32[2] = phys->origin.v.m128_f32[2] + 50.0f;
    start.v.m128_f32[3] = 0.0f;
    math::Position3 end;
    end.v.m128_f32[0] = phys->origin.v.m128_f32[0];
    end.v.m128_f32[1] = phys->origin.v.m128_f32[1];
    end.v.m128_f32[2] = phys->origin.v.m128_f32[2] - 50.0f;
    end.v.m128_f32[3] = 0.0f;
    prepare_collision_objects(ent, start, end, veh_radius, ent->clipmask,
                              proximity, entities);

    math::Position3 mins;
    mins.v.m128_f32[0] = -40.0f;
    mins.v.m128_f32[1] = -40.0f;
    mins.v.m128_f32[2] = 0.0f;
    mins.v.m128_f32[3] = 0.0f;
    math::Position3 maxs;
    maxs.v.m128_f32[0] = 40.0f;
    maxs.v.m128_f32[1] = 40.0f;
    maxs.v.m128_f32[2] = 80.0f;
    maxs.v.m128_f32[3] = 0.0f;

    collision_context_t context;
    context.pass_entity1.mHandle.mVal = ent->mHandle.mHandle.mVal;
    context.pass_entity2.mHandle.mVal = 0;
    context.pass_owner1.mHandle.mVal = 0;
    context.pass_owner2.mHandle.mVal = 0;
    context.contentmask = ent->clipmask & 0xFF7FFFFF;

    trace_t trace;
    TraceSphereFull(&proximity, &trace, &start, &mins, &maxs, &end,
                    &context);
    if (trace.allsolid != 0 || trace.startsolid != 0)
    {
        mins.v.m128_f32[0] = -20.0f;
        mins.v.m128_f32[1] = -20.0f;
        mins.v.m128_f32[2] = 0.0f;
        mins.v.m128_f32[3] = 0.0f;
        maxs.v.m128_f32[0] = 20.0f;
        maxs.v.m128_f32[1] = 20.0f;
        maxs.v.m128_f32[2] = 40.0f;
        maxs.v.m128_f32[3] = 0.0f;
        TraceSphereFull(&proximity, &trace, &start, &mins, &maxs, &end,
                        &context);
    }

    memcpy(&s_phys, &trace, sizeof(trace_t));
    s_phys.hasGround = 0;
    s_phys.onGround = 0;
    phys->origin.v.m128_f32[0] = trace.endpos.v.m128_f32[0];
    phys->origin.v.m128_f32[1] = trace.endpos.v.m128_f32[1];
    phys->origin.v.m128_f32[2] = trace.endpos.v.m128_f32[2];

    if (trace.fraction != 1.0f
        && (phys->origin.v.m128_f32[2] <= 0.0f
            || (phys->origin.v.m128_f32[0]
                    * trace.normal.v.m128_f32[0]
                + phys->origin.v.m128_f32[1]
                    * trace.normal.v.m128_f32[1]
                + phys->origin.v.m128_f32[2]
                    * trace.normal.v.m128_f32[2]) <= 10.0f))
    {
        s_phys.hasGround = 1;
        if (trace.normal.v.m128_f32[2] >= 0.69999999f)
            s_phys.onGround = 1;
    }
}

// ea: 0x0046A5C0
void VEH_UpdatePO(Entity* ent, char* move, int msec)
{
    scr_vehicle_t* vehicle = ent->scr_vehicle;
    vehicle_info_t* info = s_vehicleInfos[vehicle->infoIdx];
    float ownerViewYaw = 0.0f;
    Entity* owner = HandleDbToEnt(ent->r.mOwner);
    if (owner != nullptr)
        ownerViewYaw = owner->client->ps.viewangles[1];

    const int steerInput = move[1];
    const float steerAbs = fabsf((float)steerInput);
    float steer = 0.0f;
    if (vehicleDeadZone <= steerAbs)
    {
        const int sign = steerInput > 0 ? 1 : (steerInput >= 0 ? 0 : -1);
        steer = ((steerAbs - vehicleDeadZone)
                 / (128.0f - vehicleDeadZone)) * sign;
    }

    const int forwardInput = move[0];
    const float forwardAbs = fabsf((float)forwardInput);
    float forwardInputScaled = 0.0f;
    if (vehicleDeadZone <= forwardAbs)
    {
        const int sign = forwardInput > 0
            ? 1
            : (forwardInput >= 0 ? 0 : -1);
        forwardInputScaled = ((forwardAbs - vehicleDeadZone)
                              / (128.0f - vehicleDeadZone)) * sign;
    }

    const float input = sqrtf(forwardInputScaled * forwardInputScaled
                              + steer * steer);
    float moveDirection[3] = { steer, forwardInputScaled, 0.0f };
    float deltaYaw = vectoyaw(moveDirection) + ownerViewYaw - 90.0f;
    bool reverse = false;
    unsigned char tankStyle = 1;

    if (owner != nullptr && owner->IsLocalPlayer())
    {
        tankStyle = unk_F6A294[3208 * owner->client->mServerClientIndex];
        if (tankStyle == 0)
        {
            deltaYaw = 0.0f;
            if (input != 0.0f)
                deltaYaw = AngleNormalize180Accurate(
                    ownerViewYaw - vehicle->phys.angles.v.m128_f32[1]);
        }
        else
        {
            deltaYaw = -info->rotRate * steer;
        }
    }
    else
    {
        deltaYaw = -info->rotRate * steer;
    }

    if (fabsf(deltaYaw) > forwardInputScaled * 50.0f + 105.0f)
    {
        const int sign = deltaYaw > 0.0f
            ? 1
            : (deltaYaw >= 0.0f ? 0 : -1);
        deltaYaw -= sign * 180.0f;
        reverse = true;
    }

    const float frameSeconds = msec * 0.001f;
    float rotationStep = info->rotAccel * frameSeconds;
    float rotationDelta = deltaYaw - delta_yaw_vel;
    if (rotationDelta < -rotationStep)
        rotationDelta = -rotationStep;
    else if (rotationDelta > rotationStep)
        rotationDelta = rotationStep;
    float rotationRate = rotationDelta + delta_yaw_vel;
    delta_yaw_vel = rotationRate;
    if (rotationRate < -info->rotRate)
        rotationRate = -info->rotRate;
    else if (rotationRate > info->rotRate)
        rotationRate = info->rotRate;
    delta_yaw_vel = rotationRate;

    vehicle->phys.angles.v.m128_f32[1] = AngleNormalize360(
        vehicle->phys.angles.v.m128_f32[1] + frameSeconds * rotationRate);
    vehicle->phys.angles.v.m128_f32[0] = 0.0f;
    vehicle->phys.angles.v.m128_f32[2] = 0.0f;
    vehicle->next.mTurretAngles.v.m128_f32[1] = AngleNormalize360(
        vehicle->next.mTurretAngles.v.m128_f32[1]
        - frameSeconds * rotationRate);

    float forward[3];
    YawVectors(vehicle->phys.angles.v.m128_f32[1], forward, nullptr);
    const float horizontalSpeed = sqrtf(
        vehicle->phys.vel.v.m128_f32[0] * vehicle->phys.vel.v.m128_f32[0]
        + vehicle->phys.vel.v.m128_f32[1] * vehicle->phys.vel.v.m128_f32[1]);
    float signedSpeed = horizontalSpeed;
    if (vehicle->phys.vel.v.m128_f32[0] * forward[0]
            + vehicle->phys.vel.v.m128_f32[1] * forward[1]
            + vehicle->phys.vel.v.m128_f32[2] * forward[2] < 0.0f)
        signedSpeed = -horizontalSpeed;

    float targetSpeed;
    if (tankStyle != 0)
    {
        targetSpeed = info->maxSpeed * forwardInputScaled;
    }
    else
    {
        targetSpeed = (reverse ? -info->maxSpeed : info->maxSpeed) * input;
        if (info->maxSpeed * 0.69999999f < fabsf(signedSpeed))
        {
            const float absDeltaYaw = fabsf(deltaYaw);
            if (absDeltaYaw >= 85.0f)
                targetSpeed *= 0.1f;
            else
            {
                const float scale = (85.0f - absDeltaYaw) * 0.011764706f;
                targetSpeed *= scale * scale * 0.80000001f + 0.1f;
            }
        }
    }

    const float acceleration = frameSeconds * info->accel;
    float speedDelta = targetSpeed - signedSpeed;
    if (speedDelta < -acceleration)
        speedDelta = -acceleration;
    else if (speedDelta > acceleration)
        speedDelta = acceleration;
    const float newSpeed = signedSpeed + speedDelta;
    vehicle->phys.vel.v.m128_f32[0] = forward[0] * newSpeed;
    vehicle->phys.vel.v.m128_f32[1] = forward[1] * newSpeed;
}

// ea: 0x0046C7D0
void VEH_GroundMove(Entity* ent, int msec)
{
    scr_vehicle_t* vehicle = ent->scr_vehicle;
    scr_vehicle_t::vehicle_physic_t* phys = &vehicle->phys;
    const float velocity = sqrtf(
        phys->vel.v.m128_f32[0] * phys->vel.v.m128_f32[0]
        + phys->vel.v.m128_f32[1] * phys->vel.v.m128_f32[1]
        + phys->vel.v.m128_f32[2] * phys->vel.v.m128_f32[2]);
    if (velocity != 0.0f)
    {
        const float oldX = phys->vel.v.m128_f32[0];
        const float oldY = phys->vel.v.m128_f32[1];
        const float oldZ = phys->vel.v.m128_f32[2];
        const float dot = oldX * s_phys.groundTrace.normal.v.m128_f32[0]
                        + oldY * s_phys.groundTrace.normal.v.m128_f32[1]
                        + oldZ * s_phys.groundTrace.normal.v.m128_f32[2];
        const float correction = dot >= 0.0f ? dot * 0.99009901f
                                             : dot * 1.01f;
        phys->vel.v.m128_f32[0]
            = oldX - s_phys.groundTrace.normal.v.m128_f32[0] * correction;
        phys->vel.v.m128_f32[1]
            = oldY - s_phys.groundTrace.normal.v.m128_f32[1] * correction;
        phys->vel.v.m128_f32[2]
            = oldZ - s_phys.groundTrace.normal.v.m128_f32[2] * correction;
        if (phys->vel.v.m128_f32[0] * oldX
                + phys->vel.v.m128_f32[1] * oldY
                + phys->vel.v.m128_f32[2] * oldZ > 0.0f)
        {
            VectorNormalize(phys->vel);
            phys->vel.v.m128_f32[0] *= velocity;
            phys->vel.v.m128_f32[1] *= velocity;
            phys->vel.v.m128_f32[2] *= velocity;
        }
    }

    const int first = VEH_Slide(ent, 0, msec, 0, 0);
    const int second = VEH_Slide(ent, 0, msec, 1, first != 0);
    if (second != 0)
    {
        if (second > 1 && VEH_Slide(ent, 0, msec, 0, 0) > 1)
            vehicle->phys.angles = vehicle->phys.prevAngles;
        vehicle->lastCollision = level.time;
    }
    else
    {
        vehicle->lastNoCollision = level.time;
        vehicle->goodOrigin = phys->origin;
        vehicle->goodAngles = phys->angles;
    }
}

// ea: 0x00490680
void VEH_UpdateGunnerWeapon(Entity* ent, int msec)
{
    scr_vehicle_t* scr_vehicle = ent->scr_vehicle;
    trace_t trace;
    trace.surfaceFlags = 0;
    trace.contents = 0;

    if (scr_vehicle->gunnerWeapon == 0
        || !scr_vehicle->seats[1].gunMounted)
        return;

    const int gunnerBarrel = scr_vehicle->boneIndex.gunner_barrel;
    if (gunnerBarrel < 0)
        return;

    scr_vehicle->gunnerFireTime -= msec;

    DObjSkelMat barrelMtx;
    G_DObjGetWorldBoneIndexMatrix(ent, gunnerBarrel, &barrelMtx);

    DbLinkedHandle<EntityHandleDb, Entity> gunOperator =
        scr_vehicle->seats[1].occupant;
    Entity* gunner = HandleDbToEnt(gunOperator);
    if (gunner == nullptr)
    {
        scr_vehicle->seats[1].firing = false;
        return;
    }

    float start[3];
    float forward[3];
    if (EntityManager::sInst->IsLocalPlayer(gunner))
    {
        Client* client = gunner->client;
        if (client == nullptr)
        {
            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
            AeAssert::gCurrentFile =
                "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
            AeAssert::gCurrentLine = 4849;
            AeAssert::gCurrentExpr = "client";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("old cod assert"))
                __debugbreak();
        }

        if (!IsPlayerFullySeatedInVehicle(gunner))
            return;

        // IDA: the gunner fire input is the unnamed Client byte at +0x784.
        const unsigned char* clientBytes =
            reinterpret_cast<const unsigned char*>(client);
        scr_vehicle->seats[1].firing = (clientBytes[0x784] & 1) != 0;

        weaponFileInfo_t* weaponInfo =
            BG_GetInfoForWeapon(scr_vehicle->gunnerWeapon);
        if (scr_vehicle->seats[1].firing)
        {
            if (ent->isFiring == 0)
            {
                ent->isFiring = 1;
                PostEffectEventWeapon(
                    ent, weaponInfo->szInternalName, (EAction)0x19);
                ent->effectLoopingFire = PostEffectEventWeapon(
                    ent, weaponInfo->szInternalName, (EAction)0x1A);
            }
        }
        else if (ent->isFiring != 0)
        {
            Handle effect = ent->effectLoopingFire;
            ent->isFiring = 0;
            if (effect.mVal != 0)
                EffectEventKill(effect);
            PostEffectEventWeapon(
                ent, weaponInfo->szInternalName, (EAction)0x1B);
        }

        if ((client->ps.pm_flags & 0x4000) != 0)
            scr_vehicle->seats[1].firing = false;
        if (GamePause::IsGamePaused(gunner->GetPlayerIndex()))
            scr_vehicle->seats[1].firing = false;

        const int offset = 1580 * gunner->GetPlayerIndex();
        start[0] = dword_F63C70[offset];
        start[1] = dword_F63C70[offset + 1];
        start[2] = dword_F63C70[offset + 2];
        AnglesToForward(client->ps.viewangles, forward);
    }
    else
    {
        if (gunner->client == nullptr
            || !IsPlayerFullySeatedInVehicle(gunner))
            return;

        start[0] = barrelMtx.origin[0];
        start[1] = barrelMtx.origin[1];
        start[2] = barrelMtx.origin[2];
        forward[0] = barrelMtx.axis[0][0];
        forward[1] = barrelMtx.axis[0][1];
        forward[2] = barrelMtx.axis[0][2];
    }

    float end[3];
    end[0] = start[0] + forward[0] * 10240.0f;
    end[1] = start[1] + forward[1] * 10240.0f;
    end[2] = start[2] + forward[2] * 10240.0f;

    if (ent->has_zone_collision())
    {
        collision_context_t context(gunOperator, 41951377);
        math::Position3 startPos = native_to_cdl_pos3(start);
        math::Position3 endPos = native_to_cdl_pos3(end);
        g_LocationalTrace(&trace, startPos, endPos, context,
                          bulletPriorityMap, 0.0f);
        if (trace.fraction < 1.0f)
        {
            scr_vehicle->targetOrigin[0] = trace.endpos.v.m128_f32[0];
            scr_vehicle->targetOrigin[1] = trace.endpos.v.m128_f32[1];
            scr_vehicle->targetOrigin[2] = trace.endpos.v.m128_f32[2];
        }
    }
    else
    {
        scr_vehicle->barrelBlocked = 1;
    }

    if (scr_vehicle->gunnerFireTime <= 0)
    {
        const bool firing = scr_vehicle->seats[1].firing;
        scr_vehicle->gunnerFireTime = 0;
        if (firing && !scr_vehicle->seats[1].overheating)
            VEH_FireGunnerWeapon(ent, msec);
    }
}

// ea: 0x00463370
void VEH_DebugCapsule(const float* pos, float r, float rad, float height,
                      float g, float b)
{
    float color[4];
    float top[3];
    float dir[3];

    top[0] = pos[0];
    top[1] = pos[1];
    top[2] = pos[2] + height;
    color[0] = r;
    color[1] = g;
    color[2] = b;
    color[3] = 1.0f;
    dir[0] = 0.0f;
    dir[1] = 0.0f;
    dir[2] = 1.0f;
    G_DebugCircleEx(pos, rad, dir, color, 1, 0);
    G_DebugCircleEx(top, rad, dir, color, 1, 0);
}

float hatchLerpDuration = 0.5f; // g.o @ 0xDD7F50
void VEH_UpdateHatch(Entity* ent, int)
{
    scr_vehicle_t* scr_vehicle = ent->scr_vehicle;
    vehicle_info_t* info = s_vehicleInfos[scr_vehicle->infoIdx];
    if (info->type == 2)
    {
        float targetRight = scr_vehicle->mHatchOpen
                                 ? info->hatchOpenAngleRight
                                 : 0.0f;
        float currentRight = scr_vehicle->next.mHatchAngleRight;
        scr_vehicle->current.mHatchAngleRight = currentRight;
        scr_vehicle->next.mHatchAngleRight =
            VEH_LerpAngle(targetRight, currentRight,
                          info->hatchOpenAngleRight / hatchLerpDuration);

        float targetLeft = scr_vehicle->mHatchOpen
                                ? info->hatchOpenAngleLeft
                                : 0.0f;
        float currentLeft = scr_vehicle->next.mHatchAngleLeft;
        scr_vehicle->current.mHatchAngleLeft = currentLeft;
        scr_vehicle->next.mHatchAngleLeft =
            VEH_LerpAngle(targetLeft, currentLeft,
                          info->hatchOpenAngleLeft / hatchLerpDuration);
    }
    else
    {
        scr_vehicle->next.mHatchAngleRight = 0.0f;
        scr_vehicle->next.mHatchAngleLeft = 0.0f;
    }
}
void VEH_UpdateParticlesRBVeh(Entity* ent)
{
    scr_vehicle_t* scr_vehicle = ent->scr_vehicle;
    vehicle_info_t* info = s_vehicleInfos[scr_vehicle->infoIdx];
    if (info->type != 1 && info->type != 2)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile =
            "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 2630;
        AeAssert::gCurrentExpr =
            "( info->type == VEH_WHEELS_4 ) || ( info->type == VEH_TANK )";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    int numWheels = 2 * (info->type != 1) + 4;
    if (scr_vehicle->mRBVeh == nullptr)
        return;
    rb_vehicle* rb_veh = (rb_vehicle*)scr_vehicle->mRBVeh;
    for (int i = 0; i < numWheels; ++i)
    {
        if (rb_veh->m_wheels[i] != nullptr)
            VEH_UpdateWheelParticleEffects(ent, i);
    }
}
// ea: 0x44EA70 (g.o)
void VEH_UpdateShaderTime(Entity* ent, int msec)
{
    scr_vehicle_t* scr_vehicle = ent->scr_vehicle;
    vehicle_info_t* info = s_vehicleInfos[scr_vehicle->infoIdx];
    if (info->texScroll == 0)
        return;

    float frametime = (level.time - scr_vehicle->lastTreadTime) * 0.001f;
    scr_vehicle->lastTreadTime = level.time;
    if (frametime <= 0.0f || frametime > 1000.0f)
        return;

    float delta[3];
    delta[0] = ent->r.currentOrigin.v.m128_f32[0]
               - scr_vehicle->lastTreadPos.v.m128_f32[0];
    delta[1] = ent->r.currentOrigin.v.m128_f32[1]
               - scr_vehicle->lastTreadPos.v.m128_f32[1];
    delta[2] = ent->r.currentOrigin.v.m128_f32[2]
               - scr_vehicle->lastTreadPos.v.m128_f32[2];
    float distanceSquared = delta[0] * delta[0]
                            + delta[1] * delta[1]
                            + delta[2] * delta[2];
    float inverseFrametime = 1.0f / frametime;
    float angle = vectoyaw(delta) - ent->r.currentAngles.v.m128_f32[1];
    angle = AngleNormalize180(angle);

    scr_vehicle->lastTreadPos.v.m128_f32[0] =
        ent->r.currentOrigin.v.m128_f32[0];
    scr_vehicle->lastTreadPos.v.m128_f32[1] =
        ent->r.currentOrigin.v.m128_f32[1];
    scr_vehicle->lastTreadPos.v.m128_f32[2] =
        ent->r.currentOrigin.v.m128_f32[2];
    scr_vehicle->lastTreadPos.v.m128_f32[3] =
        ent->r.currentOrigin.v.m128_f32[3];

    float scroll = (90.0f - fabsf(angle))
                   * (sqrtf(distanceSquared) * inverseFrametime)
                   * 0.011111111f;
    if (fabsf(scroll) > 2.0f)
    {
        float value = g_vehicleTexScrollScale.value;
        float scaledScroll = scroll * 0.0056818184f;
        float contribution;
        if (g_vehicleTexScrollScale.value <= 0.0f)
        {
            contribution = scaledScroll * info->texScrollScale;
            value = (float)msec;
        }
        else
        {
            contribution = scaledScroll * (float)msec;
        }
        float treadDelta = contribution * value;
        scr_vehicle->treadTime += treadDelta;
        scr_vehicle->treadTime2 += treadDelta;
    }

    float angularDelta = AngleNormalize180(
        ent->r.currentAngles.v.m128_f32[1]
        - scr_vehicle->lastTreadAngles.v.m128_f32[1]);
    angularDelta *= inverseFrametime;
    scr_vehicle->lastTreadAngles.v.m128_f32[0] =
        ent->r.currentAngles.v.m128_f32[0];
    scr_vehicle->lastTreadAngles.v.m128_f32[1] =
        ent->r.currentAngles.v.m128_f32[1];
    scr_vehicle->lastTreadAngles.v.m128_f32[2] =
        ent->r.currentAngles.v.m128_f32[2];
    scr_vehicle->lastTreadAngles.v.m128_f32[3] =
        ent->r.currentAngles.v.m128_f32[3];
    scr_vehicle->treadTime += angularDelta;
    scr_vehicle->treadTime2 -= angularDelta;
}
// ea: 0x0046D310
void VEH_UpdateSoundLerps(Entity* ent, int msec)
{
    scr_vehicle_t* vehicle = ent->scr_vehicle;
    float idleTarget = 0.0f;
    float engineTarget = 0.0f;
    float brakeTarget = 0.0f;
    if (HandleDbToEnt(ent->r.mOwner) != nullptr)
    {
        const float speed = ent->speed;
        if (speed <= 1.0f)
        {
            const float rotVelSq =
                vehicle->phys.rotVel.v.m128_f32[0]
                    * vehicle->phys.rotVel.v.m128_f32[0]
                + vehicle->phys.rotVel.v.m128_f32[1]
                    * vehicle->phys.rotVel.v.m128_f32[1]
                + vehicle->phys.rotVel.v.m128_f32[2]
                    * vehicle->phys.rotVel.v.m128_f32[2];
            if (rotVelSq <= 1.0f)
            {
                idleTarget = 1.0f;
                engineTarget = 0.0f;
            }
            else
            {
                idleTarget = 0.0f;
                engineTarget = 1.0f;
            }
        }
        else
        {
            engineTarget = (speed - 1.0f) * 0.011494253f;
            if (engineTarget >= 1.0f)
                engineTarget = 1.0f;
            idleTarget = 1.0f - engineTarget;
        }

        rb_vehicle* rbVeh = (rb_vehicle*)vehicle->mRBVeh;
        if (rbVeh != nullptr)
        {
            if ((rbVeh->m_state_flags & 2) != 0)
            {
                brakeTarget = 1.0f;
                vehicle->brakeSndLerp = 1.0f;
            }
            else
            {
                brakeTarget = 0.0f;
            }

            const float rotEngineDest =
                fabsf(vehicle->phys.rotVel.v.m128_f32[2]) * 1.6666666f;
            if (rotEngineDest > 0.1f && rotEngineDest > engineTarget)
            {
                engineTarget = 1.0f;
                if (rotEngineDest <= 1.0f)
                    engineTarget = rotEngineDest;
                idleTarget = 1.0f - engineTarget;
            }
        }
    }

    const float frameSeconds = msec * 0.001f;
    const float idleCurrent = vehicle->idleSndLerp;
    const float idleDelta = idleTarget - idleCurrent;
    const float idleStep = frameSeconds * idleDelta * 4.0f;
    if (fabsf(idleDelta) <= 0.0049999999f
        || fabsf(idleStep) > fabsf(idleDelta))
        vehicle->idleSndLerp = idleTarget;
    else
        vehicle->idleSndLerp = idleCurrent + idleStep;

    const float engineCurrent = vehicle->engineSndLerp;
    const float engineDelta = engineTarget - engineCurrent;
    const float engineStep = frameSeconds * engineDelta * 4.0f;
    if (fabsf(engineDelta) <= 0.0049999999f
        || fabsf(engineStep) > fabsf(engineDelta))
        vehicle->engineSndLerp = engineTarget;
    else
        vehicle->engineSndLerp = engineCurrent + engineStep;

    const float brakeCurrent = vehicle->brakeSndLerp;
    const float brakeDelta = brakeTarget - brakeCurrent;
    const float brakeStep = frameSeconds * brakeDelta * 4.0f;
    if (fabsf(brakeDelta) <= 0.0049999999f
        || fabsf(brakeStep) > fabsf(brakeDelta))
        vehicle->brakeSndLerp = brakeTarget;
    else
        vehicle->brakeSndLerp = brakeCurrent + brakeStep;
}

// ea: 0x0046D560
void VEH_UpdateSounds(Entity* ent, int msec)
{
    scr_vehicle_t* vehicle = ent->scr_vehicle;
    vehicle_info_t* info = s_vehicleInfos[vehicle->infoIdx];
    ent->s.loopSound = 0;
    VEH_UpdateSoundLerps(ent, msec);

    const EAction idleAction = (EAction)0x23;
    const int engineAction = vehicle->playersAttached != 0 ? 38 : 37;
    const EAction idleStartAction = vehicle->playersAttached != 0
        ? (EAction)0x24
        : idleAction;

    if (vehicle->playEngineSound == 0)
    {
        if (vehicle->mSoundEffectHandle[0].mVal != 0)
            EffectEventStopEmitting(vehicle->mSoundEffectHandle[0]);
        const Handle engineEffect = vehicle->mSoundEffectHandle[1];
        vehicle->mSoundEffectHandle[0].mVal = 0;
        if (engineEffect.mVal != 0)
            EffectEventStopEmitting(engineEffect);
        vehicle->mSoundEffectHandle[1].mVal = 0;
    }
    else
    {
        if (vehicle->idleSndLerp <= 0.0049999999f)
        {
            if (vehicle->mSoundEffectHandle[0].mVal != 0)
                EffectEventStopEmitting(vehicle->mSoundEffectHandle[0]);
            vehicle->mSoundEffectHandle[0].mVal = 0;
        }
        else
        {
            if (vehicle->mSoundEffectHandle[0].mVal == 0)
                vehicle->mSoundEffectHandle[0] =
                    PostEffectEventVehicle(ent, info->name,
                                           idleStartAction);
            EffectEventAdjustEffect_Scale(vehicle->mSoundEffectHandle[0],
                                          "SOUND_VOLUME",
                                          vehicle->idleSndLerp);
        }

        if (vehicle->engineSndLerp <= 0.0049999999f)
        {
            if (vehicle->mSoundEffectHandle[1].mVal != 0)
                EffectEventStopEmitting(vehicle->mSoundEffectHandle[1]);
            vehicle->mSoundEffectHandle[1].mVal = 0;
        }
        else
        {
            if (vehicle->mSoundEffectHandle[1].mVal == 0)
                vehicle->mSoundEffectHandle[1] =
                    PostEffectEventVehicle(ent, info->name,
                                           (EAction)engineAction);
            EffectEventAdjustEffect_Scale(vehicle->mSoundEffectHandle[1],
                                          "SOUND_VOLUME",
                                          vehicle->engineSndLerp);
        }
    }

    if (vehicle->brakeSndLerp <= 0.0049999999f)
    {
        if (vehicle->mSoundEffectHandle[2].mVal != 0)
            EffectEventStopEmitting(vehicle->mSoundEffectHandle[2]);
        vehicle->mSoundEffectHandle[2].mVal = 0;
    }
    else
    {
        if (vehicle->mSoundEffectHandle[2].mVal == 0)
            vehicle->mSoundEffectHandle[2] =
                PostEffectEventVehicle(ent, info->name, (EAction)0x33);
        EffectEventAdjustEffect_Scale(vehicle->mSoundEffectHandle[2],
                                      "SOUND_VOLUME", vehicle->brakeSndLerp);
    }

    if (vehicle->turretState == 2)
    {
        if (vehicle->mSoundEffectHandle[3].mVal == 0)
            vehicle->mSoundEffectHandle[3] =
                PostEffectEventVehicle(ent, info->name, (EAction)0x2D);
    }
    else if (vehicle->turretState == 1)
    {
        if (vehicle->mSoundEffectHandle[3].mVal != 0)
        {
            EffectEventStopEmitting(vehicle->mSoundEffectHandle[3]);
            vehicle->mSoundEffectHandle[3].mVal = 0;
        }
        if (vehicle->mSoundEffectHandle[4].mVal == 0)
            vehicle->mSoundEffectHandle[4] =
                PostEffectEventVehicle(ent, info->name, (EAction)0x2E);
    }
    else
    {
        if (vehicle->mSoundEffectHandle[3].mVal != 0)
        {
            EffectEventStopEmitting(vehicle->mSoundEffectHandle[3]);
            vehicle->mSoundEffectHandle[3].mVal = 0;
        }
        if (vehicle->mSoundEffectHandle[4].mVal != 0)
            vehicle->mSoundEffectHandle[4].mVal = 0;
    }

    if (vehicle->crashSound != 0)
    {
        const Handle crashEffect =
            PostEffectEventVehicle(ent, info->name, (EAction)0x32);
        EffectEventAdjustEffect_Scale(crashEffect, "SOUND_VOLUME",
                                      vehicle->crashVolume);
        vehicle->crashSound = 0;
    }

    if (HandleDbToEnt(vehicle->seats[0].occupant) == nullptr)
        vehicle->hornSndLerp = 0.0f;
    if (vehicle->hornSndLerp <= 0.0049999999f
        || vehicle->seats[0].overheating)
    {
        if (vehicle->mSoundEffectHandle[5].mVal != 0)
        {
            EffectEventStopEmitting(vehicle->mSoundEffectHandle[5]);
            vehicle->mSoundEffectHandle[5].mVal = 0;
        }
    }
    else if (vehicle->mSoundEffectHandle[5].mVal == 0)
    {
        vehicle->mSoundEffectHandle[5] = PostEffectEventVehicle(
            ent, info->name, (EAction)(0x30 | 0x4));
    }
}

// ea: 0x0044E880
void VEH_UpdateSteering(Entity* ent)
{
    scr_vehicle_t* scr_vehicle = ent->scr_vehicle;
    vehicle_info_t* info = s_vehicleInfos[scr_vehicle->infoIdx];
    if (info->steerWheels == 0)
    {
        scr_vehicle->next.mSteeringAngle = 0.0f;
        return;
    }

    float currentAngle = scr_vehicle->next.mSteeringAngle;
    rb_vehicle* rb_veh = (rb_vehicle*)scr_vehicle->mRBVeh;
    float maxAngle = rb_veh->m_parameter->m_steer_angle_max
                     * rb_veh->m_steer_factor * 180.0f
                     * 0.31830987f;
    scr_vehicle->current.mSteeringAngle = currentAngle;

    float steeringAngle = VEH_LerpAngle(maxAngle, currentAngle, 20.0f);
    if (steeringAngle < -60.0f)
        steeringAngle = -60.0f;
    else if (steeringAngle > 60.0f)
        steeringAngle = 60.0f;
    scr_vehicle->next.mSteeringAngle = steeringAngle;

    float delta[3];
    delta[0] = scr_vehicle->phys.origin.v.m128_f32[0]
               - scr_vehicle->lastTreadPos.v.m128_f32[0];
    delta[1] = scr_vehicle->phys.origin.v.m128_f32[1]
               - scr_vehicle->lastTreadPos.v.m128_f32[1];
    delta[2] = scr_vehicle->phys.origin.v.m128_f32[2]
               - scr_vehicle->lastTreadPos.v.m128_f32[2];

    float angle = vectoyaw(delta) - ent->r.currentAngles.v.m128_f32[1];
    angle = AngleNormalize180(angle);
    float distanceSquared = delta[0] * delta[0] + delta[1] * delta[1]
                            + delta[2] * delta[2];

    scr_vehicle->lastTreadPos.v.m128_f32[0] =
        scr_vehicle->phys.origin.v.m128_f32[0];
    scr_vehicle->lastTreadPos.v.m128_f32[1] =
        scr_vehicle->phys.origin.v.m128_f32[1];
    scr_vehicle->lastTreadPos.v.m128_f32[2] =
        scr_vehicle->phys.origin.v.m128_f32[2];
    scr_vehicle->lastTreadPos.v.m128_f32[3] =
        scr_vehicle->phys.origin.v.m128_f32[3];

    scr_vehicle->wheelPitch +=
        (90.0f - fabsf(angle)) * 0.011111111f * sqrtf(distanceSquared)
        / (scr_vehicle->wheelRadius * 6.2831855f) * 360.0f;
}
// ea: 0x0047EF00
void VEH_UpdateWeapon(Entity* ent, int msec)
{
    scr_vehicle_t* scr_vehicle = ent->scr_vehicle;
    vehicle_info_t* info = s_vehicleInfos[scr_vehicle->infoIdx];

    trace_t trace;
    trace.surfaceFlags = 0;
    trace.contents = 0;
    scr_vehicle->hasTarget = 0;
    scr_vehicle->mTargetEnt.mHandle.mVal = 0;
    scr_vehicle->fireTime -= msec;
    scr_vehicle->targetOffset[0] = 0.0f;
    scr_vehicle->targetOffset[1] = 0.0f;
    scr_vehicle->targetOffset[2] = 0.0f;

    Entity* owner = HandleDbToEnt(ent->r.mOwner);
    if (owner == nullptr)
        return;
    Client* client = owner->client;
    if (client == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile =
            "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 4293;
        AeAssert::gCurrentExpr = "client";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
        return;
    }
    if (!IsPlayerFullySeatedInVehicle(owner))
        return;

    if (info->type == 2 && client->ps.vehPos != 0)
        return;

    if (info->type == 1 && client->ps.vehPos == 0)
    {
        if (client->ps.vehPos == 0 && owner != nullptr
            && owner->IsLocalPlayer()
            && !GamePause::IsGamePaused(owner->GetPlayerIndex()))
        {
            const int port = dword_F6A28C[802 * owner->GetPlayerIndex()];
            scr_vehicle->seats[0].firing =
                controller::inst()->button_value(port, controller::L3) != 0;
        }

        if (scr_vehicle->seats[0].firing)
        {
            scr_vehicle->hornSndLerp = 1.0f;
            if (!scr_vehicle->seats[0].overheating)
                scr_vehicle->seats[0].heat += msec * 0.0005f;
        }
        else
        {
            scr_vehicle->hornSndLerp -= (msec * decay) * 0.001f;
            if (scr_vehicle->hornSndLerp < 0.0f)
                scr_vehicle->hornSndLerp = 0.0f;
        }
    }

    if (info->type == 1 && client->ps.vehPos != 2)
        return;

    if (ent->s.weapon == 0 || scr_vehicle->boneIndex.barrel < 0
        || scr_vehicle->boneIndex.flash[0] < 0
        || (client->ps.eFlags & 0x400000) != 0)
        return;

    bool fireEvent = false;
    weaponFileInfo_t* weaponInfo = nullptr;
    if (owner->IsLocalPlayer() && !GamePause::IsGamePaused(owner->GetPlayerIndex()))
    {
        const unsigned char* clientBytes =
            reinterpret_cast<const unsigned char*>(client);
        if ((clientBytes[0x784] & 1) != 0
            && (client->ps.pm_flags & 0x4000) == 0
            && info->type != 1)
        {
            if (scr_vehicle->fireTime <= 0)
            {
                if (scr_vehicle->altWeapon != 0)
                {
                    Scr_Notify(ent, hash_const.turret_fire, 0);
                }
                else
                {
                    weaponFileInfo_t* weaponInfo =
                        BG_GetInfoForWeapon(ent->s.weapon);
                    if (weaponInfo == nullptr)
                    {
                        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)3;
                        AeAssert::gCurrentFile =
                            "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
                        AeAssert::gCurrentLine = 4369;
                        AeAssert::gCurrentExpr = "weapInfo";
                        if (!AeAssert::IsIgnored()
                            && AeAssert::Assert("No weapon info for vehicle"))
                            __debugbreak();
                    }
                    if (!scr_vehicle->seats[0].overheating)
                    {
                        scr_vehicle->seats[0].heat +=
                            (msec * weaponInfo->fFireHeat) * 0.001f;
                        Scr_Notify(ent, hash_const.turret_fire, 0);
                    }
                }
            }
            fireEvent = true;
        }
        weaponInfo = BG_GetInfoForWeapon(ent->s.weapon);
        if (fireEvent)
        {
            if (ent->isFiring == 0)
            {
                ent->isFiring = 1;
                PostEffectEventWeapon(ent, weaponInfo->szInternalName,
                                      (EAction)0x19);
                ent->effectLoopingFire = PostEffectEventWeapon(
                    ent, weaponInfo->szInternalName, (EAction)0x1A);
            }
        }
        else if (ent->isFiring != 0)
        {
            Handle effect = ent->effectLoopingFire;
            ent->isFiring = 0;
            if (effect.mVal != 0)
                EffectEventKill(effect);
            PostEffectEventWeapon(ent, weaponInfo->szInternalName,
                                  (EAction)0x1B);
        }
    }

    scr_vehicle->hasTarget = 1;
    DObjSkelMat flashMtx;
    G_DObjGetWorldBoneIndexMatrix(ent, scr_vehicle->boneIndex.flash[0],
                                  &flashMtx);
    DObjSkelMat barrelMtx;
    G_DObjGetWorldBoneIndexMatrix(ent, scr_vehicle->boneIndex.barrel,
                                  &barrelMtx);

    float forward[3];
    AnglesToForward(client->ps.viewangles, forward);

    float start[3];
    if (owner->IsLocalPlayer())
    {
        const int offset = 1580 * owner->GetPlayerIndex();
        start[0] = dword_F63C70[offset];
        start[1] = dword_F63C70[offset + 1];
        start[2] = dword_F63C70[offset + 2];
    }
    else
    {
        start[0] = barrelMtx.origin[0];
        start[1] = barrelMtx.origin[1];
        start[2] = barrelMtx.origin[2];
    }

    float end[3] = {
        start[0] + forward[0] * 10240.0f,
        start[1] + forward[1] * 10240.0f,
        start[2] + forward[2] * 10240.0f
    };
    scr_vehicle->targetOrigin[0] = end[0];
    scr_vehicle->targetOrigin[1] = end[1];
    scr_vehicle->targetOrigin[2] = end[2];

    if (!ent->has_zone_collision())
    {
        scr_vehicle->barrelBlocked = 1;
        return;
    }

    collision_context_t context(ent->r.mOwner, ent->mHandle, 0x2802091);
    math::Position3 startPos = native_to_cdl_pos3(start);
    math::Position3 endPos = native_to_cdl_pos3(end);
    g_LocationalTrace(&trace, startPos, endPos, context,
                      bulletPriorityMap, 0.0f);
    if (trace.fraction < 1.0f
        || ((trace.mEntity.mHandle.mVal >> 8) & 0xFFu) != 0)
    {
        scr_vehicle->barrelBlocked = 1;
        return;
    }

    math::Position3 barrelStart = native_to_cdl_pos3(barrelMtx.origin);
    math::Position3 flashEnd = native_to_cdl_pos3(flashMtx.origin);
    g_LocationalTrace(&trace, barrelStart, flashEnd, context,
                      bulletPriorityMap, 0.0f);
    if (trace.fraction < 1.0f)
    {
        scr_vehicle->targetOrigin[0] = trace.endpos.v.m128_f32[0];
        scr_vehicle->targetOrigin[1] = trace.endpos.v.m128_f32[1];
        scr_vehicle->targetOrigin[2] = trace.endpos.v.m128_f32[2];
    }
}

// ea: 0x46CB70 (g.o)
void VEH_VerifyPosition(Entity* ent)
{
    if (ent->r.bmodel != nullptr && level.MaxVehicles != 0)
    {
        for (int i = 0; i < level.MaxVehicles; ++i)
        {
            unsigned int mVal = s_vehicles[i].mEntity.mHandle.mVal;
            unsigned int index = mVal & 0xFFF;
            if (index >= 0x540
                || (mVal >> 12)
                       != (unsigned int)EntityHandleDb::sInst.mElements[index].mKey
                || EntityHandleDb::sInst.mElements[index].mObject == nullptr
                || mVal == ent->mHandle.mHandle.mVal)
                continue;

            index = mVal & 0xFFF;
            if (index >= 0x540
                || (mVal >> 12)
                       != (unsigned int)EntityHandleDb::sInst.mElements[index].mKey)
                continue;

            Entity* other = EntityHandleDb::sInst.mElements[index].mObject;
            if (other == nullptr || other->active != 2)
                continue;

            math::Position3 mins(
                other->r.currentOrigin.v.m128_f32[0]
                    + kVehSaftyMins.v.m128_f32[0],
                other->r.currentOrigin.v.m128_f32[1]
                    + kVehSaftyMins.v.m128_f32[1],
                other->r.currentOrigin.v.m128_f32[2]
                    + kVehSaftyMins.v.m128_f32[2]);
            math::Position3 maxs(
                other->r.currentOrigin.v.m128_f32[0]
                    + kVehSaftyMaxs.v.m128_f32[0],
                other->r.currentOrigin.v.m128_f32[1]
                    + kVehSaftyMaxs.v.m128_f32[1],
                other->r.currentOrigin.v.m128_f32[2]
                    + kVehSaftyMaxs.v.m128_f32[2]);
            if (g_EntityContactCapsule(mins, maxs, ent) != 0)
                break;
        }

        if (ent->scr_vehicle->mRBVeh != nullptr)
        {
            ent->scr_vehicle->pathPos.speed = 0.0f;
        }
        else
        {
            memcpy(&ent->scr_vehicle->pathPos, &s_backup.pathPos,
                   sizeof(vehicle_pathpos_t));
            memcpy(&ent->scr_vehicle->phys, &s_backup.phys,
                   sizeof(scr_vehicle_t::vehicle_physic_t));
        }
        ent->speed = 0.0f;
    }
}

void VEH_TryRecordFollowHistory(scr_vehicle_t* veh)
{
    if (veh->follow->numHistoryBufferEntries <= 0)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile =
            "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 3782;
        AeAssert::gCurrentExpr =
            "veh->follow->numHistoryBufferEntries > 0";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }

    vehicle_follow* follow = veh->follow;
    int previousIndex = (follow->historyBufferFront + 39) % 40;
    float deltaX = veh->phys.origin.v.m128_f32[0]
                   - follow->positionHistory[previousIndex][0];
    float deltaY = veh->phys.origin.v.m128_f32[1]
                   - follow->positionHistory[previousIndex][1];
    float deltaZ = veh->phys.origin.v.m128_f32[2]
                   - follow->positionHistory[previousIndex][2];
    if (deltaZ * deltaZ + deltaY * deltaY + deltaX * deltaX >= 324.0f)
    {
        follow->positionHistory[follow->historyBufferFront][0] =
            veh->phys.origin.v.m128_f32[0];
        veh->follow->positionHistory[veh->follow->historyBufferFront][1] =
            veh->phys.origin.v.m128_f32[1];
        veh->follow->positionHistory[veh->follow->historyBufferFront][2] =
            veh->phys.origin.v.m128_f32[2];
        ++veh->follow->numHistoryBufferEntries;
        if (veh->follow->numHistoryBufferEntries > 40)
            veh->follow->numHistoryBufferEntries = 40;
        veh->follow->historyBufferFront =
            (veh->follow->historyBufferFront + 1) % 40;
    }
}

void VEH_UpdateFollow(Entity* ent)
{
    scr_vehicle_t* scr_vehicle = ent->scr_vehicle;
    vehicle_follow* follow = scr_vehicle->follow;
    if (follow == nullptr)
        return;

    unsigned int handleValue =
        follow->claimedSlotEntityHandleList[0].mHandle.mVal;
    unsigned int index = handleValue & 0xFFF;
    int activeCount = 0;
    if (index < 0x540
        && (handleValue >> 12) == EntityHandleDb::sInst.mElements[index].mKey)
    {
        activeCount = EntityHandleDb::sInst.mElements[index].mObject != nullptr;
    }

    handleValue = follow->claimedSlotEntityHandleList[1].mHandle.mVal;
    index = handleValue & 0xFFF;
    if (index < 0x540
        && (handleValue >> 12) == EntityHandleDb::sInst.mElements[index].mKey
        && EntityHandleDb::sInst.mElements[index].mObject != nullptr)
    {
        ++activeCount;
    }

    handleValue = follow->claimedSlotEntityHandleList[2].mHandle.mVal;
    index = handleValue & 0xFFF;
    if (index < 0x540
        && (handleValue >> 12) == EntityHandleDb::sInst.mElements[index].mKey
        && EntityHandleDb::sInst.mElements[index].mObject != nullptr)
    {
        ++activeCount;
    }

    handleValue = follow->claimedSlotEntityHandleList[3].mHandle.mVal;
    index = handleValue & 0xFFF;
    if (index < 0x540
        && (handleValue >> 12) == EntityHandleDb::sInst.mElements[index].mKey
        && EntityHandleDb::sInst.mElements[index].mObject != nullptr)
    {
        ++activeCount;
    }

    handleValue = follow->claimedSlotEntityHandleList[4].mHandle.mVal;
    index = handleValue & 0xFFF;
    if (index < 0x540
        && (handleValue >> 12) == EntityHandleDb::sInst.mElements[index].mKey
        && EntityHandleDb::sInst.mElements[index].mObject != nullptr)
    {
        ++activeCount;
    }

    handleValue = follow->claimedSlotEntityHandleList[5].mHandle.mVal;
    index = handleValue & 0xFFF;
    if (index < 0x540
        && (handleValue >> 12) == EntityHandleDb::sInst.mElements[index].mKey
        && EntityHandleDb::sInst.mElements[index].mObject != nullptr)
    {
        ++activeCount;
    }

    if (activeCount != follow->numFollowingActors)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile =
            "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 3966;
        AeAssert::gCurrentExpr =
            "active_count == veh->follow->numFollowingActors";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }

    VEH_TryRecordFollowHistory(scr_vehicle);
    if (scr_vehicle->follow->numFollowingActors > 0)
        VEH_UpdateFollowFormation(scr_vehicle);
}

// Minimal view of RumbleManager (full class in core/core_systems.h).
class RumbleEffectInstanceHandle {
public:
    int mVal;  // +0x00
};
class RumbleManager {
public:
    static RumbleManager* Inst(int instance);  // ?Inst@RumbleManager@@SAPAV1@H@Z (g.o)
    RumbleEffectInstanceHandle Play(const RumbleEffect& effect,
                                    float intensity);
};


static const __m128 sSignMask = { -0.0f, -0.0f, -0.0f, -0.0f };

extern bool _tlAssert(const char* file, int line, const char* expr,
                      const char* desc);
extern void tlFatal(const char* Format, ...);

// ============================================================================
// phys_static_memory_pool<T,N> - fixed inline-slot pool
// (phys_memory_pool_base.inc). Layout verified for <vehicle_rb_parameter,10>:
// slots inline at +0x00, then m_alloc_list[N], m_index_array[N],
// m_slot_array, m_alloc_count. COMDATs in physics.o: add @0x717420,
// operator[] @0x717480, get_count @0x7174F0, reset_buffer @0x717CF0,
// call_destructors @0x717CE0, ctor @0x71AFD0, dtor @0x71B000.
// ============================================================================
template <typename T, int N>
struct phys_static_memory_pool {
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

    T& operator[](int i)
    {
        if ((i < 0 || i >= m_alloc_count)
            && _tlAssert(
                   "c:\\cod\\code\\tl\\physics\\include\\phys_memory_pool_base.inc",
                   178, "i >= 0 && i < m_alloc_count", ""))
            __debugbreak();
        return *m_alloc_list[i];
    }

    int get_count() const { return m_alloc_count; }
};

// ============================================================================
// vehicle_rb_parameter (physics.o vehicle physics config pool)
// ============================================================================

// ?g_vehicle_rb_parameters@@3V?$phys_static_memory_pool@Vvehicle_rb_parameter@@$09@@A
// (physics.o data)
phys_static_memory_pool<vehicle_rb_parameter, 10> g_vehicle_rb_parameters;

// ea: 0x004AF1C0 (g.o inline COMDAT)
const math::Position3 native_to_cdl_pos3(const float* v)
{
    math::Position3 result;
    result.v = _mm_setr_ps(v[0], v[1], v[2], 0.0f);
    return result;
}

// ea: 0x004A7920 (g.o inline COMDAT)
bool IsPlayerFullySeatedInVehicle(Entity* player)
{
    if (player == nullptr || player->client == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_local.h";
        AeAssert::gCurrentLine = 627;
        AeAssert::gCurrentExpr = "player && player->client";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Invalid player entity"))
            __debugbreak();
    }
    Client* client = player->client;
    return (client->ps.eFlags & 0x100000) == 0
        || (!client->mVehicleAnimMoving && client->ps.vehPos < 8u);
}

// ?s_vehicleInfos@@3PAPAUvehicle_info_t@@A (g.o data @ 0xEA7638, 64 pointers)
vehicle_info_t* s_vehicleInfos[64] = {};

// g.o / physics.o vehicle data
vmCvar_t g_vehicleDebug;         // ?g_vehicleDebug@@3UvmCvar_t@@A (g.o)
vmCvar_t g_vehicleDrawPath;      // ?g_vehicleDrawPath@@3UvmCvar_t@@A (g.o)
vmCvar_t g_vehicleTexScrollScale; // ?g_vehicleTexScrollScale@@3UvmCvar_t@@A (g.o)
math::Position3 kVehSaftyMaxs(105.0f, 105.0f, 175.0f); // g.o data
math::Position3 kVehSaftyMins(-105.0f, -105.0f, -25.0f); // g.o data
VehicleNodeAllocator g_vehicleNodeManager;  // ?g_vehicleNodeManager@@3VVehicleNodeAllocator@@A (g.o)
int g_vehicle_button_threshold = -1;        // ?g_vehicle_button_threshold@@3HA (physics.o)
int rb_vehicle::sRenderAllVehicles = -1;    // ?sRenderAllVehicles@rb_vehicle@@2HA (physics.o)

// ea: 0x6F46B0 (physics.o)
vehicle_rb_parameter::vehicle_rb_parameter()
{
    m_speed_max = 1500.0f;
    m_accel_max = 600.0f;
    m_reverse_scale = 0.80000001f;
    m_susp_hard_limit = 23.0f;
    m_steer_angle_max = 0.60000002f;
    m_tire_fric_fwd = 2.5f;
    m_steer_speed = 5.0f;
    m_susp_damp_k = 1.0f;
    m_tire_fric_side = 2.3f;
    m_tire_fric_brake = 2.3f;
    m_body_mass = 1.0f;
    m_inertia_scale_x = 1.0f;
    m_wheel_radius = 15.0f;
    m_upright_strength = 50.0f;
    m_tire_damp_coast = 10.0f;
    m_susp_spring_k = 20.0f;
    m_roll_stability = 20.0f;
    m_tilt_fakey = 0.25f;
    m_tire_damp_hand = 10000.0f;
    m_susp_adj = 0.0f;
    m_tire_fric_hand_brake = 0.0f;
    m_mass_center_delta_x = 0.0f;
    m_mass_center_delta_y = 0.0f;
    m_mass_center_delta_z = 0.0f;
    m_roll_resistance = 100.0f;
    m_peel_out_max_speed = 150.0f;
    m_tire_damp_brake = 100.0f;
    m_traction_type = TRACTION_TYPE_ALL_WD;
    m_bbox_min.v = _mm_setzero_ps();
    m_bbox_max.v = _mm_setzero_ps();
}

// ea: 0x6FBFB0 (physics.o)
vehicle_rb_parameter* vehicle_rb_parameter::AddRBVehParameter(const char* name)
{
    vehicle_rb_parameter* result = g_vehicle_rb_parameters.add(
        false, "phys memory pool add overflow.");
    strcpy(result->m_name, name);
    return result;
}

// ea: 0x6FBFE0 (physics.o)
vehicle_rb_parameter* vehicle_rb_parameter::GetRBVehParameter(const char* name)
{
    int m_alloc_count = g_vehicle_rb_parameters.m_alloc_count;
    int i = 0;
    if (m_alloc_count <= 0)
        return nullptr;
    while (1)
    {
        if ((i < 0 || i >= m_alloc_count)
            && _tlAssert(
                   "c:\\cod\\code\\tl\\physics\\include\\phys_memory_pool_base.inc",
                   178, "i >= 0 && i < m_alloc_count", ""))
            __debugbreak();
        if (_stricmp(g_vehicle_rb_parameters.m_alloc_list[i]->m_name,
                     name) == 0)
            break;
        m_alloc_count = g_vehicle_rb_parameters.m_alloc_count;
        if (++i >= g_vehicle_rb_parameters.m_alloc_count)
            return nullptr;
    }
    if ((i < 0 || i >= g_vehicle_rb_parameters.m_alloc_count)
        && _tlAssert(
               "c:\\cod\\code\\tl\\physics\\include\\phys_memory_pool_base.inc",
               178, "i >= 0 && i < m_alloc_count", ""))
        __debugbreak();
    return g_vehicle_rb_parameters.m_alloc_list[i];
}

struct clientActive_t {
    uint8_t _pad[0x638];
    bool    stanceHeld;  // +0x638
};
extern clientActive_t cl[2];  // ?cl@@3PAUclientActive_t@@A @ 0xDF01C0

// ea: 0x00452BC0
void VehicleNodeAllocator::Initialize()
{
    m_numNodes = 0;
    m_numBlocks = 0;
    m_currentBlockIndex = 0;
    for (int i = 0; i < 16; ++i)
        m_pNodeBlocks[i] = nullptr;
}

// ea: 0x00452C10
vehicle_node_t* VehicleNodeAllocator::AllocNode()
{
    int16_t m_numBlocks = this->m_numBlocks;
    if (m_numBlocks == 0 || this->m_currentBlockIndex >= 128)
    {
        if (m_numBlocks >= 16)
        {
            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_vehicle_path.cpp";
            AeAssert::gCurrentLine = 1605;
            AeAssert::gCurrentExpr = "0";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("Out of vehicle Nodes - Tell MikeA"))
                __debugbreak();
        }
        this->m_pNodeBlocks[this->m_numBlocks] = mem_heap_malloc(16, 0x2000u);
        vehicle_node_t* v3 = (vehicle_node_t*)this->m_pNodeBlocks[this->m_numBlocks];
        memset(v3, 0, 0x2000);
        for (int i = 128; i != 0; --i)
        {
            if (v3 != nullptr)
            {
                v3->mName = Broc::string();
                v3->mTarget = Broc::string();
                v3->script_noteworthy = Broc::string();
            }
            ++v3;
        }
        ++this->m_numBlocks;
        this->m_currentBlockIndex = 0;
    }
    uint16_t m_currentBlockIndex = this->m_currentBlockIndex;
    vehicle_node_t* result =
        (vehicle_node_t*)this->m_pNodeBlocks[this->m_numBlocks] + m_currentBlockIndex;
    this->m_currentBlockIndex = m_currentBlockIndex + 1;
    s_nodes[this->m_numNodes++] = result;
    return result;
}

// ea: 0x0045F2B0
void VehicleNodeAllocator::FreeAll()
{
    if (this->m_numBlocks > 0)
    {
        for (int i = 0; i < this->m_numBlocks; ++i)
        {
            vehicle_node_t* v3 = (vehicle_node_t*)m_pNodeBlocks[i];
            for (int j = 128; j != 0; --j)
                v3++->~vehicle_node_t();
            mem_heap_free(m_pNodeBlocks[i]);
        }
    }
    m_numNodes = 0;
    m_numBlocks = 0;
    m_currentBlockIndex = 0;
    for (int i = 0; i < 16; ++i)
        m_pNodeBlocks[i] = nullptr;
}

// ea: 0x0044D8C0
void VEH_SetupCollmap(Entity* ent)
{
    ent->s.brushmodel = 0;
    SV_SetBrushModel(ent);
    ent->r.contents = 0xA00000;
}

// ea: 0x0044F3A0
float VEH_GetMaxSpeed(scr_vehicle_t* veh)
{
    rb_vehicle* mRBVeh = (rb_vehicle*)veh->mRBVeh;
    if (mRBVeh != nullptr)
        return *(float*)(*(void**)((char*)mRBVeh + 0x250));  // m_parameter->m_speed_max
    return s_vehicleInfos[veh->infoIdx]->maxSpeed;
}

// ea: 0x0044F520
int scr_vehicle_t::GetStageAnim(Client* client)
{
    return animMap->stages[animMap->routes[client->mVehicleAnimRoute].stages[client->mVehicleAnimStage]].animRow;
}

// ea: 0x00452840
void G_VehFreePathPos(vehicle_pathpos_t* vpp)
{
    vpp->switchNode[0].mName.clear();
    vpp->switchNode[0].mTarget.clear();
    vpp->switchNode[1].mName.clear();
    vpp->switchNode[1].mTarget.clear();
}

// ea: 0x00451950
int16_t VP_GetNodeIndex(const Broc::string& name, float* origin)
{
    if (name.mBlock == nullptr)
        return -1;
    const char* v3 = (const char*)&name.mBlock[1];
    const char* v8 = v3;
    if (name.mBlock == (Broc::string::Block*)-12 || *v3 == 0)
        return -1;
    int16_t i = 0;
    if (s_numNodes <= 0)
        return -1;
    int v5 = 0;
    while (1)
    {
        vehicle_node_t* v6 = s_nodes[v5];
        const char* v7 = v6->mName.mBlock != nullptr
                             ? (const char*)&v6->mName.mBlock[1]
                             : defaultFileName;
        if (strcmp(v7, v3) == 0
            && (origin == nullptr
                || (v6->origin[0] == origin[0] && v6->origin[1] == origin[1]
                    && v6->origin[2] == origin[2])))
            break;
        v5 = ++i;
        if (i >= s_numNodes)
            return -1;
        v3 = v8;
    }
    return i;
}

// ea: 0x00451A50
float VP_CalcNodeSpeed(int16_t nodeIdx)
{
    vehicle_node_t* v2 = s_nodes[nodeIdx];
    if (v2->speed >= 0.0f)
        return v2->speed;
    float v3 = -1.0f;
    int v4 = (16 * v2->nextIdx) >> 18;
    float v5 = 0.0f;
    float v6 = 0.0f;
    float speed = -1.0f;
    if (v4 >= 0)
    {
        vehicle_node_t* v8 = s_nodes[v4];
        int16_t v9 = 0;
        if (s_numNodes > 0)
        {
            while (1)
            {
                v6 = v8->length + v6;
                ++v9;
                if (v8->speed >= 0.0f)
                    break;
                int v10 = (16 * v8->nextIdx) >> 18;
                if (v10 >= 0 && v10 != nodeIdx)
                {
                    v8 = s_nodes[v10];
                    if (v9 < s_numNodes)
                        continue;
                }
                goto label_11;
            }
            speed = v8->speed;
        }
    }
label_11:
    int16_t v11 = 0;
    if (s_numNodes > 0)
    {
        while (1)
        {
            ++v11;
            if (v2->speed >= 0.0f)
                break;
            int v12 = (v2->nextIdx << 18) >> 18;
            if (v12 >= 0 && v12 != nodeIdx)
            {
                float length = v2->length;
                v2 = s_nodes[v12];
                v5 = length + v5;
                if (v11 < s_numNodes)
                    continue;
            }
            goto label_18;
        }
        v3 = v2->speed;
    }
label_18:
    if (speed >= 0.0f)
    {
        if (v3 >= 0.0f)
        {
            float v14 = v5 + v6;
            if (v14 > 0.0f)
                return ((v6 / v14) * (v3 - speed)) + speed;
        }
        else
        {
            return speed;
        }
    }
    else if (v3 >= 0.0f)
    {
        return v3;
    }
    return 0.0f;
}

// ea: 0x00451B60
float VP_CalcNodeLookAhead(int16_t nodeIdx)
{
    vehicle_node_t* v2 = s_nodes[nodeIdx];
    if (v2->lookAhead >= 0.0f)
        return v2->lookAhead;
    float v3 = -1.0f;
    int v4 = (16 * v2->nextIdx) >> 18;
    float v5 = 0.0f;
    float v6 = 0.0f;
    float lookAhead = -1.0f;
    if (v4 >= 0)
    {
        vehicle_node_t* v8 = s_nodes[v4];
        int16_t v9 = 0;
        if (s_numNodes > 0)
        {
            while (1)
            {
                v6 = v8->length + v6;
                ++v9;
                if (v8->lookAhead > 0.0f)
                    break;
                int v10 = (16 * v8->nextIdx) >> 18;
                if (v10 >= 0 && v10 != nodeIdx)
                {
                    v8 = s_nodes[v10];
                    if (v9 < s_numNodes)
                        continue;
                }
                goto label_11;
            }
            lookAhead = v8->lookAhead;
        }
    }
label_11:
    int16_t v11 = 0;
    if (s_numNodes > 0)
    {
        while (1)
        {
            ++v11;
            if (v2->lookAhead > 0.0f)
                break;
            int v12 = (v2->nextIdx << 18) >> 18;
            if (v12 >= 0 && v12 != nodeIdx)
            {
                float length = v2->length;
                v2 = s_nodes[v12];
                v5 = length + v5;
                if (v11 < s_numNodes)
                    continue;
            }
            goto label_18;
        }
        v3 = v2->lookAhead;
    }
label_18:
    if (lookAhead >= 0.0f)
    {
        if (v3 >= 0.0f)
        {
            float v14 = v5 + v6;
            if (v14 > 0.0f)
                return ((v6 / v14) * (v3 - lookAhead)) + lookAhead;
        }
        else
        {
            return lookAhead;
        }
    }
    else if (v3 >= 0.0f)
    {
        return v3;
    }
    return 0.0f;
}

// ea: 0x00451C70
void VP_CalcNodeAngles(int16_t nodeIdx, float* angles)
{
    vehicle_node_t* v2 = s_nodes[nodeIdx];
    if (v2->angles[0] == s_invalidAngles[0]
        && v2->angles[1] == dword_DD7418 && v2->angles[2] == dword_DD741C)
    {
        int v3 = (16 * v2->nextIdx) >> 18;
        float v4 = dword_DD741C;
        float v5 = s_invalidAngles[0];
        float v6 = 0.0f;
        float prevDist = 0.0f;
        float prevAngles = s_invalidAngles[0];
        float v16 = dword_DD7418;
        float v17 = dword_DD741C;
        float nextAngles = s_invalidAngles[0];
        float v19 = dword_DD7418;
        float v20 = dword_DD741C;
        if (v3 >= 0)
        {
            vehicle_node_t* v7 = s_nodes[v3];
            int16_t v8 = 0;
            if (s_numNodes > 0)
            {
                float v9 = 0.0f;
                while (1)
                {
                    ++v8;
                    v9 = v7->length + v9;
                    if (v7->angles[0] != s_invalidAngles[0]
                        || v7->angles[1] != dword_DD7418
                        || v7->angles[2] != dword_DD741C)
                        break;
                    int v10 = (16 * v7->nextIdx) >> 18;
                    if (v10 >= 0 && v10 != nodeIdx)
                    {
                        v7 = s_nodes[v10];
                        if (v8 < s_numNodes)
                            continue;
                    }
                    prevDist = v9;
                    v4 = dword_DD741C;
                    goto label_14;
                }
                prevDist = v9;
                prevAngles = v7->angles[0];
                v16 = v7->angles[1];
                v4 = v7->angles[2];
                v17 = v4;
            }
        }
label_14:
        int16_t v11 = 0;
        if (s_numNodes > 0)
        {
            while (1)
            {
                ++v11;
                if (v2->angles[0] != s_invalidAngles[0]
                    || v2->angles[1] != dword_DD7418
                    || v2->angles[2] != dword_DD741C)
                    break;
                int v12 = (v2->nextIdx << 18) >> 18;
                if (v12 >= 0 && v12 != nodeIdx)
                {
                    float length = v2->length;
                    v2 = s_nodes[v12];
                    v6 = length + v6;
                    if (v11 < s_numNodes)
                        continue;
                }
                goto label_24;
            }
            v5 = v2->angles[0];
            v19 = v2->angles[1];
            nextAngles = v5;
            v20 = v2->angles[2];
        }
label_24:
        if (prevAngles == s_invalidAngles[0] && v16 == dword_DD7418)
        {
            if (v4 == dword_DD741C && v5 == s_invalidAngles[0]
                && v19 == dword_DD7418 && v20 == dword_DD741C)
            {
                angles[0] = 0.0f;
                angles[1] = 0.0f;
                angles[2] = 0.0f;
                return;
            }
            if (v4 == dword_DD741C)
            {
                angles[0] = v5;
                angles[1] = v19;
                angles[2] = v20;
                return;
            }
        }
        if (v5 == s_invalidAngles[0] && v19 == dword_DD7418 && v20 == dword_DD741C)
        {
            angles[0] = prevAngles;
            angles[1] = v16;
            angles[2] = v4;
            return;
        }
        float v14 = v6 + prevDist;
        if (v14 <= 0.0f)
        {
            angles[0] = 0.0f;
            angles[1] = 0.0f;
            angles[2] = 0.0f;
            return;
        }
        float totalDist = prevDist / v14;
        angles[0] = LerpAngle(prevAngles, nextAngles, totalDist);
        angles[1] = LerpAngle(v16, v19, totalDist);
        angles[2] = LerpAngle(v17, v20, totalDist);
    }
    else
    {
        angles[0] = v2->angles[0];
        angles[1] = v2->angles[1];
        angles[2] = v2->angles[2];
    }
}

// ea: 0x00452440
void G_SetupVehiclePaths(void)
{
    for (int16_t i = 0; i < s_numNodes;)
    {
        int v2 = i;
        vehicle_node_t* node = s_nodes[v2];
        Broc::string* p_mName = &node->mName;
        Broc::string::Block* mBlock = p_mName[1].mBlock;
        if (mBlock != nullptr)
        {
            Broc::string::Block* v5 = mBlock + 1;
            if (v5 != nullptr && ((char*)&v5->mBuff)[0] != 0)
                node->nextIdx = (node->nextIdx ^ (node->nextIdx ^ VP_GetNodeIndex(*(p_mName + 1), nullptr)) & 0x3FFF);
        }
        int16_t v6 = 0;
        if (s_numNodes > 0)
        {
            while (i == v6 || !(*p_mName == s_nodes[v6]->mTarget))
            {
                if (++v6 >= s_numNodes)
                    goto label_12;
            }
            node->nextIdx = (node->nextIdx ^ (node->nextIdx ^ (v6 << 14)) & 0xFFFC000);
        }
label_12:
        if ((node->nextIdx << 18) >> 18 == v2)
            node->nextIdx |= 0x3FFF;
        if ((16 * node->nextIdx) >> 18 == v2)
            node->nextIdx |= 0xFFFC000;
        ++i;
    }
    for (int v7 = 0; v7 < s_numNodes; ++v7)
    {
        vehicle_node_t* v9 = s_nodes[v7];
        int v10 = (v9->nextIdx << 18) >> 18;
        if (v10 >= 0)
        {
            v9->dir[0] = s_nodes[v10]->origin[0] - v9->origin[0];
            v9->dir[1] = s_nodes[v10]->origin[1] - v9->origin[1];
            float a1 = s_nodes[v10]->origin[2] - v9->origin[2];
            v9->dir[2] = a1;
            v9->length = VectorNormalize(v9->dir);
            if ((v9->nextIdx & 0x30000000) == 0)
                vectoangles(v9->dir, v9->angles);
        }
    }
    for (int16_t v11 = 0; v11 < s_numNodes; ++v11)
    {
        vehicle_node_t* v13 = s_nodes[v11];
        float a1 = VP_CalcNodeSpeed(v11);
        v13->speed = a1;
        a1 = VP_CalcNodeLookAhead(v11);
        v13->lookAhead = a1;
        if (a1 < 0.0f)
            Com_Error(ERR_DROP, "%s", v13->origin);
        if ((v13->nextIdx & 0x30000000) != 0)
            VP_CalcNodeAngles(v11, v13->angles);
        v13->angles[0] = AngleNormalize180(v13->angles[0]);
        v13->angles[1] = AngleNormalize180(v13->angles[1]);
        v13->angles[2] = AngleNormalize180(v13->angles[2]);
        a1 = 0.0f;
        if (v13->speed <= 0.0f || v13->lookAhead <= 0.0f)
            v13->nextIdx |= 0x3FFF;
        if ((v13->nextIdx & 0x2000) != 0)
        {
            if (v13->speed <= 0.0f)
                v13->speed = 1.0f;
            if (v13->lookAhead <= 0.0f)
                v13->lookAhead = 1.0f;
        }
    }
}

// ea: 0x0044E120
int VEH_GetGenericDistancedFollowHistoryIndex(scr_vehicle_t* veh,
                                              const float* origin,
                                              float requiredDistance,
                                              int startingIndex)
{
    vehicle_follow* follow = veh->follow;
    int i = 0;
    float v6 = requiredDistance * requiredDistance;
    int j;
    for (j = startingIndex + 40;; --j)
    {
        int result = j % 40;
        float v9 = origin[1] - follow->positionHistory[j % 40][1];
        float v10 = origin[0] - follow->positionHistory[j % 40][0];
        float v11 = origin[2] - follow->positionHistory[j % 40][2];
        if ((v11 * v11 + v9 * v9 + v10 * v10) >= v6)
            return result;
        if (++i >= 40)
        {
            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
            AeAssert::gCurrentLine = 3871;
            AeAssert::gCurrentExpr = "0";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
                __debugbreak();
            return veh->follow->historyBufferFront;
        }
    }
}

// ea: 0x0044DFE0
void VEH_GenerateRelativeFormationTable(scr_vehicle_t* veh)
{
    veh->follow->actualColumns = veh->follow->columns;
    vehicle_follow* follow = veh->follow;
    if (follow->numFollowingActors < follow->columns)
        follow->actualColumns = follow->numFollowingActors;
    veh->follow->actualRows =
        (int)ceil((float)veh->follow->numFollowingActors
                  / (float)veh->follow->actualColumns);
    vehicle_follow* v3 = veh->follow;
    int v4 = 0;
    float v5 = 0.0f;
    int r = 0;
    if (v3->actualRows > 0)
    {
        int actualColumns = v3->actualColumns;
        float startingX = (actualColumns - 1) * -0.5f * v3->columnSpacing;
        do
        {
            int v7 = 0;
            float v8 = startingX;
            if (actualColumns > 0)
            {
                vehicle_follow* v9 = veh->follow;
                int v10 = v4;
                do
                {
                    *(float*)((char*)v9->relativeFormation + v10) = v8;
                    *(float*)((char*)&v9->relativeFormation[0][0][1] + v10) = v5;
                    *(float*)((char*)&v9->relativeFormation[0][0][2] + v10) = 0.0f;
                    v9 = veh->follow;
                    actualColumns = v9->actualColumns;
                    ++v7;
                    v10 += 12;
                    v8 = v9->columnSpacing + v8;
                } while (v7 < actualColumns);
            }
            vehicle_follow* v11 = veh->follow;
            float rowSpacing = v11->rowSpacing;
            v5 = v5 - rowSpacing;
            v4 += 72;
            ++r;
        } while (r < veh->follow->actualRows);
    }
}

// ea: 0x0044E260
void VEH_UpdateFollowFormation(scr_vehicle_t* veh)
{
    float up[3] = {0.0f, 0.0f, 1.0f};
    vehicle_follow* follow = veh->follow;
    int v3 = (follow->historyBufferFront + 39) % 40;
    float minFollowDistance = follow->minFollowDistance;
    float requiredTotalRowDistance =
        (follow->actualRows - 1) * follow->rowSpacing;
    int GenericDistancedFollowHistoryIndex =
        VEH_GetGenericDistancedFollowHistoryIndex(
            veh, veh->phys.origin.v.m128_f32, minFollowDistance, v3);
    int v6 = VEH_GetGenericDistancedFollowHistoryIndex(
        veh,
        veh->follow->positionHistory[(GenericDistancedFollowHistoryIndex + 39) % 40],
        requiredTotalRowDistance, (GenericDistancedFollowHistoryIndex + 39) % 40);
    int numFollowingActors = veh->follow->numFollowingActors;
    int v8 = 0;
    int slot = 0;
    if (numFollowingActors > 0)
    {
        int v9 = GenericDistancedFollowHistoryIndex;
        requiredTotalRowDistance = 0.0f;
        do
        {
            vehicle_follow* v10 = veh->follow;
            int actualColumns = v10->actualColumns;
            int v12 = v8 / actualColumns;
            int v13 = v8 % actualColumns;
            int v14 = v12;
            float minMaxDelta[3];
            minMaxDelta[0] = v10->positionHistory[v9][0]
                             - v10->positionHistory[v6][0];
            float v15 = v10->positionHistory[v9][1]
                        - v10->positionHistory[v6][1];
            minMaxDelta[1] = v15;
            minMaxDelta[2] = v10->positionHistory[v9][2]
                             - v10->positionHistory[v6][2];
            float angleDiff[3];
            vectoangles(minMaxDelta, angleDiff);
            float rotatedRelativePosition[3];
            RotatePointAroundVector(rotatedRelativePosition, up,
                                    veh->follow->relativeFormation[v14][v13],
                                    (angleDiff[1] - 90.0f));
            veh->follow->slotGoalPosition[slot][0] =
                veh->follow->positionHistory[v9][0] + rotatedRelativePosition[0];
            veh->follow->slotGoalPosition[slot][1] =
                veh->follow->positionHistory[v9][1] + rotatedRelativePosition[1];
            veh->follow->slotGoalPosition[slot][2] =
                veh->follow->positionHistory[v9][2] + rotatedRelativePosition[2];
            v8 = slot + 1;
            slot = v8;
        } while (v8 < veh->follow->numFollowingActors);
    }
}

// ea: 0x0044E440
bool VEH_AcquirePlayerFollowSlot(Entity* vehicle, Entity* follower)
{
    if (vehicle == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 3985;
        AeAssert::gCurrentExpr = "vehicle != 0";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (follower == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 3986;
        AeAssert::gCurrentExpr = "follower != 0";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (vehicle->scr_vehicle == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 3987;
        AeAssert::gCurrentExpr = "vehicle->scr_vehicle != 0";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (follower->actor == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 3988;
        AeAssert::gCurrentExpr = "follower->actor";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    scr_vehicle_t* scr_vehicle = vehicle->scr_vehicle;
    if (scr_vehicle->follow == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 3993;
        AeAssert::gCurrentExpr = "veh->follow";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (follower->actor->iFollowSlot == -1)
    {
        if (scr_vehicle->follow->numFollowingActors + 1 > 6)
        {
            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
            AeAssert::gCurrentLine = 4004;
            AeAssert::gCurrentExpr = "0";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
                __debugbreak();
            return false;
        }
        scr_vehicle->follow->claimedSlotEntityHandleList[
            scr_vehicle->follow->numFollowingActors].mHandle.mVal =
            follower->mHandle.mHandle.mVal;
        follower->actor->iFollowSlot = scr_vehicle->follow->numFollowingActors++;
        VEH_GenerateRelativeFormationTable(scr_vehicle);
        VEH_UpdateFollowFormation(scr_vehicle);
        return true;
    }
    AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
    AeAssert::gCurrentLine = 3997;
    AeAssert::gCurrentExpr = "0";
    if (!AeAssert::IsIgnored()
        && AeAssert::Assert("This actor is already following a vehicle!!!"))
        __debugbreak();
    return false;
}

// ea: 0x0046CF00
void VEH_ReleasePlayerFollowSlot(Entity* vehicle, Entity* follower)
{
    if (vehicle == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 4025;
        AeAssert::gCurrentExpr = "vehicle != 0";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (follower == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 4026;
        AeAssert::gCurrentExpr = "follower != 0";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (vehicle->scr_vehicle == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 4027;
        AeAssert::gCurrentExpr = "vehicle->scr_vehicle != 0";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (follower->actor == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 4028;
        AeAssert::gCurrentExpr = "follower->actor";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    int iFollowSlot = follower->actor->iFollowSlot;
    scr_vehicle_t* scr_vehicle = vehicle->scr_vehicle;
    if (iFollowSlot == -1)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 4035;
        AeAssert::gCurrentExpr = "0";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("This actor was not following a vehicle!!!"))
            __debugbreak();
        return;
    }
    if (scr_vehicle->follow->numFollowingActors <= 0)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 4039;
        AeAssert::gCurrentExpr = "veh->follow->numFollowingActors > 0";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    Entity* mObject = HandleDbToEnt(
        scr_vehicle->follow->claimedSlotEntityHandleList[iFollowSlot]);
    if (mObject != follower)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 4040;
        AeAssert::gCurrentExpr = "*veh->follow->claimedSlotEntityHandleList[slot] == follower";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    scr_vehicle->follow->claimedSlotEntityHandleList[iFollowSlot].mHandle.mVal = 0;
    --scr_vehicle->follow->numFollowingActors;
    follower->actor->iFollowSlot = -1;
    vehicle_follow* follow = scr_vehicle->follow;
    if (follow->numFollowingActors > 0 && iFollowSlot != follow->numFollowingActors)
    {
        Entity* last = HandleDbToEnt(
            follow->claimedSlotEntityHandleList[follow->numFollowingActors]);
        if (last == nullptr)
        {
            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
            AeAssert::gCurrentLine = 4054;
            AeAssert::gCurrentExpr = "*veh->follow->claimedSlotEntityHandleList[veh->follow->numFollowingActors]";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
                __debugbreak();
        }
        scr_vehicle->follow->claimedSlotEntityHandleList[iFollowSlot].mHandle.mVal =
            scr_vehicle->follow->claimedSlotEntityHandleList[
                scr_vehicle->follow->numFollowingActors].mHandle.mVal;
        scr_vehicle->follow->claimedSlotEntityHandleList[
            scr_vehicle->follow->numFollowingActors].mHandle.mVal = 0;
        Entity* v7 = HandleDbToEnt(
            scr_vehicle->follow->claimedSlotEntityHandleList[iFollowSlot]);
        if (v7 == nullptr)
        {
            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
            AeAssert::gCurrentLine = 4071;
            AeAssert::gCurrentExpr = "0";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("TELL STAVRO! Vehicle's entity follow handle got hosed!!! Was weird zone loading/unloading going on???"))
                __debugbreak();
            return;
        }
        actor_s* actor = v7->actor;
        if (actor == nullptr)
        {
            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
            AeAssert::gCurrentLine = 4064;
            AeAssert::gCurrentExpr = "0";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("Vehicle's actor follow handle got hosed!!! Was weird zone loading/unloading going on???"))
                __debugbreak();
            return;
        }
        actor->iFollowSlot = iFollowSlot;
    }
    VEH_GenerateRelativeFormationTable(scr_vehicle);
    VEH_UpdateFollowFormation(scr_vehicle);
}

// ea: 0x0044E6A0
const float (*VEH_GetPlayerFollowGoalPosition(const Entity* vehicle,
                                              const Entity* follower))[3]
{
    if (vehicle == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 4083;
        AeAssert::gCurrentExpr = "vehicle != 0";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (vehicle->scr_vehicle == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 4084;
        AeAssert::gCurrentExpr = "vehicle->scr_vehicle != 0";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    unsigned int iFollowSlot = follower->actor->iFollowSlot;
    if (iFollowSlot >= 6)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 4087;
        AeAssert::gCurrentExpr = "slot >= 0 && slot < 6";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    return (const float(*)[3])vehicle->scr_vehicle->follow->slotGoalPosition[iFollowSlot];
}

// ea: 0x0046DC50
void G_FreeVehicle(Entity* ent)
{
    if (ent->scr_vehicle->mRBVeh != nullptr)
        rb_vehicle::remove_vehicle((rb_vehicle*)ent->scr_vehicle->mRBVeh);
    if (ent->scr_vehicle->follow != nullptr)
    {
        TPakId mPakId = (TPakId)ent->mPakId;
        if (mPakId == PAK_ID_INVALID)
            mPakId = CurPakId();
        vehicle_follow* follow = ent->scr_vehicle->follow;
        if (follow != nullptr)
            PakManager::sInst->MemFree(mPakId, follow, false);
        ent->scr_vehicle->follow = nullptr;
    }
    if (HandleDbToEnt(ent->scr_vehicle->mEntity) == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 6850;
        AeAssert::gCurrentExpr = "*ent->scr_vehicle->mEntity != 0";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    ent->health = 0;
    VEH_UpdateSounds(ent, 0);
    Entity* idle = HandleDbToEnt(ent->scr_vehicle->mIdleSndEnt);
    if (idle != nullptr)
        G_FreeEntity(idle, 0);
    Entity* engine = HandleDbToEnt(ent->scr_vehicle->mEngineSndEnt);
    if (engine != nullptr)
        G_FreeEntity(engine, 0);
    vehicle_path_node_t* switchNode = ent->scr_vehicle->pathPos.switchNode;
    ent->think = THINK__NULL;
    ent->pain = 0;
    ent->die = 0;
    ent->touch = 0;
    ent->use = 0;
    ent->controller = 0;
    ent->entinfo = 2;
    ent->nextthink = 0;
    ent->takedamage = 0;
    ent->speed = 0.0f;
    ent->active = 0;
    ent->s.eFlags = 0;
    ent->s.pos.trType = TR_STATIONARY;
    ent->s.apos.trType = TR_STATIONARY;
    switchNode->mName.clear();
    ent->scr_vehicle->pathPos.switchNode[1].mName.clear();
    ent->scr_vehicle->pathPos.switchNode[0].mTarget.clear();
    ent->scr_vehicle->pathPos.switchNode[1].mTarget.clear();
    ent->scr_vehicle->mEntity.mHandle.mVal = 0;
    ent->scr_vehicle = nullptr;
}

// ea: 0x0046FF60
void vehicle_InitDynamicBuffers(int vehicles)
{
    scr_vehicle_t* v1 = s_vehicles;
    scr_vehicle_t* old_vehicles = nullptr;
    int old_vehicle_size = 0;
    int old_vehicles_used = 0;
    if (s_vehicles != nullptr)
    {
        if (level.MaxVehicles != 0)
        {
            DbLinkedHandle<EntityHandleDb, Entity>* p_mEntity =
                &s_vehicles->mEntity;
            int MaxVehicles = level.MaxVehicles;
            do
            {
                unsigned int v4 = p_mEntity->mHandle.mVal & 0xFFF;
                if (v4 < 0x540
                    && p_mEntity->mHandle.mVal >> 12
                           == EntityHandleDb::sInst.mElements[v4].mKey
                    && EntityHandleDb::sInst.mElements[v4].mObject != nullptr)
                    ++old_vehicles_used;
                p_mEntity += 468;
                --MaxVehicles;
            } while (MaxVehicles != 0);
            if (old_vehicles_used != 0)
            {
                old_vehicle_size = level.MaxVehicles;
                scr_vehicle_t* v5 = (scr_vehicle_t*)mem_heap_malloc(
                    16, sizeof(scr_vehicle_t) * level.MaxVehicles);
                old_vehicles = v5;
                if (v5 == nullptr)
                {
                    AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
                    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
                    AeAssert::gCurrentLine = 10363;
                    AeAssert::gCurrentExpr = "old_vehicles";
                    if (!AeAssert::IsIgnored() && AeAssert::Assert("Out of memory"))
                        __debugbreak();
                }
                memcpy(v5, s_vehicles, sizeof(scr_vehicle_t) * level.MaxVehicles);
                v1 = s_vehicles;
            }
        }
        if (v1 != nullptr)
        {
            mem_heap_free(v1);
            s_vehicles = nullptr;
        }
    }
    level.MaxVehicles = vehicles;
    if (vehicles != 0)
    {
        s_vehicles = (scr_vehicle_t*)mem_heap_malloc(
            16, sizeof(scr_vehicle_t) * vehicles);
        if (s_vehicles == nullptr)
        {
            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
            AeAssert::gCurrentLine = 10378;
            AeAssert::gCurrentExpr = "s_vehicles";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("Out of memory"))
                __debugbreak();
        }
        memset(s_vehicles, 0, sizeof(scr_vehicle_t) * level.MaxVehicles);
    }
    G_InitScrVehicles();
    if (old_vehicles != nullptr)
    {
        if (old_vehicles_used > level.MaxVehicles)
        {
            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
            AeAssert::gCurrentLine = 10389;
            AeAssert::gCurrentExpr = "old_vehicles_used<=level.MaxVehicles";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("Warning, you're scaling down the size of the vehicle array."))
                __debugbreak();
        }
        scr_vehicle_t* v6 = s_vehicles;
        if (old_vehicle_size > 0)
        {
            DbLinkedHandle<EntityHandleDb, Entity>* v7 =
                &old_vehicles->mEntity;
            for (int vehiclesa = old_vehicle_size; vehiclesa != 0; --vehiclesa)
            {
                unsigned int v8 = v7->mHandle.mVal & 0xFFF;
                if (v8 < 0x540
                    && v7->mHandle.mVal >> 12
                           == EntityHandleDb::sInst.mElements[v8].mKey
                    && EntityHandleDb::sInst.mElements[v8].mObject != nullptr)
                {
                    memcpy(v6, &v7[-92], sizeof(scr_vehicle_t));
                    unsigned int v9 = v7->mHandle.mVal & 0xFFF;
                    Entity* mObject = nullptr;
                    if (v9 < 0x540
                        && v7->mHandle.mVal >> 12
                               == EntityHandleDb::sInst.mElements[v9].mKey)
                        mObject = EntityHandleDb::sInst.mElements[v9].mObject;
                    mObject->scr_vehicle = v6++;
                }
                v7 += 468;
            }
        }
        mem_heap_free(old_vehicles);
    }
}

// ea: 0x0045E900
vehicle_info_t* G_GetVehicleInfo(Entity* ent)
{
    if (ent != nullptr && ent->scr_vehicle != nullptr)
        return s_vehicleInfos[ent->scr_vehicle->infoIdx];
    return nullptr;
}

// ea: 0x00457F90
void G_FreeAnimTreeInstances(void)
{
    for (int i = 0; i < 16; ++i)
        g_scr_data.actorCorpseInfo[i].mEntity.mHandle.mVal = 0;
}

// ea: 0x00490EA0
void VEH_PlayerInteractionEntry(Entity* vehicle)
{
    Entity* Player = EntityManager::sInst->GetPlayer(currCl);
    VEH_LinkPlayer(vehicle, Player, 0, 0, 0);
}

// ea: 0x0044C7E0
void UpdatePaths(Entity* ent)
{
    if ((ent->flags & 0x1000) != 0)
    {
        if (ent->moverState == 7)
        {
            if (ent->key != 0)
            {
                PathNodeMgr::sInst->DisconnectPathsForEntity(ent);
                return;
            }
        }
        else if (ent->moverState == 8)
        {
            PathNodeMgr::sInst->DisconnectPathsForEntity(ent);
            return;
        }
        PathNodeMgr::sInst->ConnectPathsForEntity(ent);
    }
}

// ea: 0x0044D320
bool IsVehicleTank(Entity* ent)
{
    scr_vehicle_t* scr_vehicle = ent->scr_vehicle;
    return scr_vehicle != nullptr && s_vehicleInfos[scr_vehicle->infoIdx]->type == 2;
}

// ea: 0x0044EFD0
void G_FreeScrVehicleInfo(void)
{
    for (int i = 0; i < s_numVehicleInfos; mem_heap_free(s_vehicleInfos[i++]))
        ;
    s_numVehicleInfos = 0;
}

// ea: 0x0045E240
void G_FreeScrVehicles(void)
{
    if (level.MaxVehicles != 0)
    {
        for (int v0 = 0; v0 < level.MaxVehicles; ++v0)
        {
            Broc::string* v2 = (Broc::string*)&s_vehicles[v0];
            v2[14].clear();
            v2[15].clear();
            v2[30].clear();
            v2[31].clear();
        }
    }
}

// ea: 0x004523F0
void G_FreeVehiclePaths(void)
{
    int v1 = 0;
    if (s_numNodes > 0)
    {
        do
        {
            vehicle_node_t* v2 = s_nodes[v1];
            v2->mName.clear();
            v2->mTarget.clear();
            ++v1;
        } while (v1 < s_numNodes);
    }
    s_numNodes = 0;
}

// ea: 0x0046F300
bool scr_vehicle_t::IsPhysicsPaused()
{
    rb_vehicle* mRBVeh = (rb_vehicle*)this->mRBVeh;
    if (mRBVeh != nullptr)
        return (*(unsigned int*)((char*)mRBVeh + 0x280) & 1) != 0;  // m_flags.mMask
    Entity* mObject = HandleDbToEnt(
        *(DbLinkedHandle<EntityHandleDb, Entity>*)((char*)this + 0x1E0));  // seats[0].occupant
    return mObject == nullptr;
}

// ea: 0x0044FB30
vehicle_info_t* VEH_GetVehicleInfo(int iIndex)
{
    if (iIndex > 0x40)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 10667;
        AeAssert::gCurrentExpr = "(iIndex >= 0) && (iIndex <= 64)";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    return s_vehicleInfos[iIndex];
}

// ea: 0x0044F620
int scr_vehicle_t::GetSwitchPosRoute(int seatIdx, int fromPos, bool hasFlag)
{
    vehicleAnimMap_t* animMap = this->animMap;
    if (animMap == nullptr)
        return -1;
    int numRoutes = animMap->numRoutes;
    int result = 0;
    if (numRoutes <= 0)
        return -1;
    for (vehicleAnimRoute_t* i = animMap->routes;
         i->vehPosSrc != fromPos || i->vehPosDest != seatIdx
             || ((i->flags & 4) != 0 && !hasFlag);
         ++i)
    {
        if (++result >= numRoutes)
            return -1;
    }
    return result;
}

// ea: 0x0044D480
__int16 VEH_GetVehicleInfo(const char* name)
{
    if (name == nullptr || *name == 0)
        return -1;
    int v1 = 0;
    if (s_numVehicleInfos <= 0)
        return -1;
    while (_stricmp(name, s_vehicleInfos[v1]->name) != 0)
    {
        if (++v1 >= s_numVehicleInfos)
            return -1;
    }
    return v1;
}

// ea: 0x0044D4E0
int16_t VEH_GetPlayerVehicleInfo(const char* name)
{
    if (name == nullptr || *name == 0)
        return -1;
    int16_t v1 = 0;
    if (s_numVehicleInfos <= 0)
        return -1;
    while (_stricmp(name, s_vehicleInfos[v1]->name) != 0)
    {
        if (++v1 >= s_numVehicleInfos)
            return -1;
    }
    return v1;
}

// ea: 0x0046A370
void VEH_SetPosition(Entity* ent, const math::Position3& origin,
                     const math::Position3& angles,
                     const math::Position3& vel)
{
    scr_vehicle_t* scr_vehicle = ent->scr_vehicle;
    if (ent->takedamage != 0)
    {
        ent->s.pos.trBase[0] = ent->r.currentOrigin.v.m128_f32[0];
        ent->s.pos.trBase[1] = ent->r.currentOrigin.v.m128_f32[1];
        ent->s.pos.trBase[2] = ent->r.currentOrigin.v.m128_f32[2];
        ent->s.pos.trDelta[0] = origin.v.m128_f32[0];
        ent->s.pos.trDelta[1] = origin.v.m128_f32[1];
        ent->s.pos.trDelta[2] = origin.v.m128_f32[2];
        if (IS_NAN(ent->r.currentOrigin.v.m128_f32[0])
            || IS_NAN(ent->r.currentOrigin.v.m128_f32[1])
            || IS_NAN(ent->r.currentOrigin.v.m128_f32[2]))
        {
            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
            AeAssert::gCurrentLine = 1292;
            AeAssert::gCurrentExpr = "!IS_NAN((ent->r.currentOrigin)[0]) && !IS_NAN((ent->r.currentOrigin)[1]) && !IS_NAN((ent->r.currentOrigin)[2])";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid vector"))
                __debugbreak();
        }
        ent->r.currentOrigin.v.m128_f32[0] = origin.v.m128_f32[0];
        ent->r.currentOrigin.v.m128_f32[1] = origin.v.m128_f32[1];
        ent->r.currentOrigin.v.m128_f32[2] = origin.v.m128_f32[2];
        ent->s.apos.trBase[0] = ent->r.currentAngles.v.m128_f32[0];
        ent->s.apos.trBase[1] = ent->r.currentAngles.v.m128_f32[1];
        ent->s.apos.trBase[2] = ent->r.currentAngles.v.m128_f32[2];
        ent->s.apos.trDelta[0] = angles.v.m128_f32[0];
        ent->s.apos.trDelta[1] = angles.v.m128_f32[1];
        ent->s.apos.trDelta[2] = angles.v.m128_f32[2];
        ent->r.currentAngles.v.m128_f32[0] = angles.v.m128_f32[0];
        ent->r.currentAngles.v.m128_f32[1] = angles.v.m128_f32[1];
        ent->r.currentAngles.v.m128_f32[2] = angles.v.m128_f32[2];
        ent->s.pos.trType = TR_INTERPOLATE;
        ent->s.apos.trType = TR_INTERPOLATE;
        if (ent->takedamage != 0)
            g_LinkEntity(ent);
        Entity* v7 = HandleDbToEnt(scr_vehicle->mIdleSndEnt);
        if (v7 != nullptr)
        {
            G_SetOrigin(v7, origin);
            G_SetAngle(v7, angles);
            v7->s.pos.trType = TR_INTERPOLATE;
            v7->s.apos.trType = TR_INTERPOLATE;
            g_LinkEntity(v7);
        }
        Entity* v9 = HandleDbToEnt(scr_vehicle->mEngineSndEnt);
        if (v9 != nullptr)
        {
            G_SetOrigin(v9, origin);
            G_SetAngle(v9, angles);
            v9->s.pos.trType = TR_INTERPOLATE;
            v9->s.apos.trType = TR_INTERPOLATE;
            g_LinkEntity(v9);
        }
        if ((ent->flags & 0x20000000) != 0)
            G_SetEntityOceanHeight(ent);
    }
    else
    {
        SV_UnlinkEntity(ent);
    }
}

// ea: 0x0046DBF0
void G_SetupScrVehicles(void)
{
    int v0 = 0;
    if (level.MaxVehicles != 0)
    {
        int v1 = 0;
        do
        {
            unsigned int mVal = s_vehicles[v1].mEntity.mHandle.mVal;
            if (mVal != 0)
            {
                Entity* Entity = EntityHandleDb::sInst.GetObject(mVal);
                Entity->s.brushmodel = 0;
                SV_SetBrushModel(Entity);
                Entity->r.contents = 0xA00000;
            }
            v1 = ++v0;
        } while (v0 < level.MaxVehicles);
    }
}

// ea: 0x0045F350
void shotgunrandom(float* x, float* y, float randomA, float randomB)
{
    float sinT;
    FastSinCos(((randomA * 360.0f) * 3.1415927f) * 0.0055555557f, &sinT, &randomA);
    *x = randomA * randomB;
    *y = sinT * randomB;
}

// ea: 0x0046F2A0
bool scr_vehicle_t::IsPhysicsStable()
{
    rb_vehicle* mRBVeh = (rb_vehicle*)this->mRBVeh;
    if (mRBVeh != nullptr)
    {
        rb_extra_info* m_chassis_rbinf = *(rb_extra_info**)((char*)mRBVeh + 0x274);  // m_chassis_rbinf
        if (m_chassis_rbinf != nullptr)
            return (*(unsigned int*)((char*)m_chassis_rbinf->m_rb + 0x280) & 4) != 0;
    }
    Entity* mObject = HandleDbToEnt(
        *(DbLinkedHandle<EntityHandleDb, Entity>*)((char*)this + 0x1E0));  // seats[0].occupant
    return mObject == nullptr;
}

// ea: 0x0044F330
bool IsVehFlipped(Entity* ent)
{
    bool result = false;
    if (ent != nullptr)
    {
        scr_vehicle_t* scr_vehicle = ent->scr_vehicle;
        if (scr_vehicle != nullptr
            && scr_vehicle->mRBVeh != nullptr
            && ent->r.currentMat.z.v.m128_f32[2] < 0.2f)
        {
            return true;
        }
    }
    return result;
}

// ea: 0x0044F720
int scr_vehicle_t::GetMantleHintStringIndex()
{
    if (s_vehicleInfos[infoIdx] == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 10113;
        AeAssert::gCurrentExpr = "s_vehicleInfos[ infoIdx ]";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid info pointer in vehicle"))
            __debugbreak();
    }
    return s_vehicleInfos[infoIdx]->mMantleHintStringIndex;
}

// ea: 0x0046F350
bool scr_vehicle_t::IsOppositeTeamInVehicle(int team)
{
    for (int v2 = 0; v2 < 11; ++v2)
    {
        Entity* mObject = HandleDbToEnt(seats[v2].occupant);
        if (mObject != nullptr)
        {
            sentient_s* sentient = mObject->sentient;
            if (sentient != nullptr && sentient->eTeam != team)
                return true;
        }
    }
    return false;
}

// ea: 0x0044F6A0
void scr_vehicle_t::Mantled(Entity* player)
{
    if (mMantleTime != 0)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 10101;
        AeAssert::gCurrentExpr = "mMantleTime == 0";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Vehicle has already been mantled"))
            __debugbreak();
    }
    mMantleTime = level.time + 1800;
    mMantleEntity.mHandle.mVal = player->mHandle.mHandle.mVal;
}

// ea: 0x0045E1D0
void G_InitScrVehicles(void)
{
    if (level.MaxVehicles != 0)
    {
        int v0 = 0;
        int v1 = 0;
        do
        {
            int v2 = v1;
            G_VehInitPathPos(&s_vehicles[v2].pathPos);
            ++v0;
            s_vehicles[v2].mEntity.mHandle.mVal = 0;
            v1 = v0;
        } while (v0 < level.MaxVehicles);
    }
    level.vehicles = s_vehicles;
}

// ea: 0x0044DBD0
void VEH_StopWheelEffects(Entity* ent)
{
    scr_vehicle_t* scr_vehicle = ent->scr_vehicle;
    vehicle_info_t* v2 = s_vehicleInfos[scr_vehicle->infoIdx];
    int count = 2 * (v2->type != 1) + 4;
    if (count != -4)
    {
        for (int i = 0; i < count; ++i)
        {
            if (scr_vehicle->mWheel_ParticleEffectHandle[i].mVal != 0)
            {
                EffectEventStopEmitting(
                    scr_vehicle->mWheel_ParticleEffectHandle[i]);
                scr_vehicle->mWheel_ParticleEffectHandle[i].mVal = 0;
            }
        }
    }
    if (scr_vehicle->mRumbleEffectHandle.mVal != 0)
    {
        EffectEventStopEmitting(scr_vehicle->mRumbleEffectHandle);
        scr_vehicle->mRumbleEffectHandle.mVal = 0;
    }
}

// ea: 0x0046E040
Entity* G_IsVehicleUnusable(Entity* player)
{
    Client* client = player->client;
    if (client == nullptr)
        return nullptr;
    if ((0x100000 & client->ps.eFlags) == 0)
        return nullptr;
    Entity* v4 = HandleDbToEnt(player->r.mOwner);
    if (v4 == nullptr)
        return nullptr;
    return (0x200000 & v4->r.contents) != 0 ? v4 : nullptr;
}

// ea: 0x0046E200
bool G_IsPlayerVehicleGunner(Entity* player)
{
    Client* client = player->client;
    if (client == nullptr)
        return false;
    int eFlags = client->ps.eFlags;
    if ((0x100000 & eFlags) == 0 || (0x400000 & eFlags) != 0)
        return false;
    Entity* mOwner = HandleDbToEnt(player->r.mOwner);
    if (mOwner != nullptr && mOwner->scr_vehicle != nullptr)
        return client->ps.vehPos == 1;
    return false;
}

// ea: 0x0046F9F0
int G_EntryPointSeatAssociation(Entity* vehicle, int entryPosition)
{
    scr_vehicle_t* scr_vehicle = vehicle->scr_vehicle;
    if (s_vehicleInfos[scr_vehicle->infoIdx]->type != 2)
        return sEntryPointSeatAssociation[entryPosition];
    Entity* mObject = HandleDbToEnt(scr_vehicle->seats[0].occupant);
    return mObject != nullptr;
}

// ea: 0x00470490
vehicle_info_t* VEH_GetPlayerVehicleInfo(void)
{
    Entity* Player = EntityManager::sInst->GetPlayer(currCl);
    if (Player == nullptr)
        return nullptr;
    Entity* mObject = HandleDbToEnt(Player->r.mOwner);
    if (mObject == nullptr)
        return nullptr;
    scr_vehicle_t* scr_vehicle = mObject->scr_vehicle;
    if (scr_vehicle == nullptr)
        return nullptr;
    vehicle_info_t* result = s_vehicleInfos[scr_vehicle->infoIdx];
    if (result == nullptr)
        return nullptr;
    return result;
}

// ea: 0x0046F460
bool scr_vehicle_t::LetHatchClose()
{
    if (s_vehicleInfos[infoIdx]->type != 2)
        return true;
    int v2 = 0;
    for (int v4 = 0; v4 < 11; ++v4)
    {
        Entity* mObject = HandleDbToEnt(seats[v4].occupant);
        if (mObject == nullptr)
            continue;
        if (v4 != 0)
        {
            if (v4 == 7 && mObject->client->mVehicleAnimStage < 2)
                continue;
            return false;
        }
        if (!IsPlayerFullySeatedInVehicle(mObject))
            return false;
        v2 = 1;
    }
    return v2 != 0;
}

// ea: 0x0044F2A0
int G_GetVehicleOccupantCount(Entity* ent)
{
    if (ent->scr_vehicle != nullptr)
        return ent->scr_vehicle->playersAttached;
    AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
    AeAssert::gCurrentLine = 7731;
    AeAssert::gCurrentExpr = "ent->scr_vehicle";
    if (AeAssert::IsIgnored() || AeAssert::Assert("G_GetVehicleSeatCount: Entity not a vehicle"))
        __debugbreak();
    return ent->scr_vehicle->playersAttached;
}

// ea: 0x0045E930
int G_GetVehicleSeatCount(Entity* ent)
{
    if (ent->scr_vehicle == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 7725;
        AeAssert::gCurrentExpr = "ent->scr_vehicle";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("G_GetVehicleSeatCount: Entity not a vehicle"))
            __debugbreak();
    }
    if (ent->scr_vehicle != nullptr)
        return s_vehicleInfos[ent->scr_vehicle->infoIdx]->numSeats;
    return 0;
}

// ea: 0x004811F0
void Scr_Vehicle_GetOut(Entity* vehicle, Entity* occupant, int health)
{
    Entity* v3 = occupant;
    if (occupant != nullptr && occupant->IsLocalPlayer())
    {
        int PlayerIndex = occupant->GetPlayerIndex();
        if (InteractionController::Inst(PlayerIndex) != nullptr
            && *(void**)InteractionController::Inst(PlayerIndex) != nullptr)
        {
            InteractionController_EndInteraction(InteractionController::Inst(occupant->GetPlayerIndex()), 1);
        }
    }
    VEH_UnlinkPlayer(v3, true);
    vehicle->health = health;
    if (EntityManager::sInst->IsLocalPlayer(v3))
    {
        unsigned int occupantHandle = v3->mHandle.mHandle.mVal;
        vehicle->Notify(hash_const.deactivate, &occupantHandle);
    }
}

// ea: 0x004807D0
void VEH_UnlinkPlayerDropped(Entity* ent)
{
    if (ent->scr_vehicle == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 6449;
        AeAssert::gCurrentExpr = "ent->scr_vehicle";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    ent->s.eFlags &= 0xFFEFFFFF;
    ent->active = 0;
    ent->r.mOwner.mHandle.mVal = 0;
    Scr_Notify(ent, hash_const.player_off_vehicle, 0);
}

// ea: 0x0046E150
bool G_IsPlayerDrivingVehicle(Entity* player)
{
    Client* client = player->client;
    if (client == nullptr)
        return false;
    int eFlags = client->ps.eFlags;
    if ((0x100000 & eFlags) == 0 || (0x400000 & eFlags) != 0)
        return false;
    Entity* v3 = HandleDbToEnt(player->r.mOwner);
    if (v3 == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 7337;
        AeAssert::gCurrentExpr = "ent";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
        return false;
    }
    if (v3->scr_vehicle == nullptr)
        return false;
    return client->ps.vehPos == 0;
}

// ea: 0x004918E0
void Scr_Vehicle_GetIn(Entity* vehicle, Entity* occupant, int health,
                       int seatIdx, int entryIdx)
{
    unsigned int v5 = seatIdx;
    if (seatIdx == 0)
        vehicle->scr_vehicle->AssignPhysics(occupant);
    VEH_LinkPlayer(vehicle, occupant, (int)v5, entryIdx, 0);
    scr_vehicle_t* scr_vehicle = vehicle->scr_vehicle;
    if (scr_vehicle != nullptr && s_vehicleInfos[scr_vehicle->infoIdx]->type == 2)
        scr_vehicle->noEntryTime = level.time + 4000;
    vehicle->health = health;
    if (EntityManager::sInst->IsLocalPlayer(occupant))
    {
        unsigned int h = occupant->mHandle.mHandle.mVal;
        vehicle->Notify(hash_const.activate, &h);
    }
}

// ea: 0x00480CF0
void VEH_RotateWheels(Entity* self, vehicle_info_t* info)
{
    scr_vehicle_t* scr_vehicle = self->scr_vehicle;
    float trans[3] = { 0.0f, 0.0f, 0.0f };
    float steerAngles[3] = { 0.0f, 0.0f, 0.0f };
    int numWheels = 2 * (info->type != 1) + 4;
    if (2 * (info->type != 1) != -4)
    {
        int v4 = 0;
        int* wheel = scr_vehicle->boneIndex.wheel;
        do
        {
            int v7 = *wheel;
            if (*wheel > 0)
            {
                steerAngles[1] = scr_vehicle->current.mSteeringAngle;
                steerAngles[0] = scr_vehicle->wheelPitch;
                if (v4 != TAG_WHEEL_FRONT_LEFT && v4 != TAG_WHEEL_FRONT_RIGHT)
                    steerAngles[1] = 0.0f;
                G_DObjSetLocalTagInternal_0(trans, steerAngles, v7, self, 0);
            }
            ++v4;
            ++wheel;
        } while (v4 < numWheels);
    }
}

// ea: 0x0044DC50
void VEH_StopAllEffects(Entity* ent)
{
    scr_vehicle_t* scr_vehicle = ent->scr_vehicle;
    vehicle_info_t* v2 = s_vehicleInfos[scr_vehicle->infoIdx];
    int count = 2 * (v2->type != 1) + 4;
    if (2 * (v2->type != 1) != -4)
    {
        for (int i = 0; i < count; ++i)
        {
            if (scr_vehicle->mWheel_ParticleEffectHandle[i].mVal != 0)
            {
                EffectEventStopEmitting(
                    scr_vehicle->mWheel_ParticleEffectHandle[i]);
                scr_vehicle->mWheel_ParticleEffectHandle[i].mVal = 0;
            }
        }
    }
    if (scr_vehicle->mRumbleEffectHandle.mVal != 0)
    {
        EffectEventStopEmitting(scr_vehicle->mRumbleEffectHandle);
        scr_vehicle->mRumbleEffectHandle.mVal = 0;
    }
    for (int i = 6; i != 0; --i)
    {
        if (scr_vehicle->mSoundEffectHandle[6 - i].mVal != 0)
            EffectEventStopEmitting(scr_vehicle->mSoundEffectHandle[6 - i]);
        scr_vehicle->mSoundEffectHandle[6 - i].mVal = 0;
    }
}

// ea: 0x0046DEA0
void G_FreeVehicleRefs(Entity* ent)
{
    int i = 0;
    if (level.MaxVehicles != 0)
    {
        int v1 = 0;
        do
        {
            scr_vehicle_t* v2 = &s_vehicles[v1];
            if (HandleDbToEnt(s_vehicles[v1].mEntity) != nullptr)
            {
                if (v2->mIdleSndEnt.mHandle.mVal == ent->mHandle.mHandle.mVal)
                    v2->mIdleSndEnt.mHandle.mVal = 0;
                if (v2->mEngineSndEnt.mHandle.mVal == ent->mHandle.mHandle.mVal)
                    v2->mEngineSndEnt.mHandle.mVal = 0;
                if (v2->mTargetEnt.mHandle.mVal == ent->mHandle.mHandle.mVal)
                    v2->mTargetEnt.mHandle.mVal = 0;
            }
            ++v1;
            ++i;
        } while (i < level.MaxVehicles);
    }
}

// ea: 0x0044F100
const char* G_GetVehicleInfoName(short index)
{
    if (index < 0)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 7097;
        AeAssert::gCurrentExpr = "index >= 0";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (index < s_numVehicleInfos)
        return s_vehicleInfos[index]->name;
    AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
    AeAssert::gCurrentLine = 7098;
    AeAssert::gCurrentExpr = "index < s_numVehicleInfos";
    if (AeAssert::IsIgnored() || AeAssert::Assert("old cod assert"))
        __debugbreak();
    return s_vehicleInfos[index]->name;
}

// ea: 0x0045E9B0
void Scr_Vehicle_Pain(Entity* pSelf, Entity* pAttacker, int /*damage*/,
                      const float* /*point*/, int mod, const float* dir,
                      hitLocation_t /*hitLoc*/)
{
    scr_vehicle_t* scr_vehicle = pSelf->scr_vehicle;
    if ((scr_vehicle == nullptr || scr_vehicle->mRBVeh == nullptr) && pAttacker != nullptr)
    {
        switch (mod)
        {
        case 3:
        case 4:
        case 5:
        case 6:
        case 9:
        case 10:
        case 17:
        case 18:
        case 27:
        case 28:
        case 32:
        {
            math::Position3 v8;
            v8.v.m128_f32[0] = dir[0];
            v8.v.m128_f32[1] = dir[1];
            v8.v.m128_f32[2] = dir[2];
            v8.v.m128_f32[3] = 0.0f;
            VEH_JoltBody(pSelf, v8, 1.0f, 0.0f, 0.0f);
            break;
        }
        default:
            return;
        }
    }
}

// ea: 0x00480880
int G_IsVehicleUsable(Entity* ent, Entity* player, bool speedCheck)
{
    Client* result = player->client;
    if (result != nullptr)
    {
        if (ent->health < 0
            || (ent->s.eFlags & 0x80000) != 0
            || (result->ps.eFlags & 0x100000) != 0
            || HandleDbToEnt(player->r.mOwner) != nullptr
            || ent->scr_vehicle->noEntryTime + 200 > level.time)
        {
            return 0;
        }
        vehicle_info_t* VehicleInfo = G_GetVehicleInfo(ent);
        scr_vehicle_t* scr_vehicle = ent->scr_vehicle;
        bool canUse = scr_vehicle->CanMantleVehicle(player)
                      || (scr_vehicle->playersAttached < VehicleInfo->numSeats
                          && (VehicleInfo->type != 2
                              || player->sentient == nullptr
                              || !scr_vehicle->IsOppositeTeamInVehicle(player->sentient->eTeam)));
        if (canUse
            && (ent->r.contents & 0x200000) != 0
            && (!speedCheck || (ent->speed <= 100.0f && ent->health > 0)))
        {
            return 1;
        }
        return 0;
    }
    return 0;
}

// ea: 0x0044F010
int16_t G_GetVehicleInfoIndex(const char* name)
{
    if (name == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 7080;
        AeAssert::gCurrentExpr = "name";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (*name == 0)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 7081;
        AeAssert::gCurrentExpr = "name[0]";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    int16_t VehicleInfo = (int16_t)VEH_GetVehicleInfo(name);
    if (VehicleInfo == -1)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 7084;
        AeAssert::gCurrentExpr = "index != -1";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    return VehicleInfo;
}

// ea: 0x0046E540
DbLinkedHandle<EntityHandleDb, Entity> G_GetTankEntNum(int index)
{
    DbLinkedHandle<EntityHandleDb, Entity> result;
    result.mHandle.mVal = 0;
    if (index >= level.MaxVehicles)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 7601;
        AeAssert::gCurrentExpr = "index < level.MaxVehicles";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    scr_vehicle_t* v2 = &level.vehicles[index];
    Entity* v4 = HandleDbToEnt(v2->mEntity);
    if (v4 != nullptr
        && s_vehicleInfos[v2->infoIdx]->type == 2
        && v2->drawOnCompass != 0
        && v4->scr_vehicle != nullptr
        && v4->health > 0
        && HandleDbToEnt(v4->r.mOwner) == nullptr)
    {
        result.mHandle.mVal = v4->mHandle.mHandle.mVal;
    }
    return result;
}

// ea: 0x00488BE0
void Scr_Vehicle_Die(Entity* pSelf, Entity* pInflictor, Entity* pAttacker,
                     int /*damage*/, int mod, int weapon, const float* position,
                     const float* dir, hitLocation_t hitLoc)
{
    scr_vehicle_t* scr_vehicle = pSelf->scr_vehicle;
    scr_vehicle->playEngineSound = 0;
    VEH_StopWheelEffects(pSelf);
    for (int seat = 0; seat < 11; ++seat)
    {
        Entity* mObject = HandleDbToEnt(scr_vehicle->seats[seat].occupant);
        if (mObject != nullptr)
        {
            Scr_Vehicle_GetOut(pSelf, mObject, pSelf->health);
            G_Damage(mObject, pInflictor, pAttacker, dir, position, 9999, 160,
                     mod, hitLoc, weapon);
        }
    }
    Entity* v16 = pAttacker;
    if (pAttacker == nullptr)
    {
        v16 = pInflictor;
        if (pInflictor == nullptr)
            v16 = pSelf;
    }
    Scr_NotifyFromEnt(pSelf, hash_const.death, v16);
}

// ea: 0x00480970
void Scr_Vehicle_Controller(Entity* pSelf, int* const /*unused*/)
{
    if (pSelf == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 7746;
        AeAssert::gCurrentExpr = "pSelf";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (pSelf->scr_vehicle == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 7747;
        AeAssert::gCurrentExpr = "pSelf->scr_vehicle";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    scr_vehicle_t* scr_vehicle = pSelf->scr_vehicle;
    float bodyAngles[3];
    bodyAngles[0] = scr_vehicle->next.mBodyPosition.v.m128_f32[0];
    bodyAngles[1] = 0.0f;
    bodyAngles[2] = scr_vehicle->next.mBodyPosition.v.m128_f32[2];
    int body = scr_vehicle->boneIndex.body;
    if (body >= 0)
        G_DObjSetLocalTagInternal_0(vec3_origin, bodyAngles, body, pSelf, 0);
    float turretAngles[3] = { 0.0f, scr_vehicle->next.mTurretAngles.v.m128_f32[1], 0.0f };
    float barrelAngles[3] = { scr_vehicle->next.mTurretAngles.v.m128_f32[0], 0.0f, 0.0f };
    int turret = scr_vehicle->boneIndex.turret;
    if (turret >= 0)
        G_DObjSetLocalTagInternal_0(vec3_origin, turretAngles, turret, pSelf, 0);
    int barrel = scr_vehicle->boneIndex.barrel;
    if (barrel >= 0)
        G_DObjSetLocalTagInternal_0(vec3_origin, barrelAngles, barrel, pSelf, 0);
}

// ea: 0x0044FB90
bool VEH_VehicleTouchesMine(Entity* vehicle, EntityState* item)
{
    float v12[3];
    v12[0] = item->pos.trBase[0];
    v12[1] = item->pos.trBase[1];
    v12[2] = item->pos.trBase[2];
    weaponFileInfo_t* InfoForWeapon = BG_GetInfoForWeapon(item->weapon);
    if (InfoForWeapon->iTriggerRadius == 0)
        return false;
    rb_vehicle* mRBVeh = (rb_vehicle*)vehicle->scr_vehicle->mRBVeh;
    if (mRBVeh == nullptr)
        return false;
    int v4 = InfoForWeapon->iTriggerRadius * InfoForWeapon->iTriggerRadius;
    float dx = v12[0] - vehicle->r.currentOrigin.v.m128_f32[0];
    float dy = v12[1] - vehicle->r.currentOrigin.v.m128_f32[1];
    float dz = v12[2] - vehicle->r.currentOrigin.v.m128_f32[2];
    if (((dx * dx) + (dy * dy)) + (dz * dz)
        > (vehicle->scr_vehicle->mUseRadius * vehicle->scr_vehicle->mUseRadius) + v4)
    {
        return false;
    }
    for (int v7 = 0; v7 < 6; ++v7)
    {
        rigid_body_constraint_wheel* wheel = mRBVeh->m_wheels[v7];
        if (wheel != nullptr)
        {
            float wx = v12[0] - wheel->m_b2_hitp_loc.v.m128_f32[0];
            float wy = v12[1] - wheel->m_b2_hitp_loc.v.m128_f32[1];
            float wz = v12[2] - wheel->m_b2_hitp_loc.v.m128_f32[2];
            if (v4 > ((wx * wx) + (wy * wy)) + (wz * wz))
                return true;
        }
    }
    return false;
}

// ea: 0x004526D0
void G_VehInitPathPos(vehicle_pathpos_t* vpp)
{
    vpp->nodeIdx = -1;
    vpp->endOfPath = 0;
    vpp->frac = 0.0f;
    vpp->speed = 0.0f;
    vpp->lookAhead = 0.0f;
    vpp->slide = 0.0f;
    vpp->origin[0] = 0.0f;
    vpp->origin[1] = 0.0f;
    vpp->angles[0] = 0.0f;
    vpp->angles[1] = 0.0f;
    vpp->lookPos[0] = 0.0f;
    vpp->lookPos[1] = 0.0f;
    for (int i = 0; i < 2; ++i)
    {
        vehicle_path_node_t* node = &vpp->switchNode[i];
        node->mName.clear();
        node->mTarget.clear();
        node->speed = -1.0f;
        node->lookAhead = -1.0f;
        node->origin[0] = 0.0f;
        node->origin[1] = 0.0f;
        node->dir[0] = 0.0f;
        node->dir[1] = 0.0f;
        node->angles[0] = s_invalidAngles[0];
        node->angles[1] = dword_DD7418;
        node->angles[2] = dword_DD741C;
        node->length = 0.0f;
        node->nextIdx = 0xFFFFFFF;
    }
}

// ea: 0x00452870
void G_VehSetUpPathPos(vehicle_pathpos_t* vpp, int16_t nodeIdx)
{
    vehicle_node_t* v3 = s_nodes[nodeIdx];
    vpp->nodeIdx = nodeIdx;
    vpp->endOfPath = 0;
    vpp->frac = 0.0f;
    vpp->speed = v3->speed;
    vpp->lookAhead = v3->lookAhead;
    vpp->slide = (v3->nextIdx & 0x30000000) != 0 ? 1.0f : 0.0f;
    vpp->origin[0] = v3->origin[0];
    vpp->origin[1] = v3->origin[1];
    vpp->origin[2] = v3->origin[2];
    vpp->angles[0] = v3->angles[0];
    vpp->angles[1] = v3->angles[1];
    vpp->angles[2] = v3->angles[2];
    vpp->lookPos[0] = v3->origin[0];
    vpp->lookPos[1] = v3->origin[1];
    vpp->lookPos[2] = v3->origin[2];
    for (int i = 0; i < 2; ++i)
    {
        vehicle_path_node_t* node = &vpp->switchNode[i];
        node->mName.clear();
        node->mTarget.clear();
        node->speed = -1.0f;
        node->lookAhead = -1.0f;
        node->origin[0] = 0.0f;
        node->origin[1] = 0.0f;
        node->dir[0] = 0.0f;
        node->dir[1] = 0.0f;
        node->angles[0] = s_invalidAngles[0];
        node->angles[1] = dword_DD7418;
        node->angles[2] = dword_DD741C;
        node->length = 0.0f;
        node->nextIdx = 0xFFFFFFF;
    }
}

// ea: 0x00452A10
void G_VehSetSwitchNode(vehicle_pathpos_t* vpp, short srcNodeIdx, short dstNodeIdx)
{
    for (int i = 0; i < 2; ++i)
    {
        vehicle_path_node_t* node = &vpp->switchNode[i];
        node->mName.clear();
        node->mTarget.clear();
        node->speed = -1.0f;
        node->lookAhead = -1.0f;
        node->origin[0] = 0.0f;
        node->origin[1] = 0.0f;
        node->dir[0] = 0.0f;
        node->dir[1] = 0.0f;
        node->angles[0] = s_invalidAngles[0];
        node->angles[1] = dword_DD7418;
        node->angles[2] = dword_DD741C;
        node->length = 0.0f;
        node->nextIdx = 0xFFFFFFF;
    }
    if (srcNodeIdx >= 0 && (dstNodeIdx & 0x8000u) == 0)
    {
        vehicle_node_t* v6 = s_nodes[srcNodeIdx];
        vehicle_node_t* dstNode = s_nodes[dstNodeIdx];
        VP_CopyNode(v6, &vpp->switchNode[0]);
        VP_CopyNode(v6, &vpp->switchNode[1]);
        vpp->switchNode[0].nextIdx ^= (dstNodeIdx ^ vpp->switchNode[0].nextIdx) & 0x3FFF;
        vpp->switchNode[0].dir[0] = dstNode->origin[0] - v6->origin[0];
        vpp->switchNode[0].dir[1] = dstNode->origin[1] - v6->origin[1];
        vpp->switchNode[0].dir[2] = dstNode->origin[2] - v6->origin[2];
        vpp->switchNode[0].length = VectorNormalize(vpp->switchNode[0].dir);
    }
}

// ea: 0x0045F1A0 (VP_CopyNode)
void VP_CopyNode(vehicle_node_t* src, vehicle_path_node_t* dst)
{
    dst->mName = src->mName;
    dst->mTarget = src->mTarget;
    dst->speed = src->speed;
    dst->lookAhead = src->lookAhead;
    dst->script_noteworthy = src->script_noteworthy;
    memcpy(dst->origin, src->origin, sizeof(dst->origin));
    memcpy(dst->dir, src->dir, sizeof(dst->dir));
    memcpy(dst->angles, src->angles, sizeof(dst->angles));
    dst->length = src->length;
    dst->nextIdx = src->nextIdx;
}

void VP_CopyNode(const vehicle_path_node_t* src, vehicle_node_t* dst)
{
    dst->mName = src->mName;
    dst->mTarget = src->mTarget;
    dst->speed = src->speed;
    dst->lookAhead = src->lookAhead;
    dst->script_noteworthy = src->script_noteworthy;
    memcpy(dst->origin, src->origin, sizeof(dst->origin));
    memcpy(dst->dir, src->dir, sizeof(dst->dir));
    memcpy(dst->angles, src->angles, sizeof(dst->angles));
    dst->length = src->length;
    dst->nextIdx = src->nextIdx;
}

// ea: 0x00488280
int G_SpawnVehicle(Entity* ent, const char* typeName, int /*unused*/)
{
    scr_vehicle_t* v3;
    if (ent->scr_vehicle != nullptr)
    {
        v3 = ent->scr_vehicle;
    }
    else
    {
        v3 = nullptr;
        int16_t v4 = 0;
        if (level.MaxVehicles != 0)
        {
            do
            {
                int v5 = v4;
                unsigned int mVal = s_vehicles[v5].mEntity.mHandle.mVal;
                v3 = &s_vehicles[v5];
                if ((mVal & 0xFFF) >= 0x540
                    || mVal >> 12 != EntityHandleDb::sInst.mElements[mVal & 0xFFF].mKey
                    || EntityHandleDb::sInst.mElements[mVal & 0xFFF].mObject == nullptr)
                {
                    break;
                }
                ++v4;
            } while (v4 < level.MaxVehicles);
        }
        if (v4 == level.MaxVehicles)
            Com_Error(ERR_DROP, "Too many vehicles");
    }
    int16_t v8 = v3->infoIdx;
    memset(v3, 0, sizeof(scr_vehicle_t));
    v3->mTargetEnt.mHandle.mVal = 0;
    v3->mIdleSndEnt.mHandle.mVal = 0;
    v3->mEngineSndEnt.mHandle.mVal = 0;
    int16_t infoIdxa = v8;
    v3->playEngineSound = 1;
    if (typeName != nullptr && (infoIdxa = (int16_t)VEH_GetVehicleInfo(typeName)) < 0)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 6803;
        AeAssert::gCurrentExpr = "0";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Can't find info for script vehicle [%s]\n", typeName))
            __debugbreak();
        return 0;
    }
    VEH_InitEntity(ent, v3, infoIdxa);
    VEH_InitVehicle(ent, v3, infoIdxa);
    ent->s.brushmodel = 0;
    SV_SetBrushModel(ent);
    ent->r.contents = 0xA00000;
    if (ent->scr_vehicle != nullptr
        && s_vehicleInfos[ent->scr_vehicle->infoIdx]->type == 3)
    {
        ent->SetAlwaysRender(true);
    }
    return 1;
}

// ea: 0x0045F030
int G_VehUpdatePathPos(Entity* pEnt, vehicle_pathpos_t* vpp, bool overrideSpeed,
                       int msec, int waitNode)
{
    int hitWaitNode = 0;
    if (vpp->endOfPath != 0)
    {
        if (vpp->switchNode[0].mName.mBlock == nullptr
            || vpp->switchNode[0].mName.c_str()[0] == 0)
        {
            return 0;
        }
    }
    vehicle_path_node_t* switchNode = &vpp->switchNode[0];
    if (switchNode->mName.mBlock != nullptr && switchNode->mName.c_str()[0] != 0)
    {
        int NodeIndex = VP_GetNodeIndex(switchNode->mName, nullptr);
        if (NodeIndex >= 0)
        {
            VP_CopyNode(&vpp->switchNode[0], s_nodes[NodeIndex]);
        }
    }
    VP_GetLookAheadXYZ(vpp, vpp->lookPos);
    float lookDir[3];
    lookDir[0] = vpp->lookPos[0] - vpp->origin[0];
    lookDir[1] = vpp->lookPos[1] - vpp->origin[1];
    lookDir[2] = vpp->lookPos[2] - vpp->origin[2];
    float dist = VectorNormalize(lookDir);
    if (dist <= 0.0f)
    {
        vpp->endOfPath = 1;
    }
    else
    {
        vectoangles(lookDir, vpp->angles);
        vpp->angles[0] = AngleNormalize180(vpp->angles[0]);
        vpp->angles[1] = AngleNormalize180(vpp->angles[1]);
        vpp->angles[2] = AngleNormalize180(vpp->angles[2]);
        float v14 = (msec * 0.001f) * vpp->speed;
        if (v14 > dist)
        {
            int next = s_nodes[vpp->nodeIdx]->nextIdx;
            if (next >= 0 && (s_nodes[next]->nextIdx & 0x2000) == 0)
                v14 = dist;
        }
        vpp->origin[0] += v14 * lookDir[0];
        vpp->origin[1] += v14 * lookDir[1];
        vpp->origin[2] += v14 * lookDir[2];
        hitWaitNode = VP_UpdatePathPos(pEnt, vpp, lookDir, overrideSpeed,
                                       waitNode);
        VP_GetAngles(vpp, vpp->angles);
    }
    if (switchNode->mName.mBlock != nullptr
        && switchNode->mName.mBlock != (Broc::string::Block*)-12
        && switchNode->mName.c_str()[0] != 0)
    {
        int v18 = VP_GetNodeIndex(switchNode->mName, nullptr);
        if (v18 >= 0)
            VP_CopyNode(&vpp->switchNode[1], s_nodes[v18]);
    }
    return hitWaitNode;
}

// ea: 0x00491980
int Scr_Vehicle_SeatChange(Entity* occupant, int newSeatIdx)
{
    if (newSeatIdx > 0xA)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 9369;
        AeAssert::gCurrentExpr = "newSeatIdx > VEHPOS_UNKNOWN && newSeatIdx <= VEHPOS_LAST_USABLE";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid seat index"))
            __debugbreak();
    }
    Entity* mObject = HandleDbToEnt(occupant->r.mOwner);
    if (mObject == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 9371;
        AeAssert::gCurrentExpr = "ent";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("G_VehicleOccupantChangeSeat:  specified occupant not attached to vehicle"))
            __debugbreak();
        return 0;
    }
    int vehPos = occupant->client->ps.vehPos;
    int fromPos = vehPos;
    if (occupant->IsLocalPlayer())
    {
        int PlayerIndex = occupant->GetPlayerIndex();
        if (InteractionController::Inst(PlayerIndex) != nullptr
            && *(void**)InteractionController::Inst(PlayerIndex) != nullptr)
        {
            InteractionController_EndInteraction(InteractionController::Inst(PlayerIndex), 1);
        }
    }
    scr_vehicle_t* scr_vehicle = mObject->scr_vehicle;
    if (scr_vehicle != nullptr
        && s_vehicleInfos[scr_vehicle->infoIdx]->type == 1
        && vehPos == 1
        && occupant == EntityManager::sInst->GetPlayer(currCl))
    {
        SetClientViewAngle(occupant, mObject->r.currentAngles.v.m128_f32);
        scr_vehicle->current.mGunnerAngles.v.m128_f32[0] = 0.0f;
        scr_vehicle->current.mGunnerAngles.v.m128_f32[1] = 0.0f;
        scr_vehicle->next.mGunnerAngles.v.m128_f32[0] = 0.0f;
        scr_vehicle->next.mGunnerAngles.v.m128_f32[1] = 0.0f;
    }
    VEH_UnlinkPlayer(occupant, false);
    if (newSeatIdx == 0)
        mObject->scr_vehicle->AssignPhysics(occupant);
    VEH_LinkPlayer(mObject, occupant, (int)newSeatIdx, 0, vehPos);
    return newSeatIdx;
}

// ea: 0x00488ED0
void VEH_RespawnVehicle(Entity* ent)
{
    scr_vehicle_t* scr_vehicle = ent->scr_vehicle;
    VEH_StopAllEffects(ent);
    void* mRBVeh = scr_vehicle->mRBVeh;
    float respawn_origin[3];
    respawn_origin[0] = scr_vehicle->respawn_origin.v.m128_f32[0];
    respawn_origin[1] = scr_vehicle->respawn_origin.v.m128_f32[1];
    respawn_origin[2] = scr_vehicle->respawn_origin.v.m128_f32[2];
    float respawn_angles[3];
    respawn_angles[0] = scr_vehicle->respawn_angles.v.m128_f32[0];
    respawn_angles[1] = scr_vehicle->respawn_angles.v.m128_f32[1];
    respawn_angles[2] = scr_vehicle->respawn_angles.v.m128_f32[2];
    if (mRBVeh != nullptr)
    {
        rb_vehicle::remove_vehicle((rb_vehicle*)mRBVeh);
        scr_vehicle->mRBVeh = nullptr;
    }
    ent->s.eFlags &= ~0x80u;
    G_SpawnVehicle(ent, nullptr, 0);
    scr_vehicle->phys.origin.v.m128_f32[0] = respawn_origin[0];
    scr_vehicle->phys.origin.v.m128_f32[1] = respawn_origin[1];
    scr_vehicle->phys.origin.v.m128_f32[2] = respawn_origin[2];
    scr_vehicle->phys.prevOrigin.v.m128_f32[0] = respawn_origin[0];
    scr_vehicle->phys.prevOrigin.v.m128_f32[1] = respawn_origin[1];
    scr_vehicle->phys.prevOrigin.v.m128_f32[2] = respawn_origin[2];
    ent->r.currentOrigin.v.m128_f32[0] = respawn_origin[0];
    ent->r.currentOrigin.v.m128_f32[1] = respawn_origin[1];
    ent->r.currentOrigin.v.m128_f32[2] = respawn_origin[2];
    scr_vehicle->phys.angles.v.m128_f32[0] = respawn_angles[0];
    scr_vehicle->phys.angles.v.m128_f32[1] = respawn_angles[1];
    scr_vehicle->phys.angles.v.m128_f32[2] = respawn_angles[2];
    scr_vehicle->phys.prevAngles.v.m128_f32[0] = respawn_angles[0];
    scr_vehicle->phys.prevAngles.v.m128_f32[1] = respawn_angles[1];
    scr_vehicle->phys.prevAngles.v.m128_f32[2] = respawn_angles[2];
    ent->r.currentAngles.v.m128_f32[0] = respawn_angles[0];
    ent->r.currentAngles.v.m128_f32[1] = respawn_angles[1];
    ent->r.currentAngles.v.m128_f32[2] = respawn_angles[2];
    VEH_SetPosition(ent, scr_vehicle->phys.origin, scr_vehicle->phys.angles,
                    scr_vehicle->phys.vel);
    scr_vehicle->respawn_origin.v.m128_f32[0] = respawn_origin[0];
    scr_vehicle->respawn_origin.v.m128_f32[1] = respawn_origin[1];
    scr_vehicle->respawn_origin.v.m128_f32[2] = respawn_origin[2];
    ent->rotate.v.m128_f32[0] = respawn_origin[0];
    ent->rotate.v.m128_f32[1] = respawn_origin[1];
    ent->rotate.v.m128_f32[2] = respawn_origin[2];
    scr_vehicle->respawn_angles.v.m128_f32[0] = respawn_angles[0];
    scr_vehicle->respawn_angles.v.m128_f32[1] = respawn_angles[1];
    scr_vehicle->respawn_angles.v.m128_f32[2] = respawn_angles[2];
}

// ea: 0x00488CE0
void SP_script_vehicle(Entity* pSelf)
{
    static unsigned char s_init = 0;
    static unsigned int vehicletype_hash = 0;
    if (!(s_init & 1))
    {
        s_init |= 1;
        vehicletype_hash = HashString::CalcHash("vehicletype");
    }
    const char* typeName = nullptr;
    G_SpawnString(vehicletype_hash, nullptr, &typeName);
    if (s_numVehicleInfos <= 0)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 10231;
        AeAssert::gCurrentExpr = "s_numVehicleInfos > 0";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert(
                   "No vehicle type files are loaded.\nDo all the vehicles in your level\nhave the \"vehicletype\" key?"))
            __debugbreak();
    }
    if (typeName == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 10232;
        AeAssert::gCurrentExpr = "typeName";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert(
                   "Invalid vehicletype found in level.\nDo all the vehicles in your level\nhave the \"vehicletype\" key?"))
            __debugbreak();
    }
    if (G_SpawnVehicle(pSelf, typeName, 1) != 0)
    {
        scr_vehicle_t* scr_vehicle = pSelf->scr_vehicle;
        if (s_vehicleInfos[scr_vehicle->infoIdx]->type == 2)
        {
            VEH_Backup(pSelf);
            if ((pSelf->flags & 0x400000) == 0)
                VEH_SetPosition(pSelf, scr_vehicle->phys.origin,
                                scr_vehicle->phys.angles, scr_vehicle->phys.vel);
        }
        pSelf->scr_vehicle->respawn_origin.v.m128_f32[0] = pSelf->r.currentOrigin.v.m128_f32[0];
        pSelf->scr_vehicle->respawn_origin.v.m128_f32[1] = pSelf->r.currentOrigin.v.m128_f32[1];
        pSelf->scr_vehicle->respawn_origin.v.m128_f32[2] = pSelf->r.currentOrigin.v.m128_f32[2];
        pSelf->rotate.v.m128_f32[0] = pSelf->r.currentOrigin.v.m128_f32[0];
        pSelf->rotate.v.m128_f32[1] = pSelf->r.currentOrigin.v.m128_f32[1];
        pSelf->rotate.v.m128_f32[2] = pSelf->r.currentOrigin.v.m128_f32[2];
        pSelf->scr_vehicle->respawn_angles.v.m128_f32[0] = pSelf->r.currentAngles.v.m128_f32[0];
        pSelf->scr_vehicle->respawn_angles.v.m128_f32[1] = pSelf->r.currentAngles.v.m128_f32[1];
        pSelf->scr_vehicle->respawn_angles.v.m128_f32[2] = pSelf->r.currentAngles.v.m128_f32[2];
    }
}

// ea: 0x0046FD50
int scr_vehicle_t::GetEntryHintStringIndex(Entity* vehicle, int entryPosition)
{
    if (entryPosition >= 6)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 10121;
        AeAssert::gCurrentExpr = "entryPosition >= 0 && entryPosition < NUM_ENTRY_POINTS";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid entry point index"))
            __debugbreak();
    }
    if (s_vehicleInfos[infoIdx] == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 10122;
        AeAssert::gCurrentExpr = "s_vehicleInfos[ infoIdx ]";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid info pointer in vehicle"))
            __debugbreak();
    }
    if (s_vehicleInfos[infoIdx]->type != 2)
    {
        if (sEntryPointHintIndicies[entryPosition] == -1)
        {
            G_GetHintStringIndex(&sEntryPointHintIndicies[entryPosition],
                                 sEntryPointHintText[entryPosition]);
            if (sEntryPointHintIndicies[entryPosition] < 0)
            {
                AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
                AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
                AeAssert::gCurrentLine = 10136;
                AeAssert::gCurrentExpr = "sEntryPointHintIndicies[entryPosition]>= 0";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert("Invalid vehicle entry hint string."))
                    __debugbreak();
            }
        }
        if (sEntryPointSeatAssociation[entryPosition] != 1
            || *BG_GetInfoForWeapon((int)gunnerWeapon)->szUseHintString == 0)
        {
            return sEntryPointHintIndicies[entryPosition];
        }
        return BG_GetInfoForWeapon((int)gunnerWeapon)->iUseHintStringIndex;
    }
    if (HandleDbToEnt(vehicle->scr_vehicle->seats[0].occupant) != nullptr)
        return BG_GetInfoForWeapon((int)gunnerWeapon)->iUseHintStringIndex;
    Entity* mObject = HandleDbToEnt(mEntity);
    return BG_GetInfoForWeapon(mObject->s.weapon)->iUseHintStringIndex;
}

// ea: 0x00480AC0
void Scr_Vehicle_Init(Entity* pSelf, int /*msec*/)
{
    scr_vehicle_t* scr_vehicle = pSelf->scr_vehicle;
    vehicle_info_t* info = s_vehicleInfos[scr_vehicle->infoIdx];
    bool v4 = pSelf->active == 2;
    if (!v4 || s_clientThink != 0)
    {
        float* wheelZPos = scr_vehicle->phys.wheelZPos;
        for (int i = 0; i < 6; ++i)
        {
            int BoneIndex = SV_DObjGetBoneIndex(pSelf, s_wheelTagHashes[i]);
            if (BoneIndex >= 0)
            {
                DObjSkelMat mtx;
                G_DObjGetWorldBoneIndexMatrix(pSelf, BoneIndex, &mtx);
                wheelZPos[i] = mtx.origin[2];
            }
            scr_vehicle->mWheel_ParticleEffectHandle[i].mVal = 0;
        }
        int turretPitch = scr_vehicle->boneIndex.turret;
        if (turretPitch >= 0)
        {
            DObjSkelMat mtx;
            G_DObjGetWorldBoneIndexMatrix(pSelf, turretPitch, &mtx);
            float turretPos[3] = { mtx.origin[0], mtx.origin[1], mtx.origin[2] };
            int turretSpan = scr_vehicle->boneIndex.barrel;
            if (turretSpan >= 0)
            {
                DObjSkelMat mtx2;
                G_DObjGetWorldBoneIndexMatrix(pSelf, turretSpan, &mtx2);
                float spanPos[3] = { mtx2.origin[0], mtx2.origin[1], mtx2.origin[2] };
                scr_vehicle->mUseRadius = VectorDistance(turretPos, spanPos);
            }
        }
        int type = info->type;
        if ((type == 1 || type == 2) && Entity_has_zone_collision(pSelf))
            VEH_GroundPlant(pSelf, 0, 10000);
        float vel[3] = { 0.0f, 0.0f, 0.0f };
        VEH_SetPosition(pSelf, scr_vehicle->phys.origin, scr_vehicle->phys.angles,
                        scr_vehicle->phys.vel);
        scr_vehicle->phys.prevOrigin.v.m128_f32[0] = scr_vehicle->phys.origin.v.m128_f32[0];
        scr_vehicle->phys.prevOrigin.v.m128_f32[1] = scr_vehicle->phys.origin.v.m128_f32[1];
        scr_vehicle->phys.prevOrigin.v.m128_f32[2] = scr_vehicle->phys.origin.v.m128_f32[2];
        scr_vehicle->phys.prevAngles.v.m128_f32[0] = scr_vehicle->phys.angles.v.m128_f32[0];
        scr_vehicle->phys.prevAngles.v.m128_f32[1] = scr_vehicle->phys.angles.v.m128_f32[1];
        scr_vehicle->phys.prevAngles.v.m128_f32[2] = scr_vehicle->phys.angles.v.m128_f32[2];
        math::Position3 angles = scr_vehicle->phys.angles;
        MultiplayerMgr::sInst->ApplyLocalPhysicsToVehicle(
            pSelf, scr_vehicle->phys.origin, angles, *vel);
        collision_context_t context;

        context.pass_entity1.mHandle.mVal = 0;
        context.pass_entity2.mHandle.mVal = 0;
        context.pass_owner1.mHandle.mVal = 0;
        context.pass_owner2.mHandle.mVal = 0;
        context.contentmask = -1;
        G_DoTouchTriggers(pSelf, pSelf->r.currentOrigin, nullptr, context);
        pSelf->think = THINK__Scr_Vehicle_Think;
        pSelf->nextthink = level.time + 1;
    }
    else
    {
        pSelf->nextthink = level.time;
    }
}

// ea: 0x0044D370
int VEH_ParseSpecificField(unsigned char* pStruct, const char* pValue, int fieldType)
{
    if (fieldType == 8)
    {
        int v6 = 0;
        while (_stricmp(pValue, s_vehicleTypeNames[v6]) != 0)
        {
            if (++v6 >= 6)
                break;
        }
        if (v6 == 6)
            Com_Error(ERR_DROP, "unknown vehicle type '%s'", pValue);
        *(int16_t*)(pStruct + 0x20) = (int16_t)v6;
        return 1;
    }
    if (fieldType == 9)
    {
        int v5 = 0;
        while (_stricmp(pValue, s_vehicleSubTypeNames[v5]) != 0)
        {
            if (++v5 >= 9)
                break;
        }
        if (v5 == 9)
        {
            Com_Error(ERR_DROP, "unknown vehicle subtype '%s'", pValue);
            return 1;
        }
        *(int16_t*)(pStruct + 0x22) = (int16_t)v5;
        return 1;
    }
    AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
    AeAssert::gCurrentLine = 1002;
    AeAssert::gCurrentExpr = nullptr;
    if (!AeAssert::IsIgnored())
    {
        if (AeAssert::Warning(va("Bad vehicle field type %i\n", fieldType)))
            __debugbreak();
    }
    Com_Error(ERR_DROP, "Bad vehicle field type %i", fieldType);
    return 0;
}

// ea: 0x0044D350
void VEH_Strcpy(unsigned char* pMember, const char* pKeyValue)
{
    strcpy((char*)pMember, pKeyValue);
}

// ea: 0x0044EC90
void ParseVehicleConfigString(const char* name, const ConfigString* cfgstr)
{
    char buf[256];
    if (s_numVehicleInfos >= 64)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 6552;
        AeAssert::gCurrentExpr = "s_numVehicleInfos < 64";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Too many vehicles"))
            __debugbreak();
    }
    if (strlen(name) > 0x20)
    {
        Com_sprintf(buf, 256, "Vehicle name too long(32max): %s", name);
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 6557;
        AeAssert::gCurrentExpr = "0";
        if (!AeAssert::IsIgnored() && AeAssert::Assert(buf))
            __debugbreak();
    }
    vehicle_info_t* v3 = (vehicle_info_t*)mem_heap_malloc(16, 0x310);
    s_vehicleInfos[s_numVehicleInfos] = v3;
    memset(v3, 0, sizeof(vehicle_info_t));
    strcpy(v3->name, name);
    if (ParseConfigStringToStruct((unsigned char*)v3, s_vehicleFields, 73, cfgstr,
                                  10, VEH_ParseSpecificField,
                                  VEH_Strcpy) != 0)
    {
        int health = v3->health;
        v3->accel = v3->accel * 17.6f;
        v3->collisionSpeed = v3->collisionSpeed * 17.6f;
        v3->maxSpeed = v3->maxSpeed * 17.6f;
        v3->engineSndSpeed = v3->engineSndSpeed * 17.6f;
        if (health == 0)
            v3->health = 1500;
        if (v3->boundsRadius == 0.0f)
            v3->boundsRadius = 75.0f;
        if (v3->boundsLength == 0.0f)
            v3->boundsLength = 300.0f;
        if (v3->boundsRadius > 100.0f)
        {
            Com_sprintf(buf, 256, "Bounds radius too big for %s (MAX = %i)",
                        name, 100);
            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
            AeAssert::gCurrentLine = 6594;
            AeAssert::gCurrentExpr = "0";
            if (!AeAssert::IsIgnored() && AeAssert::Assert(buf))
                __debugbreak();
        }
        if (v3->boundsHeight == 0.0f)
            v3->boundsHeight = 150.0f;
        v3->mins.v.m128_f32[0] = -v3->boundsRadius;
        v3->mins.v.m128_f32[1] = -v3->boundsRadius;
        v3->mins.v.m128_f32[2] = -v3->boundsHeight * 0.5f;
        v3->maxs.v.m128_f32[0] = v3->boundsRadius;
        v3->maxs.v.m128_f32[1] = v3->boundsRadius;
        v3->maxs.v.m128_f32[2] = v3->boundsHeight * 0.5f;
        if (v3->type == 1 && v3->subtype == 0)
            v3->subtype = 1;
        ++s_numVehicleInfos;
    }
    char v9 = v3->mMantleHintString[0];
    v3->mMantleHintStringIndex = -1;
    if (v9 != 0)
        G_GetHintStringIndex(&v3->mMantleHintStringIndex,
                             v3->mMantleHintString);
}

// ea: 0x00463730
void ParseVehiclePhysicsConfigString(const char* name, const ConfigString* cfgstr)
{
    if (vehicle_rb_parameter::GetRBVehParameter(name) != nullptr)
        return;
    vehicle_rb_parameter* param = vehicle_rb_parameter::AddRBVehParameter(name);
    if (param == nullptr)
        return;
    const InplaceTree<InplaceString, InplaceString>& stringMap =
        cfgstr->mStringMap;
    const char* str = nullptr;
    InplaceString* entry = stringMap.Find<const char*>("tractiontype");
    if (entry != nullptr)
        str = entry->mStr;
    if (str != nullptr)
    {
        if (_stricmp(str, "all_wd") == 0)
            param->m_traction_type = TRACTION_TYPE_ALL_WD;
        else if (_stricmp(str, "front") == 0)
            param->m_traction_type = TRACTION_TYPE_FRONT;
        else if (_stricmp(str, "back") == 0)
            param->m_traction_type = TRACTION_TYPE_BACK;
    }
    for (int i = 0; i < 27; ++i)
    {
        InplaceString* item =
            stringMap.Find<const char*>(sVehicleVarConfig[i].name);
        if (item != nullptr && item->mStr != nullptr)
            *(float*)((char*)param + sVehicleVarConfig[i].offset) =
                atof(item->mStr);
    }
    entry = stringMap.Find<const char*>("bbox_min_x");
    if (entry != nullptr && entry->mStr != nullptr)
        param->m_bbox_min.v.m128_f32[0] = atof(entry->mStr);
    entry = stringMap.Find<const char*>("bbox_min_y");
    if (entry != nullptr && entry->mStr != nullptr)
        param->m_bbox_min.v.m128_f32[1] = atof(entry->mStr);
    entry = stringMap.Find<const char*>("bbox_min_z");
    if (entry != nullptr && entry->mStr != nullptr)
        param->m_bbox_min.v.m128_f32[2] = atof(entry->mStr);
    entry = stringMap.Find<const char*>("bbox_max_x");
    if (entry != nullptr && entry->mStr != nullptr)
        param->m_bbox_max.v.m128_f32[0] = atof(entry->mStr);
    entry = stringMap.Find<const char*>("bbox_max_y");
    if (entry != nullptr && entry->mStr != nullptr)
        param->m_bbox_max.v.m128_f32[1] = atof(entry->mStr);
    entry = stringMap.Find<const char*>("bbox_max_z");
    if (entry != nullptr && entry->mStr != nullptr)
        param->m_bbox_max.v.m128_f32[2] = atof(entry->mStr);
}

static const char* s_seatTags[11] = {
    "tag_driver", "tag_gunner", "tag_passenger1", "tag_passenger2",
    "tag_passenger3", "tag_passenger4", "tag_gunner", "tag_passenger1",
    "tag_driver", "tag_gunner", "tag_passenger1",
};

// ea: 0x00490A60
void VEH_LinkPlayer(Entity* ent, Entity* player, int seatIdx, int entryIdx,
                    int fromPos)
{
    scr_vehicle_t* scr_vehicle = ent->scr_vehicle;
    Client* client = player->client;
    vehicle_info_t* info = s_vehicleInfos[scr_vehicle->infoIdx];
    player->invulnerability_timeout = 0;
    if (client == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 5664;
        AeAssert::gCurrentExpr = "client";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (HandleDbToEnt(player->r.mOwner) != nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 5668;
        AeAssert::gCurrentExpr = "*player->r.mOwner == 0";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("VEH_LinkPlayer: Player already has an owner\n"))
            __debugbreak();
    }
    vehicleSeat_t& seat = scr_vehicle->seats[seatIdx];
    if (HandleDbToEnt(seat.occupant) != nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 5671;
        AeAssert::gCurrentExpr = "0";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("VEH_LinkPlayer: Vehicle seat already occupied\n"))
            __debugbreak();
    }
    else
    {
        if (seat.boneIndex < 0)
        {
            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
            AeAssert::gCurrentLine = 5682;
            AeAssert::gCurrentExpr = "veh->seats[seatIdx].boneIndex >= 0";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("VEH_LinkPlayer: Trying to use vehicle without a bone\n"))
                __debugbreak();
        }
        DObjSkelMat playerMtx;
        G_DObjGetWorldBoneIndexMatrix(ent, seat.boneIndex, &playerMtx);
        ent->active = 2;
        if (entryIdx != 0
            && scr_vehicle->animMap != nullptr
            && (scr_vehicle->GetEntryRoute(seatIdx, entryIdx - 1,
                                           player->client->ps.ctf_has_flag != 0))
                   >= 0
            && (player->flags |= 0x1000000,
                player->client->mVehicleAnimRoute =
                    scr_vehicle->GetEntryRoute(seatIdx, entryIdx - 1,
                                               player->client->ps.ctf_has_flag != 0),
                scr_vehicle->SetAnimRouteStage(player, ent,
                                               player->client->mVehicleAnimRoute,
                                               0)))
        {
            client->mVehicleAnimStageAnimPlayed = -1;
        }
        else
        {
            if (fromPos != -1
                && scr_vehicle->animMap != nullptr
                && (scr_vehicle->GetSwitchPosRoute(
                        seatIdx, fromPos,
                        player->client->ps.ctf_has_flag != 0))
                       >= 0
                && (player->flags |= 0x1000000,
                    player->client->mVehicleAnimRoute =
                        scr_vehicle->GetSwitchPosRoute(
                            seatIdx, fromPos,
                            player->client->ps.ctf_has_flag != 0),
                    scr_vehicle->SetAnimRouteStage(player, ent,
                                                   player->client->mVehicleAnimRoute,
                                                   0)))
            {
                client->mVehicleAnimStageAnimPlayed = -1;
            }
            else if (G_EntLinkToWithOffset(player, ent, s_seatTags[seatIdx],
                                          vec3_origin, vec3_origin, false) == 0)
            {
                AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
                AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
                AeAssert::gCurrentLine = 5752;
                AeAssert::gCurrentExpr = "0";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert("Missing  vehicle attach tag"))
                    __debugbreak();
            }
        }
        if (scr_vehicle->mTargetEnt.mHandle.mVal == player->mHandle.mHandle.mVal)
        {
            scr_vehicle->hasTarget = 0;
            scr_vehicle->mTargetEnt.mHandle.mVal = 0;
        }
        scr_vehicle->seats[seatIdx].occupant.mHandle.mVal =
            player->mHandle.mHandle.mVal;
        player->r.mOwner.mHandle.mVal = ent->mHandle.mHandle.mVal;
        if (seatIdx == 0 || HandleDbToEnt(ent->r.mOwner) == nullptr)
        {
            ent->r.mOwner.mHandle.mVal = player->mHandle.mHandle.mVal;
            ent->s.eFlags |= 0x100000;
        }
        client->ps.vehPos = seatIdx;
        client->ps.vehType = info->type;
        client->ps.vehSubType = info->subtype;
        client->ps.eFlags = (client->ps.eFlags & 0xFFBFFFFF) | 0x300000;
        if (EntityManager::sInst->IsLocalPlayer(player))
        {
            g_femanager.mDontDrawHud = false;
            cl_aADS[EntityManager::sInst->GetPlayerIndex(player)] = 1;
        }
        client->ps.mViewLockedEntity.mHandle.mVal = ent->mHandle.mHandle.mVal;
        G_DObjUpdate(player, false);
        Scr_Notify(ent, hash_const.player_on_vehicle, 0);
    }
}

// Local Camera view (core.o Camera; 496-byte instances)
struct LocalCamera {
    uint8_t _pad[0x40];
    math::Position3 mPrevAngles;   // +0x40
    uint8_t _pad50[0x194 - 0x50];
    int     mVehicleCamMode;       // +0x194
    bool IsTweening() { return false; }
};
static LocalCamera* CameraAt(int idx)
{
    return (LocalCamera*)((char*)&gCamera + idx * 496);
}

// ea: 0x0048CE20
bool scr_vehicle_t::SetAnimRouteStage(Entity* player, Entity* ent,
                                      int routeIdx, int stageIdx)
{
    vehicle_info_t* info = s_vehicleInfos[this->infoIdx];
    Client* client = player->client;
    if (client == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 9459;
        AeAssert::gCurrentExpr = "client";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    vehicleAnimMap_t* animMap = vehicleAnimMaps[info->type];
    if (animMap == nullptr)
        return false;
    vehicleAnimRoute_t* route = &animMap->routes[routeIdx];
    if (stageIdx >= route->numStages)
    {
        if (route->vehPosDest >= 8 || stageIdx <= 0)
            return false;
        vehicleAnimMap_t* v10 = this->animMap;
        int v11 = v10->routes[routeIdx].stages[stageIdx - 1];
        vehicleAnimStage_t* stages = v10->stages;
        v11 *= 16;
        int v14 = *(int*)((char*)&stages->endTag + v11);
        vehicleAnimStage_t* v15 = (vehicleAnimStage_t*)((char*)stages + v11);
        int BoneIndex = SV_DObjGetBoneIndex(ent, animMap->tags[v14].hash);
        DObjSkelMat tagMtx;
        G_DObjGetWorldBoneIndexMatrix(ent, BoneIndex, &tagMtx);
        SetClientOrigin(player, tagMtx.origin);
        float angles[3];
        AxisToAngles((const float(*)[3])tagMtx.axis, angles);
        if (EntityManager::sInst->IsLocalPlayer(player))
        {
            int vehPosDest = route->vehPosDest;
            angles[2] = 0.0f;
            if (vehPosDest == 0 && info->type != 2)
                angles[1] = 0.0f;
            else if (vehPosDest != 1)
                angles[0] = 0.0f;
            else
                angles[0] = 0.0f;
            angles[0] = AngleNormalize180(angles[0]);
            SetClientViewAngle(player, angles);
        }
        if (G_EntLinkToWithOffsetHash(
                player, ent, animMap->tags[*(int*)((char*)v15 + 4)].hash,
                vec3_origin, vec3_origin, true) == 0)
        {
            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
            AeAssert::gCurrentLine = 9497;
            AeAssert::gCurrentExpr = "0";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
                __debugbreak();
        }
        return false;
    }
    vehicleAnimMap_t* v19 = this->animMap;
    int v20 = v19->routes[routeIdx].stages[stageIdx];
    vehicleAnimStage_t* v21 = v19->stages;
    v20 *= 16;
    int v22 = *(int*)((char*)&v21->startTag + v20);
    vehicleAnimStage_t* v23 = (vehicleAnimStage_t*)((char*)v21 + v20);
    if (G_EntLinkToWithOffsetHash(
            player, ent, animMap->tags[v22].hash, vec3_origin, vec3_origin,
            true) == 0)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 9506;
        AeAssert::gCurrentExpr = "0";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (player->client->ps.vehPos == 7)
    {
        sentient_s* sentient = player->sentient;
        if (sentient != nullptr)
        {
            char eventStr[64];
            if (sentient->eTeam == TEAM_ALLIES)
                sprintf(eventStr, "TANK_ALLIES_MANTLE_%i", stageIdx);
            else
                sprintf(eventStr, "TANK_AXIS_MANTLE_%i", stageIdx);
            PostEffectEventScriptCall(player, eventStr, false, PAK_ID_INVALID,
                                     false);
        }
    }
    client->mVehicleAnimRoute = routeIdx;
    client->mVehicleAnimMoving = true;
    client->mVehicleAnimStage = stageIdx;
    client->mVehicleAnimStageChangeTime = level.time;
    client->mFakerootOriginMatrixValid = false;
    if (stageIdx == 0)
    {
        client->mVehicleAnimStageAnim = stageIdx;
        client->mVehicleAnimStageAnimPlayed = -1;
        client->mVehicleAnimGetOut = stageIdx != 0;
        client->mVehicleAnimDisableCamera = stageIdx != 0;
        if (EntityManager::sInst->IsLocalPlayer(player))
        {
            int flags = route->flags;
            if ((flags & 1) != 0
                && ((flags & 2) == 0
                    || CameraAt(EntityManager::sInst->GetPlayerIndex(player))
                               ->mVehicleCamMode == 1 /* VEH_MODE_FIRSTPERSON */))
                client->mVehicleAnimDisableCamera = true;
        }
    }
    client->mVehicleAnimMoving = true;
    client->mVehicleAnimAngleOffset[1] = 0.0f;
    client->mVehicleAnimAngleOffset[2] = 0.0f;
    client->mVehicleAnimAngleOffset[0] = 0.0f;
    client->mVehicleAnimFirstPersonCam = v23->flags & 1;
    if ((v23->flags & 0x20) != 0)
        this->noEntryTime = level.time + 2000;
    if ((v23->flags & 0x40) != 0)
        this->forceGunnerCrouchTime = level.time + 2500;
    if (v23->flags < 0
        && HandleDbToEnt(this->mMantleEntity) != nullptr
        && HandleDbToEnt(this->mMantleEntity) != player)
        this->noExitTime = level.time + 2500;
    return true;
}

// ea: 0x0046C9E0
void VEH_InvalidateCaches(void)
{
    if (level.vehicles != nullptr)
    {
        for (int i = 0; i < level.MaxVehicles; ++i)
        {
            Entity* mObject = HandleDbToEnt(level.vehicles[i].mEntity);
            if (mObject != nullptr && mObject->proximity_data != nullptr)
            {
                mObject->proximity_data->lo.v.m128_f32[0] = 3.4028235e38f;
                mObject->proximity_data->lo.v.m128_f32[1] = 3.4028235e38f;
                mObject->proximity_data->lo.v.m128_f32[2] = 3.4028235e38f;
                mObject->proximity_data->lo.v.m128_f32[3] = 3.4028235e38f;
                mObject->proximity_data->hi.v.m128_f32[0] = -3.4028235e38f;
                mObject->proximity_data->hi.v.m128_f32[1] = -3.4028235e38f;
                mObject->proximity_data->hi.v.m128_f32[2] = -3.4028235e38f;
                mObject->proximity_data->hi.v.m128_f32[3] = -3.4028235e38f;
            }
        }
    }
}

// ea: 0x0046E0B0
bool G_IsPlayerInVehicle(Entity* player)
{
    Client* client = player->client;
    if (client == nullptr)
        return false;
    int eFlags = client->ps.eFlags;
    if ((0x100000 & eFlags) == 0 || (0x400000 & eFlags) != 0)
        return false;
    Entity* v3 = HandleDbToEnt(player->r.mOwner);
    if (v3 == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 7302;
        AeAssert::gCurrentExpr = "ent";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
        return false;
    }
    return v3->scr_vehicle != nullptr;
}

// ea: 0x0045F210
vehicle_node_t* SP_create_info_vehicle_node(void)
{
    vehicle_node_t* v0 = g_vehicleNodeManager.AllocNode();
    v0->mName.clear();
    v0->mTarget.clear();
    int v1 = v0->nextIdx;
    v0->speed = -1.0f;
    v0->lookAhead = -1.0f;
    v0->origin[0] = 0.0f;
    v0->origin[1] = 0.0f;
    v0->dir[0] = 0.0f;
    v0->dir[1] = 0.0f;
    v1 &= 0xCFFFFFFF;
    v0->nextIdx = v1;
    v0->angles[0] = s_invalidAngles[0];
    v0->angles[1] = dword_DD7418;
    float v2 = dword_DD741C;
    v0->nextIdx = v1 | 0xFFFFFFF;
    v0->angles[2] = v2;
    v0->length = 0.0f;
    ++s_numNodes;
    return v0;
}

// ea: 0x0044F3D0
float Scr_Vehicle_CalcSpeed(const scr_vehicle_t* pVehicle)
{
    if (pVehicle == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 8963;
        AeAssert::gCurrentExpr = "pVehicle";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid vehicle"))
            __debugbreak();
        return 0.0f;
    }
    rb_vehicle* mRBVeh = (rb_vehicle*)pVehicle->mRBVeh;
    if (mRBVeh == nullptr)
        return 0.0f;
    float v4[3];
    math::Dir3 vel = mRBVeh->get_velocity();
    v4[0] = vel.v.m128_f32[0];
    v4[1] = vel.v.m128_f32[1];
    v4[2] = vel.v.m128_f32[2];
    return sqrtf(v4[0] * v4[0] + v4[1] * v4[1] + v4[2] * v4[2]);
}

// ea: 0x0046F3C0
void scr_vehicle_t::AssignPhysics(Entity* player)
{
    if (mRBVeh != nullptr)
    {
        if (EntityManager::sInst->IsLocalPlayer(player)
            && mPhysicsOwner.mHandle.mVal != player->mHandle.mHandle.mVal)
        {
            if (IsPhysicsPaused())
                rb_vehicle_unpause_physics((rb_vehicle*)mRBVeh);
            rb_vehicle_update_from_network((rb_vehicle*)mRBVeh,
                                           &phys.origin, &phys.angles, &phys.vel,
                                           &phys.rotVel);
        }
    }
    mPhysicsOwner.mHandle.mVal = player->mHandle.mHandle.mVal;
}

// ea: 0x0044F580
int scr_vehicle_t::GetEntryRoute(int seatIdx, int entryIdx, bool hasFlag)
{
    vehicleAnimMap_t* animMap = this->animMap;
    if (animMap == nullptr)
        return -1;
    int numRoutes = animMap->numRoutes;
    int result = 0;
    if (numRoutes > 0)
    {
        do
        {
            if (animMap->routes[result].vehPosSrc == -1
                && animMap->routes[result].vehPosDest == seatIdx)
            {
                if (this->animMap->stages[this->animMap->routes[result].stages[0]].startTag
                        == this->animMap->entryTags[entryIdx]
                    && ((animMap->routes[result].flags & 4) == 0 || hasFlag))
                {
                    return result;
                }
                numRoutes = animMap->numRoutes;
            }
            ++result;
        } while (result < numRoutes);
    }
    return -1;
}

// ea: 0x004639D0
void G_ParseScrVehicleInfo(void)
{
    ConfigStringManager* v0 = ConfigStringManager::sInst;
    s_numVehicleInfos = 0;
    TPakId v1 = CurPakId();
    v0->CallbackSearch(v1, "VEHICLEFILE", ParseVehicleConfigString);
    ConfigStringManager* v2 = ConfigStringManager::sInst;
    TPakId v3 = CurPakId();
    v2->CallbackSearch(v3, "VEHICLEPHYSICSFILE", ParseVehiclePhysicsConfigString);
}

// ea: 0x0044D2E0
vehicle_info_t* VEH_GetInfo(int idx)
{
    return s_vehicleInfos[idx];
}

// ea: 0x0044F290
int G_GetNonPVSTankInfo(float* /*origin*/, DbLinkedHandle<EntityHandleDb, Entity> /*ent*/)
{
    return 0;
}

// ea: 0x0044F470
void G_FreeVehicleSeat(Entity* /*ent*/, Entity* /*veh*/, int /*seat*/)
{
    ;
}

// ea: 0x0044F480
int G_RequestVehicleSeat(Entity* /*ent*/, Entity* /*veh*/,
                         const HashString& /*seat*/, bool /*bForce*/)
{
    return -1;
}

// ea: 0x0044F490
int G_RequestVehicleBestSeat(Entity* /*ent*/, Entity* /*veh*/, bool /*bForce*/,
                             bool /*bPassenger*/, bool /*bCanDrive*/)
{
    return -1;
}

// ea: 0x0044F4A0
int G_StealVehicleSeat(Entity* /*ent*/, Entity* /*veh*/, int /*seat*/, bool /*bForce*/)
{
    return -1;
}

// ea: 0x0044F4B0
void Scr_Vehicle_OccupantStartEntering(scr_vehicle_t* /*veh*/, const Entity* /*ent*/,
                                       int /*seat*/)
{
    ;
}

// ea: 0x0044F4C0
void Scr_Vehicle_OccupantIsSeat(scr_vehicle_t* /*veh*/, const Entity* /*ent*/, int /*seat*/)
{
    ;
}

// ea: 0x0044F4D0
void Scr_Vehicle_OccupantStartExiting(scr_vehicle_t* /*veh*/, const Entity* /*ent*/,
                                      int /*seat*/)
{
    ;
}

// ea: 0x0044F4E0
void Scr_Vehicle_OccupantIsOut(scr_vehicle_t* /*veh*/, Entity* /*ent*/, int /*seat*/)
{
    ;
}

// ea: 0x004517F0
vehicle_node_t* GetVehicleNode(int idx)
{
    return s_nodes[idx];
}

// ea: 0x004523E0
void G_InitVehiclePaths(void)
{
    s_numNodes = 0;
}

// ea: 0x0044D2B0
void SP_script_prop_collmap(Entity* pSelf)
{
    pSelf->r.contents = 0;
    pSelf->s.eType = 17;
}

// ea: 0x0044D2F0
bool Is4WheeledVehicle(Entity* ent)
{
    scr_vehicle_t* scr_vehicle = ent->scr_vehicle;
    if (scr_vehicle == nullptr)
        return false;
    vehicle_info_t* v2 = s_vehicleInfos[scr_vehicle->infoIdx];
    if (v2->type != 1)
        return false;
    return true;
}

// ea: 0x0044F1D0
vehicle_info_t* G_GetVehicleInfo(scr_vehicle_t* veh)
{
    if (veh != nullptr)
        return s_vehicleInfos[veh->infoIdx];
    return nullptr;
}

// ea: 0x0044F4F0
vehicleAnimStage_t* scr_vehicle_t::GetRouteStage(int routeIdx, int stage)
{
    return &animMap->stages[animMap->routes[routeIdx].stages[stage]];
}

// ea: 0x0044F560
float scr_vehicle_t::GetAnimSpeedScale(Client* client)
{
    return animMap->routes[client->mVehicleAnimRoute].animSpeedScale;
}

// ea: 0x0044F680
float scr_vehicle_t::GetThrottle()
{
    rb_vehicle* mRBVeh = (rb_vehicle*)this->mRBVeh;
    if (mRBVeh != nullptr)
        return mRBVeh->m_throttle;
    return 0.0f;
}

// ea: 0x0044F790
void SP_script_vehicle_collmap(Entity* pSelf)
{
    pSelf->r.contents = 0;
    pSelf->s.eType = 16;
}

// ea: 0x0044F7B0
void vehicle_FreeDynamicBuffers(void)
{
    if (s_vehicles != nullptr)
    {
        mem_heap_free(s_vehicles);
        s_vehicles = nullptr;
        level.MaxVehicles = 0;
    }
}

// ea: 0x0045EA80
void scr_vehicle_t::ReleasePhysics(Entity* player)
{
    if (mPhysicsOwner.mHandle.mVal == player->mHandle.mHandle.mVal)
        mPhysicsOwner.mHandle.mVal = 0;
}

// ea: 0x00480860
void VEH_PlayerInteractionExit(void)
{
    Entity* Player = EntityManager::sInst->GetPlayer(currCl);
    VEH_UnlinkPlayer(Player, true);
}

// ea: 0x0045E290
void G_UpdateVehicleTags(Entity* ent)
{
    if (ent == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 6984;
        AeAssert::gCurrentExpr = "ent";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (ent->scr_vehicle == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 6985;
        AeAssert::gCurrentExpr = "ent->scr_vehicle";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    scr_vehicle_t* scr_vehicle = ent->scr_vehicle;
    scr_vehicle->boneIndex.leader = SV_DObjGetBoneIndex(ent, HashString::CalcHash("tag_leader"));
    scr_vehicle->boneIndex.player = SV_DObjGetBoneIndex(ent, HashString::CalcHash("tag_driver"));
    scr_vehicle->boneIndex.detach = SV_DObjGetBoneIndex(ent, HashString::CalcHash("tag_detach"));
    scr_vehicle->boneIndex.popout = SV_DObjGetBoneIndex(ent, HashString::CalcHash("tag_popout"));
    scr_vehicle->boneIndex.body = SV_DObjGetBoneIndex(ent, HashString::CalcHash("tag_body"));
    scr_vehicle->boneIndex.turret = SV_DObjGetBoneIndex(ent, HashString::CalcHash("tag_turret"));
    scr_vehicle->boneIndex.barrel = SV_DObjGetBoneIndex(ent, HashString::CalcHash("tag_barrel"));
    scr_vehicle->boneIndex.coax = SV_DObjGetBoneIndex(ent, HashString::CalcHash("tag_guncoax"));
    scr_vehicle->boneIndex.gunner_barrel =
        SV_DObjGetBoneIndex(ent, HashString::CalcHash("tag_gunner_barrel"));
    scr_vehicle->boneIndex.gunner_player =
        SV_DObjGetBoneIndex(ent, HashString::CalcHash("tag_gunner_player"));
    scr_vehicle->boneIndex.gunner_flash =
        SV_DObjGetBoneIndex(ent, HashString::CalcHash("tag_gunner_flash"));
    scr_vehicle->boneIndex.steering_wheel =
        SV_DObjGetBoneIndex(ent, HashString::CalcHash("tag_steeringwheel"));
    for (int i = 0; i < 4; ++i)
        scr_vehicle->boneIndex.flash[i] = SV_DObjGetBoneIndex(ent, s_flashTagHashes[i]);
    for (int i = 0; i < 6; ++i)
        scr_vehicle->boneIndex.wheel[i] = SV_DObjGetBoneIndex(ent, s_wheelTagHashes[i]);
    float maxDist = 0.0f;
    if (scr_vehicle->animMap == nullptr)
    {
        for (int i = 0; i < 6; ++i)
        {
            int v24 = SV_DObjGetBoneIndex(ent, s_entryPointTagHashes[i]);
            scr_vehicle->boneIndex.entryPoint[i] = v24;
            if (v24 >= 0)
            {
                scr_vehicle->mHasEntryPoints = true;
                G_DObjCalcBone(ent, v24);
                const DObjSkelMat* MatrixArray = SV_DObjGetMatrixArray(ent);
                const float* origin = MatrixArray[v24].origin;
                float dist = sqrt(origin[0] * origin[0] + origin[1] * origin[1]
                                  + origin[2] * origin[2]);
                if (dist > maxDist)
                    maxDist = dist;
            }
        }
    }
    else
    {
        vehicleAnimMap_t* animMap = scr_vehicle->animMap;
        if (animMap->numEntryTags >= 6)
        {
            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
            AeAssert::gCurrentLine = 7041;
            AeAssert::gCurrentExpr = "veh->animMap->numEntryTags < NUM_ENTRY_POINTS";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
                __debugbreak();
        }
        for (int i = 0; i < animMap->numEntryTags; ++i)
        {
            int v30 = SV_DObjGetBoneIndex(ent,
                                          animMap->tags[animMap->entryTags[i]].hash);
            scr_vehicle->boneIndex.entryPoint[i] = v30;
            if (v30 >= 0)
            {
                scr_vehicle->mHasEntryPoints = true;
                G_DObjCalcBone(ent, v30);
                const DObjSkelMat* v32 = SV_DObjGetMatrixArray(ent);
                const float* origin = v32[v30].origin;
                float v36 = sqrt(origin[0] * origin[0] + origin[1] * origin[1]
                                 + origin[2] * origin[2]);
                if (v36 > maxDist)
                    maxDist = v36;
            }
        }
    }
    if (maxDist > 0.0f)
        scr_vehicle->mUseRadius = (maxDist + 50.0f);
}

int scr_vehicle_t::sDebugMantle;  // ?sDebugMantle@scr_vehicle_t@@2HA
int scr_vehicle_t::sRenderEntryPoints;  // ?sRenderEntryPoints@scr_vehicle_t@@2HA
int scr_vehicle_t::sDebugAnims;         // ?sDebugAnims@scr_vehicle_t@@2HA

// ea: 0x0046F680
bool scr_vehicle_t::CanUseVehicle(Entity* player, float& distToUsePoint,
                                  int& entryPoint)
{
    if (player == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 9909;
        AeAssert::gCurrentExpr = "player";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid pointer"))
            __debugbreak();
    }
    Entity* vehicle = HandleDbToEnt(mEntity);
    if (vehicle == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 9914;
        AeAssert::gCurrentExpr = "vehicle";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid pointer."))
            __debugbreak();
    }
    float playerPos[4];
    playerPos[0] = player->r.currentOrigin.v.m128_f32[0];
    playerPos[1] = player->r.currentOrigin.v.m128_f32[1];
    playerPos[2] = player->r.currentOrigin.v.m128_f32[2];
    playerPos[3] = player->r.currentOrigin.v.m128_f32[3];
    float vehCenter[3];
    vehCenter[0] = (vehicle->r.absmax.v.m128_f32[0] + vehicle->r.absmin.v.m128_f32[0]) * 0.5f;
    vehCenter[1] = (vehicle->r.absmax.v.m128_f32[1] + vehicle->r.absmin.v.m128_f32[1]) * 0.5f;
    vehCenter[2] = (vehicle->r.absmax.v.m128_f32[2] + vehicle->r.absmin.v.m128_f32[2]) * 0.5f;
    float dx = playerPos[0] - vehCenter[0];
    float dy = playerPos[1] - vehCenter[1];
    float dz = playerPos[2] - vehCenter[2];
    float dist2 = dx * dx + dy * dy + dz * dz;
    if (dist2 > (mUseRadius * mUseRadius))
        return false;
    if (!mHasEntryPoints)
    {
        distToUsePoint = sqrt(dist2);
        return true;
    }
    int numEntryTags = animMap != nullptr ? animMap->numEntryTags : 6;
    float bestDist = 2500.0f;
    int bestPoint = -1;
    for (int i = 0; i < numEntryTags; ++i)
    {
        int bone = boneIndex.entryPoint[i];
        if (bone < 0)
            continue;
        Client* client = player->client;
        if (client == nullptr
            || client->ps.ctf_has_flag == 0
            || s_vehicleInfos[infoIdx]->type != 2
                && (int)(sEntryPointSeatAssociation[i]) >= 2
                && (int)(sEntryPointSeatAssociation[i]) <= 5)
        {
            if (s_vehicleInfos[infoIdx]->type == 2
                || HandleDbToEnt(seats[sEntryPointSeatAssociation[i]].occupant) == nullptr)
            {
                DObjSkelMat tagMat;
                G_DObjGetWorldBoneIndexMatrix(vehicle, bone, &tagMat);
                float ex = playerPos[0] - tagMat.origin[0];
                float ey = playerPos[1] - tagMat.origin[1];
                float ez = playerPos[2] - tagMat.origin[2];
                float d = ex * ex + ey * ey + ez * ez;
                if (bestDist > d)
                {
                    bestDist = d;
                    bestPoint = i;
                }
            }
        }
    }
    if (bestPoint < 0)
        return false;
    distToUsePoint = sqrt(bestDist);
    entryPoint = bestPoint;
    return true;
}

// ea: 0x0046FA60
bool scr_vehicle_t::CanMantleVehicle(Entity* player)
{
    if (player == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 10032;
        AeAssert::gCurrentExpr = "player";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid player"))
            __debugbreak();
    }
    if (player->sentient == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 10033;
        AeAssert::gCurrentExpr = "player->sentient";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Player does not have a valid sentient"))
            __debugbreak();
    }
    if (s_vehicleInfos[infoIdx]->type != 2)
        return false;
    if (mMantleTime != 0)
        return false;
    if (HandleDbToEnt(seats[7].occupant) != nullptr)
        return false;
    Entity* vehEnt = HandleDbToEnt(mEntity);
    if (vehEnt == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 10040;
        AeAssert::gCurrentExpr = "*mEntity";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Invalid entity handle in the vehicle"))
            __debugbreak();
    }
    if (HandleDbToEnt(seats[1].occupant) != nullptr
        || HandleDbToEnt(seats[6].occupant) != nullptr)
        return false;
    Entity* driver = HandleDbToEnt(vehEnt->r.mOwner);
    if (driver != nullptr && driver->sentient == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 10048;
        AeAssert::gCurrentExpr = "!driver || driver->sentient";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Driver is not a sentient"))
            __debugbreak();
    }
    Entity* v7 = player;
    if (player->IsLocalPlayer())
    {
        int v8 = player->client->ps.weaponslots[4];
        weaponFileInfo_t* InfoForWeapon = BG_GetInfoForWeapon(v8);
        if (InfoForWeapon == nullptr
            || InfoForWeapon->bCanMantle == 0
            || player->client->ps.ammoclip[BG_ClipForWeapon(v8)] <= 0)
            return false;
        v7 = player;
    }
    Client* client = v7->client;
    if (client != nullptr && client->ps.ctf_has_flag != 0)
        return false;
    Entity* phyOwner = HandleDbToEnt(mPhysicsOwner);
    if (phyOwner != nullptr
        && phyOwner->IsLocalPlayer()
        && sqrt(phys.vel.v.m128_f32[0] * phys.vel.v.m128_f32[0]
                + phys.vel.v.m128_f32[1] * phys.vel.v.m128_f32[1]
                + phys.vel.v.m128_f32[2] * phys.vel.v.m128_f32[2])
               > ((rb_vehicle*)mRBVeh)->m_parameter->m_speed_max - 5.0f)
        return false;
    if (sDebugMantle != 0
        || driver != nullptr
            && (!cgGlobal.teamGame
                || driver->sentient->eTeam != player->sentient->eTeam))
        return true;
    return false;
}

// ea: 0x0044F7E0
float Scr_Vehicle_DamageScale(Entity* pSelf, Entity* pAttacker,
                              Entity* pInflictor, const float* point, int mod)
{
    scr_vehicle_t* scr_vehicle = pSelf->scr_vehicle;
    vehicle_info_t* v6 = s_vehicleInfos[scr_vehicle->infoIdx];
    auto* p_phys = &scr_vehicle->phys;
    float width = cos(0.3490658700466156f);
    float scalar_best_side;
    switch (mod)
    {
    case 4: case 6: case 10: case 18:
        scalar_best_side = 1.0f;
        break;
    default:
        scalar_best_side = 0.0f;
        break;
    }
    float bulletDamage;
    switch (mod)
    {
    case 1: case 2:
        bulletDamage = v6->bulletDamage;
        break;
    case 3: case 4:
        bulletDamage = v6->grenadeDamage;
        break;
    case 5: case 6:
        bulletDamage = v6->mineDamage;
        break;
    case 9: case 10: case 17: case 18:
        bulletDamage = v6->projectileDamage;
        break;
    default:
        bulletDamage = 1.0f;
        break;
    }
    float scale = bulletDamage;
    float axis[3][3];
    AnglesToAxis(scr_vehicle->phys.angles, axis);
    float vdir[3];
    vdir[0] = point[0] - p_phys->origin.v.m128_f32[0];
    vdir[1] = point[1] - p_phys->origin.v.m128_f32[1];
    vdir[2] = 0.0f;
    VectorNormalize(vdir);
    float dotAxis = (axis[0][0] * vdir[0]) + (axis[0][1] * vdir[1]) + (axis[0][2] * vdir[2]);
    float dotOther = (axis[1][0] * vdir[0]) + (axis[1][1] * vdir[1]) + (axis[1][2] * vdir[2]);
    float bestDot = dotAxis;
    int v11 = 0;
    if (fabs(dotOther) > fabs(dotAxis))
    {
        bestDot = dotOther;
        v11 = 1;
    }
    if (scalar_best_side == 0.0f)
    {
        if (v11 != 0)
            return scale * 1.5f;
        if (bestDot >= 0.0f)
            return scale * 1.0f;
        if (width > -bestDot)
            return scale * 1.5f;
        return scale + scale;
    }
    scalar_best_side = 1.0f;
    float scalar_worst_side = 1.0f;
    width = pSelf->r.maxs.v.m128_f32[1] * 0.80000001f;
    float height = pSelf->r.maxs.v.m128_f32[2] * 0.5f;
    float dx = point[0] - p_phys->origin.v.m128_f32[0];
    float dy = point[1] - p_phys->origin.v.m128_f32[1];
    float dz = point[2] - p_phys->origin.v.m128_f32[2];
    float dist = dx * dx + dy * dy + dz * dz;
    if (mod == 4)
    {
        if ((p_phys->origin.v.m128_f32[2] + height) > (point[2] - 10.0f)
            && (width * width) > dist)
            return scale + scale;
        return scale;
    }
    float v14;
    if (v11 != 0)
    {
        scalar_best_side = 1.5f;
        float frontDot = (axis[0][0] * vdir[0]) + (axis[0][1] * vdir[1]) + (axis[0][2] * vdir[2]);
        v14 = frontDot >= 0.0f ? 1.0f : 2.0f;
    }
    else
    {
        v14 = 2.0f;
        if (bestDot >= 0.0f)
            v14 = 1.0f;
        scalar_best_side = v14;
        v14 = 1.5f;
    }
    float v16 = fabs(bestDot);
    return ((1.0f - v16) * v14 + scalar_best_side * v16) * scale * 0.69999999f;
}

static int last_use;  // @ 0xEF5934 (g_scr_vehicle.cpp local)

// ea: 0x00480DA0
void Scr_Vehicle_Use(Entity* pEnt, Entity* pOther, Entity* /*unused*/)
{
    Client* v21 = pOther->client;
    if (v21 == nullptr)
        return;
    if (last_use != 0 && last_use > level.time - 1000 && last_use <= level.time)
        return;
    last_use = level.time;
    if ((0x100000 & v21->ps.eFlags) == 0)
    {
        if (IsVehFlipped(pEnt))
        {
            math::Position3 hitp;
            hitp.v = pOther->r.currentOrigin.v;
            math::Dir3 hitd;
            hitd.v = _mm_setzero_ps();
            hitd.v.m128_f32[2] = 1.0f;
            ApplyPhysics(pEnt, &hitp, &hitd, 70.0f, false, HITLOC_TORSO_UPR);
        }
        else if (v21->ps.ctf_has_flag == 0
                 || s_vehicleInfos[pEnt->scr_vehicle->infoIdx]->type != 2)
        {
            if (pEnt->scr_vehicle->CanMantleVehicle(pOther))
            {
                MultiplayerMgr::sInst->AttemptToGetInVehicle(pEnt, pOther, 7,
                                                             v21->mVehicleAnimRoute);
            }
            else
            {
                scr_vehicle_t* scr_vehicle = pEnt->scr_vehicle;
                if (scr_vehicle == nullptr)
                {
                    AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
                    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
                    AeAssert::gCurrentLine = 8906;
                    AeAssert::gCurrentExpr = "pEnt->scr_vehicle";
                    if (!AeAssert::IsIgnored()
                        && AeAssert::Assert("Entity not a vehicle"))
                        __debugbreak();
                }
                if (s_vehicleInfos[scr_vehicle->infoIdx]->type == 2)
                {
                    int v17 = 0;
                    while (HandleDbToEnt(scr_vehicle->seats[v17].occupant) != nullptr)
                    {
                        ++v17;
                        if (v17 > 10)
                            return;
                    }
                    MultiplayerMgr::sInst->AttemptToGetInVehicle(pEnt, pOther, v17,
                                                                 v21->mVehicleAnimRoute);
                }
                else
                {
                    float distToUsePoint;
                    int entryPoint;
                    if (!scr_vehicle->CanUseVehicle(pOther, distToUsePoint,
                                                    entryPoint))
                    {
                        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
                        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
                        AeAssert::gCurrentLine = 8934;
                        AeAssert::gCurrentExpr = "canUseVehicle";
                        if (!AeAssert::IsIgnored()
                            && AeAssert::Assert("Player not in an entry point"))
                            __debugbreak();
                    }
                    if (pOther->client->ps.ctf_has_flag == 0
                        || sEntryPointSeatAssociation[entryPoint] == 2)
                    {
                        int seatIdx = sEntryPointSeatAssociation[entryPoint];
                        if (HandleDbToEnt(pEnt->scr_vehicle->seats[seatIdx].occupant)
                            == nullptr)
                            MultiplayerMgr::sInst->AttemptToGetInVehicle(
                                pEnt, pOther, seatIdx, v21->mVehicleAnimRoute);
                    }
                }
            }
        }
    }
    else
    {
        scr_vehicle_t* v4 = pEnt->scr_vehicle;
        if (v4->noExitTime > level.time)
            return;
        if (v21->ps.vehType == 2 && v21->ps.vehPos == 0
            && HandleDbToEnt(v4->seats[7].occupant) != nullptr)
        {
            *(int*)((char*)v21 + 0xB78) = level.time;
        }
        else if (v21->ps.vehPos == 6)
        {
            MultiplayerMgr::sInst->AttemptVehicleSeatChange(pEnt, pOther, 1);
        }
        else if (IsPlayerFullySeatedInVehicle(pOther))
        {
            scr_vehicle_t* v6 = pEnt->scr_vehicle;
            vehicleAnimMap_t* animMap = v6->animMap;
            if (animMap != nullptr && animMap->exitMap != nullptr)
            {
                Client* client = pOther->client;
                int vehPos = client->ps.vehPos;
                int v11 = animMap->exitMap[vehPos];
                if (v11 == -1
                    || v6->GetSwitchPosRoute(v11, vehPos,
                                             client->ps.ctf_has_flag != 0) < 0)
                {
                    tlPrintf("=======================================GetOutOfVehicle due to a use during an anim\n");
                    MultiplayerMgr::sInst->GetOutOfVehicle(
                        pEnt, pOther->client->ps.vehPos);
                }
                else
                {
                    MultiplayerMgr::sInst->AttemptVehicleSeatChange(pEnt, pOther,
                                                                    v11);
                }
            }
            else
            {
                tlPrintf("=======================================GetOutOfVehicle due to a use\n");
                MultiplayerMgr::sInst->GetOutOfVehicle(pEnt,
                                                       pOther->client->ps.vehPos);
            }
        }
        else
        {
            Client* v5 = pOther->client;
            if (v5->ps.vehType == 2 && v5->mVehicleAnimStage <= 4)
                v5->mVehicleAnimGetOut = true;
        }
    }
}

static int s_newDebugLineLocal;  // @ 0xDD7410
static float s_start[3];   // @ 0xEF392C
static float s_end[3];     // @ 0xEF393C
static float s_dir[3];     // @ 0xEF394C

// ea: 0x0045EC30 (file-local)
static void VP_AddDebugLine(const float* start, const float* end, int forceDraw)
{
    float dir[3];
    dir[0] = end[0] - start[0];
    dir[1] = end[1] - start[1];
    dir[2] = end[2] - start[2];
    VectorNormalize(dir);
    if (s_newDebugLineLocal != 0)
    {
        s_newDebugLineLocal = 0;
    }
    else
    {
        if ((s_dir[0] * dir[0]) + (s_dir[1] * dir[1]) + (s_dir[2] * dir[2])
                >= 0.99989998f
            && forceDraw == 0)
        {
            s_end[0] = end[0];
            s_end[1] = end[1];
            s_end[2] = end[2];
            return;
        }
        float k_lineColor[4] = {1.0f, 0.0f, 0.0f, 1.0f};
        CL_AddDebugLine(s_start, s_end, k_lineColor, 1, 0, 1, 0);
    }
    s_start[0] = start[0];
    s_start[1] = start[1];
    s_start[2] = start[2];
    s_end[0] = end[0];
    s_end[1] = end[1];
    s_end[2] = end[2];
    s_dir[0] = dir[0];
    s_dir[1] = dir[1];
    s_dir[2] = dir[2];
}

// ea: 0x00464710
void VP_DrawPath(const vehicle_pathpos_t* vpp)
{
    vehicle_pathpos_t prevVPP = *vpp;
    vehicle_pathpos_t nextVPP = *vpp;
    s_newDebugLineLocal = 1;
    int v3 = 0;
    int loopNode = -1;
    int count = 0;
    while (1)
    {
        count = count + 1;
        if (count > 50000)
            break;
        if (prevVPP.nodeIdx != vpp->nodeIdx)
            loopNode = vpp->nodeIdx;
        prevVPP = nextVPP;
        int updated = G_VehUpdatePathPos(nullptr, &nextVPP, false,
                                         ServerTime::sInst.mTickMSec, loopNode);
        if (nextVPP.endOfPath != 0 || updated != 0)
            v3 = 1;
        VP_AddDebugLine(prevVPP.origin, nextVPP.origin, v3);
        if (v3 != 0)
            goto draw_boxes;
    }
    Com_Printf("WARNING: Invalid vehicle path.  Possible infinite loop\n");
draw_boxes:
    int nodeIdx = vpp->nodeIdx;
    vehicle_node_t* v7 = s_nodes[nodeIdx];
    float k_boxColor1[4] = {0.0f, 1.0f, 0.0f, 1.0f};
    float k_boxColor2[4] = {0.0f, 0.0f, 1.0f, 1.0f};
    int v11 = 0;
    for (int v8 = 0; v8 < s_numNodes; v7 = s_nodes[v11])
    {
        vehicle_node_t* v9 = s_nodes[nodeIdx];
        float mins[3] = {v7->origin[0] + 4.0f, v7->origin[1] + 4.0f,
                         v7->origin[2] + 4.0f};
        float maxs[3] = {v7->origin[0] - 4.0f, v7->origin[1] - 4.0f,
                         v7->origin[2] - 4.0f};
        ++v8;
        const float* v10 = v7 != v9 ? k_boxColor2 : k_boxColor1;
        G_DebugBox(mins, maxs, v10, 1, 0, 0);
        int v11 = (v7->nextIdx << 18) >> 18;
        if (v11 < 0)
            break;
        if (v11 == nodeIdx)
            break;
    }
}

// ea: 0x00464980
void G_DrawVehiclePaths()
{
    vehicle_pathpos_t vpp;
    memset(&vpp, 0, sizeof(vpp));
    if (g_vehicleDrawPath.string[0] == 0 || g_vehicleDrawPath.string[0] == '0')
        return;
    int16_t v0 = 0;
    if (s_numNodes > 0)
    {
        int v1 = 0;
        while (1)
        {
            Broc::string::Block* mBlock = s_nodes[v1]->mName.mBlock;
            const char* v3 = mBlock != nullptr ? (const char*)&mBlock[1]
                                               : defaultFileName;
            if (_stricmp(v3, g_vehicleDrawPath.string) == 0)
                break;
            v1 = ++v0;
            if (v0 >= s_numNodes)
                goto done;
        }
        vpp.switchNode[0].mName.clear();
        vpp.switchNode[0].mTarget.clear();
        vpp.switchNode[1].mName.clear();
        vpp.switchNode[1].mTarget.clear();
        G_VehSetUpPathPos(&vpp, v0);
        VP_DrawPath(&vpp);
    }
done:
    ;
}

static float VEH_LerpAngle(float targetAngle, float currentAngle, float rate)
{
    while (targetAngle - currentAngle > 180.0f)
        targetAngle -= 360.0f;
    while (targetAngle - currentAngle < -180.0f)
        targetAngle += 360.0f;
    float delta = targetAngle - currentAngle;
    float step = ServerTime::sInst.mTickDelta * rate;
    if (delta <= 0.0f)
        step = -step;
    if (fabsf(delta) <= 0.005f || fabsf(step) > fabsf(delta))
        return AngleNormalize180(targetAngle);
    return AngleNormalize180(step + currentAngle);
}

// ea: 0x0047E030
void VEH_UpdateGunnerAim(Entity* ent)
{
    scr_vehicle_t* scr_vehicle = ent->scr_vehicle;
    vehicle_info_t* info =
        scr_vehicle != nullptr ? s_vehicleInfos[scr_vehicle->infoIdx] : nullptr;
    Entity* mObject = HandleDbToEnt(scr_vehicle->seats[1].occupant);
    if (mObject == nullptr)
    {
        Entity* v6 = HandleDbToEnt(scr_vehicle->seats[6].occupant);
        if (v6 == nullptr)
        {
            float targetYaw = 0.0f;
            if (info->type == 2 && info->vehicleAnimMatrixColumn == 0)
                targetYaw = 45.0f;
            scr_vehicle->current.mGunnerAngles.v.m128_f32[1] =
                VEH_LerpAngle(targetYaw,
                              scr_vehicle->current.mGunnerAngles.v.m128_f32[1],
                              120.0f);
            scr_vehicle->current.mGunnerAngles.v.m128_f32[0] =
                VEH_LerpAngle(0.0f,
                              scr_vehicle->current.mGunnerAngles.v.m128_f32[0],
                              90.0f);
            scr_vehicle->next.mGunnerAngles.v =
                scr_vehicle->current.mGunnerAngles.v;
        }
        return;
    }
    if (ent->health <= 0)
        goto no_target;
    Client* client = mObject->client;
    if (client != nullptr)
    {
        if (client->mVehicleAnimMoving || client->mVehicleAnimPauseRemoteAngles)
        {
            scr_vehicle->current.mGunnerAngles.v.m128_f32[0] =
                VEH_LerpAngle(0.0f,
                              scr_vehicle->current.mGunnerAngles.v.m128_f32[0],
                              info->turretRotRate);
            return;
        }
        if (EntityManager::sInst->IsLocalPlayer(mObject)
            && CameraAt(EntityManager::sInst->GetPlayerIndex(mObject))
                   ->IsTweening())
        {
            scr_vehicle->current.mGunnerAngles.v.m128_f32[0] =
                VEH_LerpAngle(0.0f,
                              scr_vehicle->current.mGunnerAngles.v.m128_f32[0],
                              info->turretRotRate);
            return;
        }
    }
no_target:
    Entity* tgtEnt = HandleDbToEnt(scr_vehicle->mGunnerTargetEnt);
    float tgtDir[3];
    if (tgtEnt != nullptr)
    {
        tgtDir[0] = tgtEnt->r.currentOrigin.v.m128_f32[0]
                    + scr_vehicle->gunnerTargetOffset[0];
        tgtDir[1] = tgtEnt->r.currentOrigin.v.m128_f32[1]
                    + scr_vehicle->gunnerTargetOffset[1];
        tgtDir[2] = tgtEnt->r.currentOrigin.v.m128_f32[2]
                    + scr_vehicle->gunnerTargetOffset[2];
    }
    else
    {
        tgtDir[0] = scr_vehicle->gunnerTargetOrigin[0];
        tgtDir[1] = scr_vehicle->gunnerTargetOrigin[1];
        tgtDir[2] = scr_vehicle->gunnerTargetOrigin[2];
        tgtEnt = nullptr;
    }
    if (scr_vehicle->boneIndex.gunner_barrel >= 0)
    {
        DObjSkelMat barrelMtx;
        G_DObjGetWorldBoneIndexMatrix(ent, scr_vehicle->boneIndex.gunner_barrel,
                                      &barrelMtx);
        float tgtPos[3];
        tgtPos[0] = ent->r.currentAngles.v.m128_f32[0];
        tgtPos[1] = ent->r.currentAngles.v.m128_f32[1];
        tgtPos[2] = AngleNormalize360(
            scr_vehicle->current.mTurretAngles.v.m128_f32[1]
            + ent->r.currentAngles.v.m128_f32[1]);
        float angles[3];
        Entity* v22 = HandleDbToEnt(scr_vehicle->seats[1].occupant);
        if (v22 != nullptr && v22->client != nullptr)
        {
            float ps[3];
            float spanDownScale[3];
            ps[0] = v22->client->ps.viewangles[0];
            ps[1] = v22->client->ps.viewangles[1];
            float tgtAxis[3][3];
            AnglesToAxis(tgtPos, tgtAxis);
            float viewAxis[3][3];
            AnglesToAxis(ps, viewAxis);
            float invTgt[3][3];
            MatrixTranspose(tgtAxis, invTgt);
            float rel[3][3];
            MatrixMultiply(viewAxis, invTgt, rel);
            AxisToAngles(rel, angles);
            float deltaAngles[3];
            AnglesSubtract(*(const math::Position3*)angles,
                           scr_vehicle->current.mGunnerAngles,
                           *(math::Position3*)deltaAngles);
            float absPitch = fabs(deltaAngles[0]);
            float absYaw = fabs(deltaAngles[1]);
            float deltaYAW = AngleNormalize180(
                AngleNormalize360(v22->client->ps.viewangles[1])
                - tgtPos[2]);
            float pitchScale = 1.0f;
            if (info->type == 2)
            {
                if (deltaYAW >= deltaYAWmins && deltaYAWmaxs >= deltaYAW)
                {
                    float psin, pcos;
                    FastSinCos(((deltaYAW - deltaYAWmins)
                                / (deltaYAWmaxs - deltaYAWmins))
                                   * 3.1415927f,
                               &psin, &pcos);
                    float v34 = minPitch / info->turretGunnerVertSpanUp;
                    pitchScale = ((1.0f - v34) * (1.0f - psin)) + v34;
                }
            }
            if (v22 == HandleDbToEnt(scr_vehicle->seats[1].occupant))
            {
                scr_vehicle->next.mGunnerAngles.v.m128_f32[0] =
                    AngleNormalize180(angles[0]);
                scr_vehicle->next.mGunnerAngles.v.m128_f32[1] =
                    AngleNormalize180(angles[1]);
                float pitch = -info->turretGunnerVertSpanDown;
                if (pitch <= scr_vehicle->next.mGunnerAngles.v.m128_f32[0])
                {
                    pitch = info->turretGunnerVertSpanUp * pitchScale;
                    if (scr_vehicle->next.mGunnerAngles.v.m128_f32[0] <= pitch)
                        pitch = scr_vehicle->next.mGunnerAngles.v.m128_f32[0];
                }
                scr_vehicle->next.mGunnerAngles.v.m128_f32[0] = pitch;
                scr_vehicle->current.mGunnerAngles.v.m128_f32[0] = pitch;
                scr_vehicle->current.mGunnerAngles.v.m128_f32[1] =
                    scr_vehicle->next.mGunnerAngles.v.m128_f32[1];
            }
            if (v22->client != nullptr)
            {
                float* viewYaw = &v22->client->ps.viewangles[1];
                float minYaw = -info->turretGunnerVertSpanDown;
                if (minYaw <= *viewYaw)
                {
                    float maxYaw = info->turretGunnerVertSpanUp * pitchScale;
                    if (*viewYaw <= maxYaw)
                        minYaw = *viewYaw;
                }
                *viewYaw = minYaw;
                float viewAngles[3] = {
                    v22->client->ps.viewangles[0],
                    *viewYaw,
                    v22->client->ps.viewangles[2],
                };
                SetClientViewAngle(v22, viewAngles);
                if (fabs(deltaYAW) > 140.0f /* turretGunnerVertSpanUp */)
                {
                    float newYaw = deltaYAW <= 0.0f
                                       ? AngleNormalize360(tgtPos[2] - 140.0f)
                                       : AngleNormalize360(tgtPos[2] + 140.0f);
                    v22->client->ps.viewangles[1] = newYaw;
                    float newAngles[3] = {
                        v22->client->ps.viewangles[0], newYaw,
                        v22->client->ps.viewangles[2],
                    };
                    SetClientViewAngle(v22, newAngles);
                }
            }
        }
        else if (scr_vehicle->hasGunnerTarget != 0)
        {
            float rel[3];
            rel[0] = tgtDir[0] - barrelMtx.origin[0];
            rel[1] = tgtDir[1] - barrelMtx.origin[1];
            rel[2] = tgtDir[2] - barrelMtx.origin[2];
            VectorNormalize(rel);
            float angles[3];
            vectoangles(rel, angles);
            float tgtAxis[3][3];
            AnglesToAxis(tgtPos, tgtAxis);
            float viewAxis[3][3];
            AnglesToAxis(angles, viewAxis);
            float invTgt[3][3];
            MatrixTranspose(tgtAxis, invTgt);
            float relMtx[3][3];
            MatrixMultiply(viewAxis, invTgt, relMtx);
            AxisToAngles(relMtx, angles);
            scr_vehicle->next.mGunnerAngles.v.m128_f32[0] =
                VEH_LerpAngle(angles[0],
                              scr_vehicle->current.mGunnerAngles.v.m128_f32[0],
                              info->turretRotRate);
            scr_vehicle->next.mGunnerAngles.v.m128_f32[1] =
                VEH_LerpAngle(angles[1],
                              scr_vehicle->current.mGunnerAngles.v.m128_f32[1],
                              info->turretRotRate);
            float pitch = -info->turretGunnerVertSpanDown;
            if (pitch <= scr_vehicle->next.mGunnerAngles.v.m128_f32[0])
            {
                if (scr_vehicle->next.mGunnerAngles.v.m128_f32[0]
                    <= info->turretGunnerVertSpanUp)
                    pitch = scr_vehicle->next.mGunnerAngles.v.m128_f32[0];
                else
                    pitch = info->turretGunnerVertSpanUp;
            }
            scr_vehicle->next.mGunnerAngles.v.m128_f32[0] = pitch;
            float yaw = -info->turretGunnerVertSpanDown;
            if (yaw <= scr_vehicle->next.mGunnerAngles.v.m128_f32[1])
            {
                if (scr_vehicle->next.mGunnerAngles.v.m128_f32[1]
                    <= info->turretGunnerVertSpanUp)
                    yaw = scr_vehicle->next.mGunnerAngles.v.m128_f32[1];
                else
                    yaw = info->turretGunnerVertSpanUp;
            }
            scr_vehicle->next.mGunnerAngles.v.m128_f32[1] = yaw;
            scr_vehicle->current.mGunnerAngles.v.m128_f32[0] =
                scr_vehicle->next.mGunnerAngles.v.m128_f32[0];
            scr_vehicle->current.mGunnerAngles.v.m128_f32[1] =
                scr_vehicle->next.mGunnerAngles.v.m128_f32[1];
            float deltaAngles[3];
            AnglesSubtract(*(const math::Position3*)angles,
                           scr_vehicle->current.mGunnerAngles,
                           *(math::Position3*)deltaAngles);
            float absPitch = fabs(deltaAngles[0]);
            float absYaw = fabs(deltaAngles[1]);
            if (scr_vehicle->hasGunnerTarget != 0
                && absPitch < 1.0f && absYaw < 1.0f)
            {
                Scr_Notify(ent, hash_const.turret_on_target, 0);
                if (tgtEnt != nullptr
                    && ((com_frameNumber + ((int)ent >> 5)) & 3) != 0)
                {
                    int hitNum = 0;
                    collision_context_t ctx(1);
                    ctx.pass_entity1.mHandle.mVal = ent->mHandle.mHandle.mVal;
                    ctx.pass_entity2.mHandle.mVal =
                        tgtEnt->mHandle.mHandle.mVal;
                    math::Position3 zeroA;
                    math::Position3 zeroB;
                    zeroA.v = _mm_setzero_ps();
                    zeroB.v = _mm_setzero_ps();
                    math::Position3 start;
                    start.v.m128_f32[0] = barrelMtx.origin[0];
                    start.v.m128_f32[1] = barrelMtx.origin[1];
                    start.v.m128_f32[2] = barrelMtx.origin[2];
                    math::Position3 end;
                    end.v.m128_f32[0] = tgtDir[0];
                    end.v.m128_f32[1] = tgtDir[1];
                    end.v.m128_f32[2] = tgtDir[2];
                    g_SightTrace(&scr_vehicle->turretHitNum, start, zeroA,
                                 zeroB, end, ctx);
                    if (scr_vehicle->turretHitNum == 0)
                        Scr_Notify(ent, hash_const.turret_on_vistarget, 0);
                }
            }
        }
        else
        {
            scr_vehicle->next.mGunnerAngles.v.m128_f32[0] =
                VEH_LerpAngle(0.0f,
                              scr_vehicle->current.mGunnerAngles.v.m128_f32[0],
                              info->turretRotRate);
            scr_vehicle->current.mGunnerAngles.v.m128_f32[0] =
                scr_vehicle->next.mGunnerAngles.v.m128_f32[0];
            scr_vehicle->next.mGunnerAngles.v.m128_f32[1] =
                VEH_LerpAngle(0.0f,
                              scr_vehicle->current.mGunnerAngles.v.m128_f32[1],
                              info->turretRotRate);
            scr_vehicle->current.mGunnerAngles.v.m128_f32[1] =
                scr_vehicle->next.mGunnerAngles.v.m128_f32[1];
        }
    }
}

static int lastGunnerCrouchMsgLocal;  // @ 0xEF59D4
int byte_A00000 = 0xA00000;           // @ .data 0xDD6B40

// ea: 0x00490ED0
// ea: 0x0048D200
void scr_vehicle_t::UpdateAnimRoute(Entity* ent, Entity* player)
{
    Client* client = player->client;
    if (client == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 9635;
        AeAssert::gCurrentExpr = "client";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    vehicle_info_t* info = s_vehicleInfos[infoIdx];
    vehicleAnimMap_t* animMap = vehicleAnimMaps[info->type];
    if (animMap == nullptr)
        return;
    int route = client->mVehicleAnimRoute;
    int stageIdx = client->mVehicleAnimStage;
    vehicleAnimStage_t* stage =
        &animMap->stages[animMap->routes[route].stages[stageIdx]];
    if ((stage->flags & 2) != 0)
    {
        if (client->mVehicleAnimStageChangeTime
            < level.time - (info->vehicleAnimMatrixColumn != 0 ? 50 : 200))
        {
            DObjSkelMat mtx;
            int bone = SV_DObjGetBoneIndex(
                ent, animMap->tags[stage->startTag].hash);
            G_DObjGetWorldBoneIndexMatrix(ent, bone, &mtx);
            float angles1[3];
            AxisToAngles((const float(*)[3])mtx.axis, angles1);
            bone = SV_DObjGetBoneIndex(ent,
                                       animMap->tags[stage->endTag].hash);
            G_DObjGetWorldBoneIndexMatrix(ent, bone, &mtx);
            float angles2[3];
            AxisToAngles((const float(*)[3])mtx.axis, angles2);
            float delta =
                AngleNormalize180(AngleSubtract(angles2[1], angles1[1]));
            float rate = fabsf(delta) * 1.3333334f;
            float angleb = VEH_LerpAngle(delta,
                                         client->mVehicleAnimAngleOffset[1],
                                         rate);
            client->mVehicleAnimAngleOffset[1] =
                AngleNormalize180(angleb);
            client = player->client;
            if (fabsf(client->mVehicleAnimAngleOffset[1] - delta) > 2.0f)
                return;
        }
    }
    if (client->mVehicleAnimStageAnim > client->mVehicleAnimStage)
    {
        VEH_UpdateControllers(ent, 0);
        if (sDebugAnims == 0
            || controller_button_pressed(controller::inst(), 0, 7))
        {
            if (EntityManager::sInst->IsLocalPlayer(player)
                && (stage->flags & 1) != 0)
            {
                Client* c = player->client;
                int clipIdx = BG_ClipForWeapon(c->ps.weaponslots[4]);
                if (c->ps.ammoclip[clipIdx] > 0)
                {
                    --c->ps.ammoclip[clipIdx];
                    MultiplayerMgr::sInst->VehicleMantled(ent, player);
                }
                else
                {
                    c->mVehicleAnimGetOut = true;
                }
            }
            if ((stage->flags & 0x10) != 0
                || player->client->mVehicleAnimGetOut)
            {
                if (EntityManager::sInst->IsLocalPlayer(player))
                {
                    MultiplayerMgr::sInst->GetOutOfVehicle(
                        ent, player->client->ps.vehPos);
                }
                else
                {
                    G_EntUnlink(player);
                }
                Client* c = player->client;
                player->flags &= ~0x1000000;
                if (c->ps.ctf_has_flag == 0)
                    c->mVehicleNoWeaponTime = level.time + 300;
            }
            if (!SetAnimRouteStage(
                    player, ent, player->client->mVehicleAnimRoute,
                    player->client->mVehicleAnimStageAnim))
            {
                player->client->mVehicleAnimMoving = false;
                if (player->IsLocalPlayer()
                    && player->client->ps.vehPos == 2)
                {
                    int idx = player->GetPlayerIndex();
                    weaponFileInfo_t* weap =
                        BG_GetInfoForWeapon(cg_aWeaponSelect[idx]);
                    if (weap == nullptr || weap->type == WEAPTYPE_INTERACT)
                    {
                        if (CG_SelectFirstWeaponInSlotWithLocalIndex(1, 1,
                                                                     idx)
                            == 0)
                        {
                            if (CG_SelectFirstWeaponNotInSlotWithLocalIndex(
                                    1, 1, idx)
                                == 0)
                            {
                                CG_SelectFirstWeaponInSlotWithLocalIndex(1, 0,
                                                                         idx);
                            }
                        }
                    }
                }
            }
        }
        return;
    }
    if (player->client->ps.vehPos == 7 && client->mVehicleAnimStage < 2)
    {
        collision_context_t ctx(0x2000000);
        int bone = boneIndex.barrel;
        if (bone >= 0)
        {
            DObjSkelMat mtx;
            G_DObjGetWorldBoneIndexMatrix(ent, bone, &mtx);
            float dist =
                VectorDistance(&mtx.origin[0],
                               &player->r.currentOrigin.v.m128_f32[0]);
            float barrelExit[3];
            barrelExit[0] = mtx.axis[0][0] * dist + mtx.origin[0];
            barrelExit[1] = mtx.axis[0][1] * dist + mtx.origin[1];
            barrelExit[2] = mtx.axis[0][2] * dist + mtx.origin[2];
            float playerCenter[3];
            playerCenter[0] = player->r.currentOrigin.v.m128_f32[0];
            playerCenter[1] = player->r.currentOrigin.v.m128_f32[1];
            playerCenter[2] = player->r.currentOrigin.v.m128_f32[2] + 16.0f;
            if (VectorDistance(barrelExit, playerCenter) < 24.0f)
                player->client->mVehicleAnimGetOut = true;
        }
    }
    client = player->client;
    if (client->mVehicleAnimGetOut && info->type == 2
        && client->mVehicleAnimStage <= 4)
    {
        if (player->IsLocalPlayer())
        {
            MultiplayerMgr::sInst->GetOutOfVehicle(
                ent, player->client->ps.vehPos);
        }
        G_EntUnlink(player);
        player->flags &= ~0x1000000;
        client = player->client;
        if (client->ps.ctf_has_flag == 0)
            client->mVehicleNoWeaponTime = level.time + 300;
    }
}

// ea: 0x00490080
void VEH_FireGunnerWeapon(Entity* ent, int msec)
{
    scr_vehicle_t* veh = ent->scr_vehicle;
    Entity* gunner = HandleDbToEnt(veh->seats[1].occupant);
    if (!veh->seats[1].gunMounted)
        return;
    weaponFileInfo_t* info = BG_GetInfoForWeapon(veh->gunnerWeapon);
    veh->gunnerFireTime = info->iFireTime;
    weaponParms wp;
    memset(&wp, 0, sizeof(wp));
    wp.pWeapInfo = info;
    if (veh->boneIndex.gunner_flash < 0)
    {
        gpBrocAPI->mBrocExports.mFireTurret(ent->mHandle.mHandle.mVal, true);
        if (wp.pWeapInfo->type != WEAPTYPE_BULLET)
            return;
        veh->seats[1].heat += (msec * info->fFireHeat) * 0.001f;
        return;
    }
    DObjSkelMat flashMtx;
    G_DObjGetWorldBoneIndexMatrix(ent, veh->boneIndex.gunner_flash,
                                  &flashMtx);
    wp.right[0] = flashMtx.axis[1][0];
    wp.right[1] = flashMtx.axis[1][1];
    wp.right[2] = flashMtx.axis[1][2];
    wp.up[0] = flashMtx.axis[2][0];
    wp.up[1] = flashMtx.axis[2][1];
    wp.up[2] = flashMtx.axis[2][2];
    wp.gunForward[0] = flashMtx.axis[0][0];
    wp.gunForward[1] = flashMtx.axis[0][1];
    wp.gunForward[2] = flashMtx.axis[0][2];
    wp.forward[0] = flashMtx.axis[0][0];
    wp.forward[1] = flashMtx.axis[0][1];
    wp.forward[2] = flashMtx.axis[0][2];
    if (veh->barrelBlocked != 0)
    {
        wp.muzzleTrace[0] = (0.0f - veh->barrelOffset) * flashMtx.axis[0][0]
                          + flashMtx.origin[0];
        wp.muzzleTrace[1] = (0.0f - veh->barrelOffset) * flashMtx.axis[0][1]
                          + flashMtx.origin[1];
        wp.muzzleTrace[2] = (0.0f - veh->barrelOffset) * flashMtx.axis[0][2]
                          + flashMtx.origin[2];
    }
    else
    {
        wp.muzzleTrace[0] = flashMtx.origin[0];
        wp.muzzleTrace[1] = flashMtx.origin[1];
        wp.muzzleTrace[2] = flashMtx.origin[2];
    }
    if (wp.pWeapInfo->type == WEAPTYPE_BULLET)
    {
        float spread = (100.0f - info->accuracy) * 0.1f;
        Entity* attacker = gunner != nullptr ? gunner : ent;
        Entity* attacker2 = gunner != nullptr ? gunner : ent;
        int damage = info->iDamage;
        if (EntityManager::sInst->IsLocalPlayer(attacker2))
        {
            Bullet_Fire(gunner != nullptr ? gunner : ent, spread, damage, &wp,
                        ent, 0.0f);
        }
        else
        {
            Bullet_Fire_Fake(gunner != nullptr ? gunner : ent, 0.0f, damage,
                             &wp, ent, 0.0f);
        }
        CG_FireWeapon(attacker, &attacker->s, 187, 0);
        for (int i = 0; i < 11; ++i)
        {
            Entity* occupant = HandleDbToEnt(veh->seats[i].occupant);
            if (occupant != nullptr
                && EntityManager::sInst->IsLocalPlayer(occupant))
            {
                int client = occupant->client->mServerClientIndex;
                if (RumbleManager::Inst(client) != nullptr)
                {
                    RumbleEffect effect;
                    effect.mRumbleDataArray[0].enabled = true;
                    effect.mRumbleDataArray[0].delay = 0.0f;
                    effect.mRumbleDataArray[0].steady_duration = 0.5f;
                    effect.mRumbleDataArray[1].enabled = true;
                    effect.mRumbleDataArray[1].delay = 0.0f;
                    effect.mRumbleDataArray[1].steady_duration = 0.2f;
                    effect.mRumbleDataArray[1].ramp_up_duration = 0.0f;
                    effect.mRumbleDataArray[1].ramp_down_duration = 0.0f;
                    RumbleEffect_SetIntensity(&effect, kRumbleLEFT, 0.5f);
                    float playIntensity;
                    if (i == 1)
                    {
                        RumbleEffect_SetIntensity(&effect, kRumbleRIGHT,
                                                  0.5f);
                        playIntensity = 1.0f;
                    }
                    else
                    {
                        RumbleEffect_SetIntensity(&effect, kRumbleRIGHT,
                                                  0.2f);
                        playIntensity = 0.5f;
                    }
                    RumbleManager::Inst(client)->Play(effect, playIntensity);
                }
            }
        }
        veh->seats[1].heat += (info->iFireTime * info->fFireHeat)
                              * 0.00075000001f;
        return;
    }
    Weapon_RocketLauncher_Fire(ent, 0.0f, &wp, 10.0f, true);
}

// ea: 0x0047E990
void VEH_UpdateControllers(Entity* entity, int msec)
{
    scr_vehicle_t* veh = entity->scr_vehicle;
    vehicle_info_t* info = veh != nullptr ? s_vehicleInfos[veh->infoIdx]
                                          : nullptr;
    if (entity->mDObj == nullptr || entity->mDObj->skel == nullptr)
        return;
    float turretShake[3] = { 0.0f, 0.0f, 0.0f };
    float turretAngles[3] = { 0.0f, 0.0f, 0.0f };
    float barrelAngles[3] = { 0.0f, 0.0f, 0.0f };
    float gunnerAngles[3] = { 0.0f, 0.0f, 0.0f };
    float gunnerBarrelAngles[3] = { 0.0f, 0.0f, 0.0f };
    float steerAngles[3] = { 0.0f, 0.0f, 0.0f };
    float hatchRightAngles[2] = { 0.0f, 0.0f };
    float hatchLeftAngles[2] = { 0.0f, 0.0f };
    float offset = 0.0f;
    float rotation = 0.0f;
    if (veh != nullptr)
    {
        barrelAngles[0] = veh->current.mTurretAngles.v.m128_f32[0];
        gunnerBarrelAngles[0] = veh->current.mGunnerAngles.v.m128_f32[0];
        rotation = veh->current.mSteeringAngle;
    }
    static unsigned int sS124 = 0;
    static unsigned int tag_body_hash_0 = 0;
    static unsigned int tag_turret_hash_0 = 0;
    static unsigned int tag_barrel_hash_0 = 0;
    static unsigned int tag_gunner_turret_hash = 0;
    static unsigned int tag_gunner_barrel_hash_0 = 0;
    static unsigned int tag_hatch_right_hash = 0;
    static unsigned int tag_hatch_left_hash = 0;
    static unsigned int tag_recoil = 0;
    if ((sS124 & 1) == 0)
    {
        sS124 |= 1u;
        tag_body_hash_0 = HashString::CalcHash("tag_body");
    }
    if ((sS124 & 2) == 0)
    {
        sS124 |= 2u;
        tag_turret_hash_0 = HashString::CalcHash("tag_turret");
    }
    if ((sS124 & 4) == 0)
    {
        sS124 |= 4u;
        tag_barrel_hash_0 = HashString::CalcHash("tag_barrel");
    }
    if ((sS124 & 8) == 0)
    {
        sS124 |= 8u;
        tag_gunner_turret_hash = HashString::CalcHash("tag_gunner_turret");
    }
    if ((sS124 & 0x10) == 0)
    {
        sS124 |= 0x10u;
        tag_gunner_barrel_hash_0 =
            HashString::CalcHash("tag_gunner_barrel");
    }
    if ((sS124 & 0x20) == 0)
    {
        sS124 |= 0x20u;
        tag_hatch_right_hash = HashString::CalcHash("tag_hatch_right");
    }
    if ((sS124 & 0x40) == 0)
    {
        sS124 |= 0x40u;
        tag_hatch_left_hash = HashString::CalcHash("tag_hatch_left");
    }
    if ((sS124 & 0x80) == 0)
    {
        sS124 |= 0x80u;
        tag_recoil = HashString::CalcHash("tag_recoil");
    }
    if (info != nullptr && info->type != 2 && info->type != 5)
    {
        if (veh->fireTime > 0)
            turretShake[0] = flrand(-0.5f, 0.5f);
        if (veh->gunnerFireTime > 0)
            offset = flrand(-0.5f, 0.5f);
    }
    if (entity->s.weapon != 0)
    {
        int bone = SV_DObjGetBoneIndex(entity, tag_turret_hash_0);
        if (bone >= 0)
            G_DObjSetLocalTagInternal_0(nullptr, turretAngles, bone,
                                        entity, 0);
        bone = SV_DObjGetBoneIndex(entity, tag_barrel_hash_0);
        if (bone >= 0)
            G_DObjSetLocalTagInternal_0(turretShake, barrelAngles, bone,
                                        entity, 0);
    }
    if (veh != nullptr && veh->gunnerWeapon != 0)
    {
        int bone = SV_DObjGetBoneIndex(entity, tag_gunner_turret_hash);
        if (bone >= 0)
            G_DObjSetLocalTagInternal_0(nullptr, gunnerAngles, bone,
                                        entity, 0);
        bone = SV_DObjGetBoneIndex(entity, tag_gunner_barrel_hash_0);
        if (bone >= 0)
            G_DObjSetLocalTagInternal_0(&offset, gunnerBarrelAngles, bone,
                                        entity, 0);
    }
    if (veh != nullptr && veh->joltTime > 0.0f)
    {
        float scale;
        if (veh->joltTime < 0.65f)
            scale = veh->joltTime * 1.5384616f;
        else
            scale = 1.0f - ((veh->joltTime - 0.65f) * 6.6666651f);
        offset = scale * -24.0f;
        int bone = SV_DObjGetBoneIndex(entity, tag_recoil);
        if (bone >= 0)
            G_DObjSetLocalTagInternal_0(&offset, nullptr, bone, entity, 1);
    }
    int steerBone = veh != nullptr ? veh->boneIndex.steering_wheel : -1;
    if (steerBone > 0)
    {
        Entity* owner = HandleDbToEnt(entity->r.mOwner);
        if (owner != nullptr && owner->IsLocalPlayer())
        {
            int playerIndex = owner->GetPlayerIndex();
            void* ic = InteractionController::Inst(playerIndex);
            if (ic != nullptr && (*(unsigned char*)ic & 0x20) != 0
                && *(int*)((char*)gCamera + playerIndex * 0x1F0 + 0x190)
                       == 2 /* CAM_VEHICLE_FIRST */)
            {
                rotation = InteractionController_GetRotation(
                    InteractionController::Inst(playerIndex));
            }
        }
        G_DObjSetLocalTagInternal_0(vec3_origin, steerAngles, steerBone,
                                    entity, 0);
    }
    int bone = SV_DObjGetBoneIndex(entity, tag_hatch_right_hash);
    if (bone >= 0)
        G_DObjSetLocalTagInternal_0(nullptr, hatchRightAngles, bone,
                                    entity, 0);
    bone = SV_DObjGetBoneIndex(entity, tag_hatch_left_hash);
    if (bone >= 0)
        G_DObjSetLocalTagInternal_0(nullptr, hatchLeftAngles, bone,
                                    entity, 0);
}

// ea: 0x0047FFB0
void VEH_UnlinkPlayer(Entity* player, bool setOrigin)
{
    Client* client = player->client;
    if (client == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 6149;
        AeAssert::gCurrentExpr = "client";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if ((client->ps.eFlags & 0x100000) == 0)
        return;
    Entity* vehEnt = HandleDbToEnt(player->r.mOwner);
    if (vehEnt == nullptr)
    {
        client->ps.eFlags &= ~0x100000;
        client->mVehicleAnimStage = -1;
        return;
    }
    scr_vehicle_t* veh = vehEnt->scr_vehicle;
    if (veh == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 6176;
        AeAssert::gCurrentExpr = "ent->scr_vehicle";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    vehicle_info_t* info = s_vehicleInfos[veh->infoIdx];
    player->flags &= ~0x1000000;
    G_EntUnlink(player);
    DObjSkelMat detachMtx;
    bool haveDetach = false;
    if (veh->boneIndex.detach >= 0)
    {
        G_DObjGetWorldBoneIndexMatrix(vehEnt, veh->boneIndex.detach,
                                      &detachMtx);
        haveDetach = true;
    }
    int seatIdx = client->mVehicleAnimStage;
    if (seatIdx >= 0 && seatIdx < 11)
    {
        veh->seats[seatIdx].occupant.mHandle.mVal = 0;
        veh->seats[seatIdx].flags = 0;
    }
    Entity* owner = HandleDbToEnt(vehEnt->r.mOwner);
    if (owner == player)
    {
        vehEnt->r.mOwner.mHandle.mVal = 0;
        for (int i = 0; i < 11; ++i)
        {
            Entity* occ = HandleDbToEnt(veh->seats[i].occupant);
            if (occ != nullptr)
                vehEnt->r.mOwner = veh->seats[i].occupant;
        }
    }
    if (vehEnt->r.mOwner.mHandle.mVal == 0 && veh->playersAttached == 0)
    {
        vehEnt->active = 0;
        vehEnt->s.eFlags &= ~0x100000;
        vehEnt->r.mOwner.mHandle.mVal = 0;
    }
    float origin[3];
    bool bWasGunner = false;
    bool bWasLocalPlayer = false;
    if (client->mVehicleAnimStage >= 8 || client->mVehicleAnimGetOut)
    {
        bWasGunner = true;
        if (client->mVehicleAnimStage == 2)
            bWasLocalPlayer = true;
        origin[0] = player->r.currentOrigin.v.m128_f32[0];
        origin[1] = player->r.currentOrigin.v.m128_f32[1];
        origin[2] = player->r.currentOrigin.v.m128_f32[2];
        if (!VEH_FindValidDismountSpot(
                vehEnt, &player->r.mins.v.m128_f32[0],
                &player->r.maxs.v.m128_f32[0], origin, player, true))
        {
            origin[0] = veh->phys.origin.v.m128_f32[0];
            origin[1] = veh->phys.origin.v.m128_f32[1];
            origin[2] = veh->phys.origin.v.m128_f32[2] + 80.0f;
        }
    }
    else
    {
        if (!VEH_FindValidDismountSpot(
                vehEnt, &player->r.mins.v.m128_f32[0],
                &player->r.maxs.v.m128_f32[0], origin, player, false))
        {
            origin[0] = veh->phys.origin.v.m128_f32[0];
            origin[1] = veh->phys.origin.v.m128_f32[1];
            origin[2] = veh->phys.origin.v.m128_f32[2] + 80.0f;
        }
    }
    player->r.mOwner.mHandle.mVal = 0;
    if (EntityManager::sInst->IsLocalPlayer(player))
        cl_aADS[player->GetPlayerIndex()] = 1;
    client = player->client;
    client->ps.eFlags &= 0xFF8FFFFF;
    *(int*)((char*)client + 0x4A0) = 0;
    client->mVehicleAnimStage = -1;
    client->mVehicleAnimStageAnim = 0;
    client->mVehicleAnimStageAnimPlayed = 0;
    if (EntityManager::sInst->IsLocalPlayer(player) && setOrigin)
    {
        float dirTo[3];
        float fwd[3];
        if (bWasGunner)
        {
            dirTo[0] = origin[0] - vehEnt->r.currentOrigin.v.m128_f32[0];
            dirTo[1] = origin[1] - vehEnt->r.currentOrigin.v.m128_f32[1];
            dirTo[2] = 0.0f;
        }
        else
        {
            dirTo[0] = origin[0] - vehEnt->r.currentOrigin.v.m128_f32[0];
            dirTo[1] = origin[1] - vehEnt->r.currentOrigin.v.m128_f32[1];
            dirTo[2] = 0.0f;
        }
        VectorNormalize(dirTo);
        int mask = player->clipmask & 0xFDFFFFFF;
        collision_context_t ctx;

        ctx.pass_entity1.mHandle.mVal = 0;
        ctx.pass_entity2.mHandle.mVal =
            vehEnt->s.eType == 14 ? 0x200051 : 0x200011;
        ctx.pass_owner1.mHandle.mVal = 0;
        ctx.pass_owner2.mHandle.mVal = 0;
        ctx.contentmask = mask;
        int steps = bWasGunner ? 30 : 15;
        float stepSize = bWasGunner ? 4.0f : 16.0f;
        for (int i = 0; i < steps; ++i)
        {
            math::Position3 start;
            start.v.m128_f32[0] = origin[0];
            start.v.m128_f32[1] = origin[1];
            start.v.m128_f32[2] = origin[2] - 32.0f;
            start.v.m128_f32[3] = 0.0f;
            math::Position3 end;
            end.v.m128_f32[0] = origin[0];
            end.v.m128_f32[1] = origin[1];
            end.v.m128_f32[2] = origin[2];
            end.v.m128_f32[3] = 0.0f;
            math::Position3 zero;
            zero.v = _mm_setzero_ps();
            trace_t tr;
            memset(&tr, 0, sizeof(tr));
            SV_Trace(&tr, &start, &zero, &zero, &end, &ctx, 1, 0, nullptr,
                     0, 0.0f);
            if (tr.fraction == 0.0f)
                break;
            origin[0] += dirTo[0] * stepSize;
            origin[1] += dirTo[1] * stepSize;
            origin[2] += stepSize * 0.5f + 2.0f;
        }
    }
    G_DObjUpdate(player, false);
    if (EntityManager::sInst->IsLocalPlayer(player))
        SetClientOrigin(player, origin);
    if (info->type == 2)
        g_femanager.IGO->SetHUDType((hud_type)0, currCl);
    if (bWasLocalPlayer && EntityManager::sInst->IsLocalPlayer(player))
    {
        float fwd[3];
        YawVectors(client->ps.viewangles[1], fwd, nullptr);
        fwd[2] = 0.0f;
        VectorNormalize(fwd);
        float vel[3];
        vel[0] = fwd[0] * 320.0f;
        vel[1] = fwd[1] * 320.0f;
        vel[2] = sqrtf((float)client->ps.gravity * 124.8f);
        float norm[3] = { vel[0], vel[1], vel[2] };
        VectorNormalize(norm);
        float angles[3];
        vectoangles(norm, angles);
        angles[0] = 0.0f;
        client->ps.pm_flags = (client->ps.pm_flags & 0xBFFFDEFF) | 0x2000200;
        client->ps.pm_time = 4000;
        MultiplayerMgr::sInst->AnimEvent(9);
    }
    if (bWasGunner)
    {
        float camAngles[3];
        camAngles[0] = gCamera[currCl].mPrevAngles.v.m128_f32[0];
        camAngles[1] = gCamera[currCl].mPrevAngles.v.m128_f32[1];
        camAngles[2] = gCamera[currCl].mPrevAngles.v.m128_f32[2];
        SetClientViewAngle(player, camAngles);
    }
    Cvar_Set("cl_stance", "0");
    if (player->IsLocalPlayer())
        cl_stance_ss[player->GetPlayerIndex()] = 0;
    Scr_Notify(vehEnt, hash_const.player_off_vehicle, 0);
}

// ea: 0x00488420
void ChiefMammalInChargeOfVehicleDamageAndPushOut(Entity* pSelf)
{
    scr_vehicle_t* veh = pSelf->scr_vehicle;
    vehicle_info_t* info = s_vehicleInfos[veh->infoIdx];
    if (info->collisionDamage <= 0.0f)
        return;
    float velSq = veh->phys.vel.v.m128_f32[0] * veh->phys.vel.v.m128_f32[0]
                + veh->phys.vel.v.m128_f32[1] * veh->phys.vel.v.m128_f32[1]
                + veh->phys.vel.v.m128_f32[2] * veh->phys.vel.v.m128_f32[2];
    if (sqrtf(velSq) < 1.0f)
        return;
    static bool sS128 = false;
    static TouchEntityData entities;
    if (!sS128)
    {
        sS128 = true;
        memset(&entities, 0, sizeof(entities));
    }
    proximity_data_t proximity;
    memset(&proximity, 0, sizeof(proximity));
    prepare_collision_objects(
        pSelf, veh->phys.origin, veh->phys.origin, 200.0f,
        pSelf->clipmask | 0x2000000, proximity, entities);
    math::Mat43 rot = pSelf->CalcRotTranMat43();
    math::Position3 lo = rot.w;
    // bmodel bounds in the tree's DCGSet layout: min/max at +0x30/+0x40
    const math::Position3* bmin =
        (const math::Position3*)((const char*)pSelf->r.bmodel + 0x30);
    const math::Position3* bmax =
        (const math::Position3*)((const char*)pSelf->r.bmodel + 0x40);
    for (int i = 0; i < entities.num; ++i)
    {
        Entity* ent = HandleDbToEnt(entities.touch[i]);
        if (ent == nullptr)
            continue;
        if (ent->s.eType != 1 && ent->s.eType != 11)
            continue;
        if (ent->tagInfo != nullptr)
            continue;
        math::Position3 origin = ent->r.currentOrigin;
        // transform into vehicle local space
        __m128 local =
            _mm_add_ps(
                _mm_add_ps(
                    _mm_mul_ps(_mm_shuffle_ps(origin.v, origin.v, 0),
                               rot.x.v),
                    _mm_mul_ps(_mm_shuffle_ps(origin.v, origin.v, 85),
                               rot.y.v)),
                _mm_add_ps(
                    _mm_mul_ps(_mm_shuffle_ps(origin.v, origin.v, 170),
                               rot.z.v),
                    _mm_xor_ps(rot.w.v, sSignMask)));
        __m128 clamped = _mm_min_ps(_mm_max_ps(local, bmin->v), bmax->v);
        __m128 delta = _mm_sub_ps(local, clamped);
        __m128 d2 = _mm_mul_ps(delta, delta);
        __m128 sum =
            _mm_add_ps(d2,
                       _mm_add_ps(_mm_shuffle_ps(d2, d2, 85),
                                  _mm_shuffle_ps(d2, d2, 170)));
        float dist2 = sum.m128_f32[0];
        if (radius_0 * radius_0 <= dist2 || dist2 <= 0.001f)
            continue;
        float pushDir[3];
        pushDir[0] = veh->phys.origin.v.m128_f32[0]
                   - veh->phys.prevOrigin.v.m128_f32[0];
        pushDir[1] = veh->phys.origin.v.m128_f32[1]
                   - veh->phys.prevOrigin.v.m128_f32[1];
        pushDir[2] = veh->phys.origin.v.m128_f32[2]
                   - veh->phys.prevOrigin.v.m128_f32[2];
        math::Dir3 moveDir;
        if (VectorNormalize2(pushDir, &moveDir.v.m128_f32[0]) <= 0.005f)
            continue;
        float ddx = ent->r.currentOrigin.v.m128_f32[0]
                  - pSelf->r.currentOrigin.v.m128_f32[0];
        float ddy = ent->r.currentOrigin.v.m128_f32[1]
                  - pSelf->r.currentOrigin.v.m128_f32[1];
        float ddz = ent->r.currentOrigin.v.m128_f32[2]
                  - pSelf->r.currentOrigin.v.m128_f32[2];
        float dist = sqrtf(ddx * ddx + ddy * ddy + ddz * ddz);
        float dirTo[3];
        if (dist <= 0.01f)
        {
            dirTo[0] = 0.0f;
            dirTo[1] = 0.0f;
            dirTo[2] = 1.0f;
        }
        else
        {
            dirTo[0] = ddx / dist;
            dirTo[1] = ddy / dist;
            dirTo[2] = ddz / dist;
        }
        float speedFrac = pSelf->speed / info->collisionSpeed;
        if (speedFrac > 1.0f)
            speedFrac = 1.0f;
        float dot = moveDir.v.m128_f32[0] * dirTo[0]
                  + moveDir.v.m128_f32[1] * dirTo[1]
                  + moveDir.v.m128_f32[2] * dirTo[2];
        if (dot > 0.8f)
        {
            int damage = (int)(((dot - 0.8f) * speedFrac * 5.0000005f)
                               * 120.0f);
            G_Damage(ent, pSelf, pSelf, dirTo,
                     &ent->r.currentOrigin.v.m128_f32[0], damage, 0, 20,
                     HITLOC_NONE, -1);
        }
        int pushes = 0;
        while (push_in_world(ent->r.currentOrigin, radius_0, proximity,
                             entities))
        {
            if (++pushes > 4)
                break;
        }
        MultiplayerMgr::sInst->SetPlayerPos(
            ent, &ent->r.currentOrigin.v.m128_f32[0]);
    }
}

// ea: 0x0046AAC0
int VEH_SlideMove(Entity* ent, int gravity, int msec)
{
    scr_vehicle_t* veh = ent->scr_vehicle;
    math::Position3& origin = veh->phys.origin;
    vehicle_info_t* info = s_vehicleInfos[veh->infoIdx];
    trace_t trace;
    memset(&trace, 0, sizeof(trace));
    int bumpCount = 0;
    float timeScale = msec * 0.001f;
    math::Dir3 endVel;
    endVel.v.m128_f32[3] = timeScale;
    float origVel[3];
    if (gravity != 0)
    {
        origVel[0] = veh->phys.vel.v.m128_f32[0];
        origVel[1] = veh->phys.vel.v.m128_f32[1];
        origVel[2] = veh->phys.vel.v.m128_f32[2]
                   - timeScale * 800.0f;
        float avgZ = (origVel[2] + veh->phys.vel.v.m128_f32[2]) * 0.5f;
        veh->phys.vel.v.m128_f32[2] = avgZ;
        if (s_phys.hasGround)
        {
            float dot =
                veh->phys.vel.v.m128_f32[0]
                    * s_phys.groundTrace.normal.v.m128_f32[0]
                + veh->phys.vel.v.m128_f32[1]
                      * s_phys.groundTrace.normal.v.m128_f32[1]
                + avgZ * s_phys.groundTrace.normal.v.m128_f32[2];
            float scale = dot >= 0.0f ? dot * 0.99009901f : dot * 1.01f;
            veh->phys.vel.v.m128_f32[0] -=
                scale * s_phys.groundTrace.normal.v.m128_f32[0];
            veh->phys.vel.v.m128_f32[1] -=
                scale * s_phys.groundTrace.normal.v.m128_f32[1];
            veh->phys.vel.v.m128_f32[2] -=
                scale * s_phys.groundTrace.normal.v.m128_f32[2];
        }
    }
    math::Dir3 planes[5];
    memset(planes, 0, sizeof(planes));
    int numPlanes = 0;
    if (s_phys.hasGround)
    {
        planes[0] = s_phys.groundTrace.normal;
        numPlanes = 1;
    }
    VectorNormalize2(veh->phys.vel, veh->phys.vel);
    collision_context_t context;

    context.pass_entity1.mHandle.mVal = ent->mHandle.mHandle.mVal;
    context.pass_entity2.mHandle.mVal = ent->clipmask;
    context.pass_owner1.mHandle.mVal = 0;
    context.pass_owner2.mHandle.mVal = 0;
    context.contentmask = 0;
    float timeLeft = 0.0f;
    float clipVel[3];
    int i = 0;
    for (;;)
    {
        math::Position3 end;
        end.v.m128_f32[0] =
            origin.v.m128_f32[0] + veh->phys.vel.v.m128_f32[0] * timeScale;
        end.v.m128_f32[1] =
            origin.v.m128_f32[1] + veh->phys.vel.v.m128_f32[1] * timeScale;
        end.v.m128_f32[2] =
            origin.v.m128_f32[2] + veh->phys.vel.v.m128_f32[2] * timeScale;
        SV_Trace(&trace, &origin, &info->mins, &info->maxs, &end, &context,
                 1, 0, nullptr, 0, 0.0f);
        if (trace.allsolid)
        {
            veh->phys.vel.v.m128_f32[0] = 0.0f;
            veh->phys.vel.v.m128_f32[1] = 0.0f;
            veh->phys.vel.v.m128_f32[2] = 0.0f;
            return 1;
        }
        float normalY = trace.normal.v.m128_f32[1];
        if (normalY > 0.0f)
        {
            origin.v.m128_f32[0] = trace.endpos.v.m128_f32[0];
            origin.v.m128_f32[1] = trace.endpos.v.m128_f32[1];
            origin.v.m128_f32[2] = trace.endpos.v.m128_f32[2];
            normalY = trace.normal.v.m128_f32[1];
        }
        if (normalY == 1.0f)
        {
            if (gravity != 0)
            {
                veh->phys.vel.v.m128_f32[0] = origVel[0];
                veh->phys.vel.v.m128_f32[1] = origVel[1];
                veh->phys.vel.v.m128_f32[2] = origVel[2];
            }
            return bumpCount == 0 && i != 0;
        }
        Entity* hitEnt = HandleDbToEnt(*(DbLinkedHandle<EntityHandleDb, Entity>*)
                                           &trace.surfaceFlags);
        if (hitEnt != nullptr
            && (hitEnt->s.eType == 14 || hitEnt->s.eType == 1))
            bumpCount = 1;
        timeScale -= trace.fraction * timeScale;
        if (numPlanes >= 5)
        {
            veh->phys.vel.v.m128_f32[0] = 0.0f;
            veh->phys.vel.v.m128_f32[1] = 0.0f;
            veh->phys.vel.v.m128_f32[2] = 0.0f;
            return bumpCount == 0;
        }
        bool planeMatch = false;
        for (int p = 0; p < numPlanes; ++p)
        {
            float dot = planes[p].v.m128_f32[0]
                            * trace.normal.v.m128_f32[0]
                        + planes[p].v.m128_f32[1]
                              * trace.normal.v.m128_f32[1]
                        + planes[p].v.m128_f32[2]
                              * trace.normal.v.m128_f32[2];
            if (dot > 0.99f)
            {
                veh->phys.vel.v.m128_f32[0] += trace.normal.v.m128_f32[0];
                veh->phys.vel.v.m128_f32[1] += trace.normal.v.m128_f32[1];
                veh->phys.vel.v.m128_f32[2] += trace.normal.v.m128_f32[2];
                planeMatch = true;
                break;
            }
        }
        if (planeMatch)
        {
            ++i;
            if (i >= 4)
                break;
            continue;
        }
        planes[numPlanes] = trace.normal;
        ++numPlanes;
        int clipIdx = -1;
        for (int p = 0; p < numPlanes; ++p)
        {
            float dot = planes[p].v.m128_f32[0]
                            * veh->phys.vel.v.m128_f32[0]
                        + planes[p].v.m128_f32[1]
                              * veh->phys.vel.v.m128_f32[1]
                        + planes[p].v.m128_f32[2]
                              * veh->phys.vel.v.m128_f32[2];
            if (dot < 0.1f)
            {
                clipIdx = p;
                break;
            }
        }
        if (clipIdx < 0)
        {
            ++i;
            if (i >= 4)
                break;
            continue;
        }
        math::Dir3& plane = planes[clipIdx];
        float vdot = plane.v.m128_f32[0] * veh->phys.vel.v.m128_f32[0]
                   + plane.v.m128_f32[1] * veh->phys.vel.v.m128_f32[1]
                   + plane.v.m128_f32[2] * veh->phys.vel.v.m128_f32[2];
        float vscale = vdot >= 0.0f ? vdot * 0.99009901f : vdot * 1.01f;
        clipVel[0] = veh->phys.vel.v.m128_f32[0]
                   - plane.v.m128_f32[0] * vscale;
        clipVel[1] = veh->phys.vel.v.m128_f32[1]
                   - plane.v.m128_f32[1] * vscale;
        clipVel[2] = veh->phys.vel.v.m128_f32[2]
                   - plane.v.m128_f32[2] * vscale;
        float odot = plane.v.m128_f32[0] * origVel[0]
                   + plane.v.m128_f32[1] * origVel[1]
                   + plane.v.m128_f32[2] * origVel[2];
        float oscale = odot >= 0.0f ? odot * 0.99009901f : odot * 1.01f;
        float endClipVel[3] = {
            origVel[0] - plane.v.m128_f32[0] * oscale,
            origVel[1] - plane.v.m128_f32[1] * oscale,
            origVel[2] - plane.v.m128_f32[2] * oscale,
        };
        veh->phys.vel.v.m128_f32[0] = clipVel[0];
        veh->phys.vel.v.m128_f32[1] = clipVel[1];
        veh->phys.vel.v.m128_f32[2] = clipVel[2];
        origVel[0] = endClipVel[0];
        origVel[1] = endClipVel[1];
        origVel[2] = endClipVel[2];
        for (int p = 0; p < numPlanes; ++p)
        {
            if (p == clipIdx)
                continue;
            float d2 = planes[p].v.m128_f32[0] * clipVel[0]
                     + planes[p].v.m128_f32[1] * clipVel[1]
                     + planes[p].v.m128_f32[2] * clipVel[2];
            if (d2 >= 0.1f)
                continue;
            float s2 = d2 >= 0.0f ? d2 * 0.99009901f : d2 * 1.01f;
            clipVel[0] -= planes[p].v.m128_f32[0] * s2;
            clipVel[1] -= planes[p].v.m128_f32[1] * s2;
            clipVel[2] -= planes[p].v.m128_f32[2] * s2;
            float e2 = planes[p].v.m128_f32[0] * endClipVel[0]
                     + planes[p].v.m128_f32[1] * endClipVel[1]
                     + planes[p].v.m128_f32[2] * endClipVel[2];
            float s3 = e2 >= 0.0f ? e2 * 0.99009901f : e2 * 1.01f;
            endClipVel[0] -= planes[p].v.m128_f32[0] * s3;
            endClipVel[1] -= planes[p].v.m128_f32[1] * s3;
            endClipVel[2] -= planes[p].v.m128_f32[2] * s3;
            float edgeDot = clipVel[0] * plane.v.m128_f32[0]
                          + clipVel[1] * plane.v.m128_f32[1]
                          + clipVel[2] * plane.v.m128_f32[2];
            if (edgeDot >= 0.0f)
                continue;
            // slide along the edge (cross product of the two normals)
            float edge[3];
            edge[0] = plane.v.m128_f32[1] * planes[p].v.m128_f32[2]
                    - plane.v.m128_f32[2] * planes[p].v.m128_f32[1];
            edge[1] = plane.v.m128_f32[2] * planes[p].v.m128_f32[0]
                    - plane.v.m128_f32[0] * planes[p].v.m128_f32[2];
            edge[2] = plane.v.m128_f32[0] * planes[p].v.m128_f32[1]
                    - plane.v.m128_f32[1] * planes[p].v.m128_f32[0];
            VectorNormalize(edge);
            float ev = edge[0] * veh->phys.vel.v.m128_f32[0]
                     + edge[1] * veh->phys.vel.v.m128_f32[1]
                     + edge[2] * veh->phys.vel.v.m128_f32[2];
            float eo = edge[0] * origVel[0] + edge[1] * origVel[1]
                     + edge[2] * origVel[2];
            veh->phys.vel.v.m128_f32[0] = edge[0] * ev;
            veh->phys.vel.v.m128_f32[1] = edge[1] * ev;
            veh->phys.vel.v.m128_f32[2] = edge[2] * ev;
            origVel[0] = edge[0] * eo;
            origVel[1] = edge[1] * eo;
            origVel[2] = edge[2] * eo;
            clipVel[0] = veh->phys.vel.v.m128_f32[0];
            clipVel[1] = veh->phys.vel.v.m128_f32[1];
            clipVel[2] = veh->phys.vel.v.m128_f32[2];
            endClipVel[0] = origVel[0];
            endClipVel[1] = origVel[1];
            endClipVel[2] = origVel[2];
            break;
        }
        veh->phys.vel.v.m128_f32[0] = clipVel[0];
        veh->phys.vel.v.m128_f32[1] = clipVel[1];
        veh->phys.vel.v.m128_f32[2] = clipVel[2];
        origVel[0] = endClipVel[0];
        origVel[1] = endClipVel[1];
        origVel[2] = endClipVel[2];
        ++i;
        if (i >= 4)
            break;
    }
    if (gravity != 0)
    {
        veh->phys.vel.v.m128_f32[0] = origVel[0];
        veh->phys.vel.v.m128_f32[1] = origVel[1];
        veh->phys.vel.v.m128_f32[2] = origVel[2];
    }
    return bumpCount == 0 && i != 0;
}

// ea: 0x0046C240
int VEH_Slide(Entity* ent, int gravity, int msec, int move, int allowHit)
{
    scr_vehicle_t* veh = ent->scr_vehicle;
    vehicle_info_t* info = s_vehicleInfos[veh->infoIdx];
    float timeScale = msec * 0.001f;
    float target[3];
    if (move != 0)
    {
        target[0] = veh->phys.origin.v.m128_f32[0]
                  + veh->phys.vel.v.m128_f32[0] * timeScale;
        target[1] = veh->phys.origin.v.m128_f32[1]
                  + veh->phys.vel.v.m128_f32[1] * timeScale;
        target[2] = veh->phys.origin.v.m128_f32[2]
                  + veh->phys.vel.v.m128_f32[2] * timeScale;
    }
    else
    {
        target[0] = veh->phys.origin.v.m128_f32[0];
        target[1] = veh->phys.origin.v.m128_f32[1];
        target[2] = veh->phys.origin.v.m128_f32[2];
    }
    static bool sS123 = false;
    static TouchEntityData entities;
    if (!sS123)
    {
        sS123 = true;
        memset(&entities, 0, sizeof(entities));
    }
    proximity_data_t proximity;
    memset(&proximity, 0, sizeof(proximity));
    prepare_collision_objects(ent, veh->phys.origin, veh->phys.origin,
                              200.0f, ent->clipmask, proximity, entities);
    DObjSkelMat bodyMtx;
    G_DObjGetWorldBoneIndexMatrix(ent, veh->boneIndex.body, &bodyMtx);
    float dir[3] = { bodyMtx.axis[0][0], bodyMtx.axis[0][1],
                     bodyMtx.axis[0][2] };
    float up[3] = { bodyMtx.axis[2][0], bodyMtx.axis[2][1],
                    bodyMtx.axis[2][2] };
    float right[3] = { bodyMtx.axis[1][0], bodyMtx.axis[1][1],
                       bodyMtx.axis[1][2] };
    float fwd[3] = { dir[0] * udelta + up[0] * fdelta,
                     dir[1] * udelta + up[1] * fdelta,
                     dir[2] * udelta + up[2] * fdelta };
    float bwd[3] = { dir[0] * -fdelta + up[0] * udelta,
                     dir[1] * -fdelta + up[1] * udelta,
                     dir[2] * -fdelta + up[2] * udelta };
    float hLen = (info->boundsLength * 0.5f) - info->boundsRadius;
    float rLen = (info->boundsRadius * 0.5f) - info->boundsRadius;
    float probe[3][3] = {
        { fwd[0] * hLen, fwd[1] * hLen, fwd[2] * hLen },
        { bwd[0] * hLen, bwd[1] * hLen, bwd[2] * hLen },
        { right[0] * rLen * 0.85f, right[1] * rLen * 0.85f,
          right[2] * rLen * 0.85f },
    };
    int hitCount = 0;
    int result = 0;
    float pushDir[3];
    float hitNormal[3];
    float curTarget[3] = { target[0], target[1], target[2] };
    for (int i = 0; i < 3; ++i)
    {
        float radius = info->boundsRadius;
        float probeLen = sqrtf(probe[i][0] * probe[i][0]
                               + probe[i][1] * probe[i][1]
                               + probe[i][2] * probe[i][2]);
        float probeDir[3];
        if (probeLen != 0.0f)
        {
            probeDir[0] = probe[i][0] / probeLen;
            probeDir[1] = probe[i][1] / probeLen;
            probeDir[2] = probe[i][2] / probeLen;
        }
        else
        {
            probeDir[0] = probeDir[1] = probeDir[2] = 0.0f;
        }
        float scale = 1.0f;
        if (i == 2)
            scale = 0.7f;
        float probeRadius = info->boundsRadius * scale;
        float probeCenter[3];
        probeCenter[0] = curTarget[0] + probeDir[0] * probeRadius;
        probeCenter[1] = curTarget[1] + probeDir[1] * probeRadius;
        probeCenter[2] = curTarget[2] + probeDir[2] * probeRadius;
        math::Position3 in;
        math::Position3 out;
        in.v.m128_f32[0] = probeCenter[0];
        in.v.m128_f32[1] = probeCenter[1];
        in.v.m128_f32[2] = probeCenter[2];
        bool collided = collide_sphere(ent, proximity, entities, in, radius,
                                       out);
        if (!collided)
        {
            in.v.m128_f32[2] += 18.0f;
            collided = collide_sphere(ent, proximity, entities, in, radius,
                                      out);
            if (!collided)
            {
                in.v.m128_f32[2] -= 36.0f;
                collided = collide_sphere(ent, proximity, entities, in,
                                          radius, out);
            }
        }
        if (collided)
        {
            pushDir[0] = out.v.m128_f32[0] - in.v.m128_f32[0];
            pushDir[1] = out.v.m128_f32[1] - in.v.m128_f32[1];
            pushDir[2] = out.v.m128_f32[2] - in.v.m128_f32[2];
            ++hitCount;
            float plen = VectorNormalize(pushDir);
            (void)plen;
            hitNormal[0] = pushDir[0];
            hitNormal[1] = pushDir[1];
            hitNormal[2] = pushDir[2];
            result = 1.0f;
            curTarget[0] = out.v.m128_f32[0] - pushDir[0] * (1.0f + scale);
            curTarget[1] = out.v.m128_f32[1] - pushDir[1] * (1.0f + scale);
            curTarget[2] = out.v.m128_f32[2] - pushDir[2] * (1.0f + scale);
        }
        else
        {
            result = 0.0f;
        }
        if (i + 1 >= 3)
            break;
    }
    if (move != 0)
    {
        if (result < 3 || allowHit != 0)
        {
            float dz = fabsf(veh->phys.origin.v.m128_f32[2] - curTarget[2]);
            if (dz < 36.0f)
            {
                veh->phys.origin.v.m128_f32[0] = curTarget[0];
                veh->phys.origin.v.m128_f32[1] = curTarget[1];
                veh->phys.origin.v.m128_f32[2] = curTarget[2];
            }
        }
        if (result != 0)
        {
            float speed = sqrtf(veh->phys.vel.v.m128_f32[0]
                                    * veh->phys.vel.v.m128_f32[0]
                                + veh->phys.vel.v.m128_f32[1]
                                      * veh->phys.vel.v.m128_f32[1]
                                + veh->phys.vel.v.m128_f32[2]
                                      * veh->phys.vel.v.m128_f32[2]);
            if (speed > 40.0f && veh->lastCollision < level.time - 1000)
            {
                float intensity = (speed - 20.0f) * 0.0033333334f;
                float velDir[3];
                VectorNormalize2(&veh->phys.vel.v.m128_f32[0], velDir);
                float dot = 1.0f
                          - (velDir[0] * hitNormal[0]
                             + velDir[1] * hitNormal[1]
                             + velDir[2] * hitNormal[2]);
                if (dot < 0.0f)
                    dot = 0.0f;
                else if (dot > 1.0f)
                    dot = 1.0f;
                intensity *= dot;
                veh->crashVolume = intensity;
                VEH_JoltBody(ent, *((const math::Position3*)hitNormal), intensity,
                             0.0f, 0.0f);
                veh->crashSound = 1;
            }
        }
    }
    return hitCount;
}

// ea: 0x0045CE60
int VEH_FindValidDismountSpot(Entity* ent, float* mins, float* maxs,
                              float* origin, Entity* player,
                              bool bOriginInput)
{
    if (player == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 5934;
        AeAssert::gCurrentExpr = "player";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (ent == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 5940;
        AeAssert::gCurrentExpr = "ent";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    scr_vehicle_t* veh = ent->scr_vehicle;
    if (veh == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 5941;
        AeAssert::gCurrentExpr = "ent->scr_vehicle";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    vehicle_info_t* info = s_vehicleInfos[veh->infoIdx];
    if (bOriginInput)
    {
        if (ent->health > 0)
        {
            float testOrigin[3] = { origin[0], origin[1], origin[2] };
            float delta[3];
            delta[0] = testOrigin[0] - veh->phys.origin.v.m128_f32[0];
            delta[1] = testOrigin[1] - veh->phys.origin.v.m128_f32[1];
            delta[2] = testOrigin[2] - veh->phys.origin.v.m128_f32[2];
            float dist = sqrtf(delta[0] * delta[0] + delta[1] * delta[1]
                               + delta[2] * delta[2]);
            if (dist > 0.0f)
            {
                delta[0] /= dist;
                delta[1] /= dist;
                delta[2] /= dist;
            }
            collision_context_t ctx;

            ctx.pass_entity1 = player->mHandle;
            ctx.pass_entity2.mHandle.mVal = 0x2000000;
            ctx.pass_owner1.mHandle.mVal = 0;
            ctx.pass_owner2.mHandle.mVal = 0;
            ctx.contentmask = 0;
            for (int i = 0; i < 4; ++i)
            {
                math::Position3 s;
                math::Position3 e;
                s.v.m128_f32[0] = testOrigin[0];
                s.v.m128_f32[1] = testOrigin[1];
                s.v.m128_f32[2] = testOrigin[2];
                e.v.m128_f32[0] = testOrigin[0];
                e.v.m128_f32[1] = testOrigin[1];
                e.v.m128_f32[2] = testOrigin[2];
                math::Position3 zero;
                zero.v = _mm_setzero_ps();
                trace_t tr;
                memset(&tr, 0, sizeof(tr));
                SV_Trace(&tr, &s, &zero, &zero, &e, &ctx, 0, 0, nullptr, 0,
                         0.0f);
                if (!tr.allsolid)
                {
                    math::Position3 start;
                    math::Position3 end;
                    start.v.m128_f32[0] = testOrigin[0];
                    start.v.m128_f32[1] = testOrigin[1];
                    start.v.m128_f32[2] = testOrigin[2] + 30.0f;
                    end.v.m128_f32[0] = veh->phys.origin.v.m128_f32[0];
                    end.v.m128_f32[1] = veh->phys.origin.v.m128_f32[1];
                    end.v.m128_f32[2] =
                        veh->phys.origin.v.m128_f32[2] + 30.0f;
                    math::Position3 pmins;
                    pmins.v.m128_f32[0] = mins[0];
                    pmins.v.m128_f32[1] = mins[1];
                    pmins.v.m128_f32[2] = mins[2];
                    math::Position3 pmaxs;
                    pmaxs.v.m128_f32[0] = maxs[0];
                    pmaxs.v.m128_f32[1] = maxs[1];
                    pmaxs.v.m128_f32[2] = maxs[2] - 30.0f;
                    memset(&tr, 0, sizeof(tr));
                    SV_Trace(&tr, &start, &pmins, &pmaxs, &end, &ctx, 0, 0,
                             nullptr, 0, 0.0f);
                    if (!tr.allsolid
                        && (tr.fraction == 1.0f
                            || tr.mEntity.mHandle.mVal
                                   == ent->mHandle.mHandle.mVal))
                    {
                        origin[0] = testOrigin[0];
                        origin[1] = testOrigin[1];
                        origin[2] = testOrigin[2];
                        return 1;
                    }
                }
                testOrigin[0] += delta[0] * 16.0f;
                testOrigin[1] += delta[1] * 16.0f;
            }
        }
        else
        {
            return 1;
        }
    }
    if (info->maxSpeed > 0.0f)
    {
        float fwd[3], right[3], up[3];
        AngleVectors(&veh->phys.angles.v.m128_f32[0], fwd, right, up);
        float velLen = sqrtf(veh->phys.vel.v.m128_f32[0]
                                 * veh->phys.vel.v.m128_f32[0]
                             + veh->phys.vel.v.m128_f32[1]
                                   * veh->phys.vel.v.m128_f32[1]
                             + veh->phys.vel.v.m128_f32[2]
                                   * veh->phys.vel.v.m128_f32[2]);
        float dir[3];
        if (velLen <= 0.0f)
        {
            dir[0] = -fwd[0];
            dir[1] = -fwd[1];
            dir[2] = -fwd[2];
        }
        else
        {
            dir[0] = -veh->phys.vel.v.m128_f32[0];
            dir[1] = -veh->phys.vel.v.m128_f32[1];
            dir[2] = -veh->phys.vel.v.m128_f32[2];
        }
        VectorNormalize(dir);
        float fwdDist = (info->boundsLength * 0.5f) + maxs[0] + 40.0f;
        float rightDist = info->boundsRadius + maxs[1] + 40.0f;
        float side = 1.0f;
        if (fwd[0] * dir[0] + fwd[1] * dir[1] + fwd[2] * dir[2] <= 0.0f)
            side = -1.0f;
        float dfc = side * fwdDist;
        if (dfc > rightDist)
            dfc = rightDist;
        else if (dfc < -rightDist)
            dfc = -rightDist;
        float dr = 0.0f;
        if (rightDist < 0.0f)
            dr = rightDist;
        collision_context_t ctx;

        ctx.pass_entity1 = player->mHandle;
        ctx.pass_entity2.mHandle.mVal = 0x2000000;
        ctx.pass_owner1.mHandle.mVal = 0;
        ctx.pass_owner2.mHandle.mVal = 0;
        ctx.contentmask = 0;
        for (int i = 0; i < 8; ++i)
        {
            float offF = (i & 4) ? -dfc : dfc;
            float offR = dr + 20.0f * (i & 3);
            float tryOrigin[3];
            tryOrigin[0] = veh->phys.origin.v.m128_f32[0]
                         + fwd[0] * offF + right[0] * offR;
            tryOrigin[1] = veh->phys.origin.v.m128_f32[1]
                         + fwd[1] * offF + right[1] * offR;
            tryOrigin[2] = veh->phys.origin.v.m128_f32[2] + 4.0f;
            for (int dz = 0; dz < 2; ++dz)
            {
                for (float z = tryOrigin[2]; z >= tryOrigin[2] - 256.0f;
                     z -= 32.0f)
                {
                    math::Position3 start;
                    math::Position3 end;
                    start.v.m128_f32[0] = tryOrigin[0];
                    start.v.m128_f32[1] = tryOrigin[1];
                    start.v.m128_f32[2] = z + 1.0f;
                    end.v.m128_f32[0] = tryOrigin[0];
                    end.v.m128_f32[1] = tryOrigin[1];
                    end.v.m128_f32[2] = z;
                    math::Position3 pmins;
                    pmins.v.m128_f32[0] = mins[0];
                    pmins.v.m128_f32[1] = mins[1];
                    pmins.v.m128_f32[2] = mins[2];
                    math::Position3 pmaxs;
                    pmaxs.v.m128_f32[0] = maxs[0];
                    pmaxs.v.m128_f32[1] = maxs[1];
                    pmaxs.v.m128_f32[2] = maxs[2];
                    trace_t tr;
                    memset(&tr, 0, sizeof(tr));
                    SV_Trace(&tr, &start, &pmins, &pmaxs, &end, &ctx, 1, 0,
                             nullptr, 0, 0.0f);
                    if (tr.fraction == 1.0f && !tr.allsolid)
                    {
                        math::Position3 dstart;
                        math::Position3 dend;
                        dstart.v.m128_f32[0] = tryOrigin[0];
                        dstart.v.m128_f32[1] = tryOrigin[1];
                        dstart.v.m128_f32[2] = tryOrigin[2];
                        dend.v.m128_f32[0] = tryOrigin[0];
                        dend.v.m128_f32[1] = tryOrigin[1];
                        dend.v.m128_f32[2] = z - 256.0f;
                        memset(&tr, 0, sizeof(tr));
                        SV_Trace(&tr, &dstart, &pmins, &pmaxs, &dend, &ctx,
                                 1, 0, nullptr, 0, 0.0f);
                        if (!tr.allsolid && tr.fraction < 1.0f)
                        {
                            math::Position3 cstart;
                            math::Position3 cend;
                            cstart.v.m128_f32[0] =
                                veh->phys.origin.v.m128_f32[0];
                            cstart.v.m128_f32[1] =
                                veh->phys.origin.v.m128_f32[1];
                            cstart.v.m128_f32[2] =
                                veh->phys.origin.v.m128_f32[2] + 30.0f;
                            cend.v.m128_f32[0] = tryOrigin[0];
                            cend.v.m128_f32[1] = tryOrigin[1];
                            cend.v.m128_f32[2] = tryOrigin[2] + 30.0f;
                            math::Position3 cmins;
                            cmins.v.m128_f32[0] = mins[0];
                            cmins.v.m128_f32[1] = mins[1];
                            cmins.v.m128_f32[2] = mins[2];
                            math::Position3 cmaxs;
                            cmaxs.v.m128_f32[0] = maxs[0];
                            cmaxs.v.m128_f32[1] = maxs[1];
                            cmaxs.v.m128_f32[2] = maxs[2] - 30.0f;
                            memset(&tr, 0, sizeof(tr));
                            SV_Trace(&tr, &cstart, &cmins, &cmaxs, &cend,
                                     &ctx, 1, 0, nullptr, 0, 0.0f);
                            if (!tr.allsolid
                                && (tr.fraction == 1.0f
                                    || tr.mEntity.mHandle.mVal
                                           == ent->mHandle.mHandle.mVal))
                            {
                                origin[0] = tryOrigin[0];
                                origin[1] = tryOrigin[1];
                                origin[2] = z;
                                return 1;
                            }
                        }
                    }
                }
            }
        }
        return 0;
    }
    if (veh->boneIndex.detach >= 0)
    {
        DObjSkelMat mtx;
        G_DObjGetWorldBoneIndexMatrix(ent, veh->boneIndex.detach, &mtx);
        origin[0] = mtx.origin[0];
        origin[1] = mtx.origin[1];
        origin[2] = mtx.origin[2];
    }
    else
    {
        origin[0] = ent->r.currentOrigin.v.m128_f32[0];
        origin[1] = ent->r.currentOrigin.v.m128_f32[1];
        origin[2] = ent->r.currentOrigin.v.m128_f32[2]
                  + (float)(info->mMantleHintStringIndex + 48);
    }
    collision_context_t ctx;

    ctx.pass_entity1 = player->mHandle;
    ctx.pass_entity2.mHandle.mVal = 0x2000000;
    ctx.pass_owner1.mHandle.mVal = 0;
    ctx.pass_owner2.mHandle.mVal = 0;
    ctx.contentmask = 0;
    for (int i = 0; i < 8; ++i)
    {
        math::Position3 start;
        math::Position3 end;
        start.v.m128_f32[0] = origin[0];
        start.v.m128_f32[1] = origin[1];
        start.v.m128_f32[2] = origin[2];
        end.v.m128_f32[0] = origin[0];
        end.v.m128_f32[1] = origin[1];
        end.v.m128_f32[2] = origin[2];
        math::Position3 pmins;
        pmins.v.m128_f32[0] = mins[0];
        pmins.v.m128_f32[1] = mins[1];
        pmins.v.m128_f32[2] = mins[2];
        math::Position3 pmaxs;
        pmaxs.v.m128_f32[0] = maxs[0];
        pmaxs.v.m128_f32[1] = maxs[1];
        pmaxs.v.m128_f32[2] = maxs[2];
        trace_t tr;
        memset(&tr, 0, sizeof(tr));
        SV_Trace(&tr, &start, &pmins, &pmaxs, &end, &ctx, 1, 0, nullptr, 0,
                 0.0f);
        if (tr.fraction == 1.0f && !tr.allsolid)
            return 1;
        origin[2] += 4.0f;
    }
    return 0;
}

// ea: 0x0047C4F0
void VEH_GroundPlant(Entity* ent, int gravity, int msec)
{
    scr_vehicle_t* veh = ent->scr_vehicle;
    vehicle_info_t* info = s_vehicleInfos[veh->infoIdx];
    if (info->type != 1 && info->type != 2)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 2746;
        AeAssert::gCurrentExpr =
            "( info->type == VEH_WHEELS_4 ) || ( info->type == VEH_TANK )";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    int numWheels = 2 * (info->type != 1) + 4;
    if (ent->takedamage == 0)
    {
        for (int i = 0; i < numWheels; ++i)
        {
            if (veh->mWheel_ParticleEffectHandle[i].mVal != 0)
            {
                EffectEventStopEmitting(veh->mWheel_ParticleEffectHandle[i]);
                veh->mWheel_ParticleEffectHandle[i].mVal = 0;
            }
        }
        if (veh->mRumbleEffectHandle.mVal != 0)
        {
            EffectEventStopEmitting(veh->mRumbleEffectHandle);
            veh->mRumbleEffectHandle.mVal = 0;
        }
        return;
    }
    float trans[3][3];
    AnglesToAxis(&veh->phys.angles.v.m128_f32[0], trans);
    float curMin[3] = { 3.4028235e38f, 3.4028235e38f, 3.4028235e38f };
    float curMax[3] = { -3.4028235e38f, -3.4028235e38f, -3.4028235e38f };
    float wheelPos[6][3];
    for (int i = 0; i < numWheels; ++i)
    {
        int bone = VEH_GetWheelOrigin(ent);
        DObjSkelMat mtx;
        if (bone >= 0)
            G_DObjGetWorldBoneIndexMatrix(ent, bone, &mtx);
        float local[3] = { mtx.origin[0] - veh->phys.origin.v.m128_f32[0],
                           mtx.origin[1] - veh->phys.origin.v.m128_f32[1],
                           mtx.origin[2] - veh->phys.origin.v.m128_f32[2] };
        float t[3];
        MatrixTransformVector43(local, trans, t);
        wheelPos[i][0] = t[0];
        wheelPos[i][1] = t[1];
        wheelPos[i][2] = t[2];
        if (t[0] < curMin[0]) curMin[0] = t[0];
        if (t[1] < curMin[1]) curMin[1] = t[1];
        if (t[2] - 256.0f < curMin[2]) curMin[2] = t[2] - 256.0f;
        if (t[0] > curMax[0]) curMax[0] = t[0];
        if (t[1] > curMax[1]) curMax[1] = t[1];
        if (t[2] + 64.0f > curMax[2]) curMax[2] = t[2] + 64.0f;
    }
    if (ent->proximity_data != nullptr)
    {
        math::Position3 lo;
        math::Position3 hi;
        lo.v.m128_f32[0] =
            curMin[0] + veh->phys.origin.v.m128_f32[0] - r;
        lo.v.m128_f32[1] =
            curMin[1] + veh->phys.origin.v.m128_f32[1] - r;
        lo.v.m128_f32[2] =
            curMin[2] + veh->phys.origin.v.m128_f32[2] - r;
        hi.v.m128_f32[0] =
            curMax[0] + veh->phys.origin.v.m128_f32[0] + r;
        hi.v.m128_f32[1] =
            curMax[1] + veh->phys.origin.v.m128_f32[1] + r;
        hi.v.m128_f32[2] =
            curMax[2] + veh->phys.origin.v.m128_f32[2] + r;
        query_proximity_data(lo, hi, *ent->proximity_data);
    }
    proximity_data_t filtered;
    memset(&filtered, 0, sizeof(filtered));
    int mask = (ent->active != 2) ? 593 : 0x10000 + 593;
    for (int i = 0; i < numWheels; ++i)
    {
        int bone = VEH_GetWheelOrigin(ent);
        DObjSkelMat mtx;
        if (bone >= 0)
            G_DObjGetWorldBoneIndexMatrix(ent, bone, &mtx);
        float local[3] = { mtx.origin[0] - veh->phys.origin.v.m128_f32[0],
                           mtx.origin[1] - veh->phys.origin.v.m128_f32[1],
                           mtx.origin[2] - veh->phys.origin.v.m128_f32[2] };
        float wheelCenter[3];
        MatrixTransformVector43(local, trans, wheelCenter);
        float start[3] = {
            wheelCenter[0] + veh->phys.origin.v.m128_f32[0],
            wheelCenter[1] + veh->phys.origin.v.m128_f32[1],
            wheelCenter[2] + veh->phys.origin.v.m128_f32[2] + 64.0f,
        };
        float end[3] = {
            wheelCenter[0] + veh->phys.origin.v.m128_f32[0],
            wheelCenter[1] + veh->phys.origin.v.m128_f32[1],
            wheelCenter[2] + veh->phys.origin.v.m128_f32[2] - 256.0f,
        };
        float t = 1.0f;
        int sflags = 0;
        int cflags = 0;
        math::Position3 loP;
        math::Position3 hiP;
        loP.v.m128_f32[0] = start[0] < end[0] ? start[0] : end[0];
        loP.v.m128_f32[1] = start[1] < end[1] ? start[1] : end[1];
        loP.v.m128_f32[2] = start[2] < end[2] ? start[2] : end[2];
        hiP.v.m128_f32[0] = start[0] > end[0] ? start[0] : end[0];
        hiP.v.m128_f32[1] = start[1] > end[1] ? start[1] : end[1];
        hiP.v.m128_f32[2] = start[2] > end[2] ? start[2] : end[2];
        filter_proximity_data(loP, hiP, mask, *ent->proximity_data,
                              filtered);
        math::Position3 p0;
        math::Position3 p1;
        p0.v.m128_f32[0] = start[0];
        p0.v.m128_f32[1] = start[1];
        p0.v.m128_f32[2] = start[2];
        p1.v.m128_f32[0] = end[0];
        p1.v.m128_f32[1] = end[1];
        p1.v.m128_f32[2] = end[2];
        collide_segment(filtered, p0, p1, t, sflags, cflags, nullptr);
        if (t < 1.0f)
        {
            start[0] += (end[0] - start[0]) * t;
            start[1] += (end[1] - start[1]) * t;
            start[2] += (end[2] - start[2]) * t;
        }
        if (t > 0.4f)
        {
            collision_context_t ctx;

            ctx.pass_entity1.mHandle.mVal = 0;
            ctx.pass_entity2.mHandle.mVal = 0;
            ctx.pass_owner1.mHandle.mVal = 0;
            ctx.pass_owner2.mHandle.mVal = 0;
            ctx.contentmask = mask;
            math::Position3 zero;
            zero.v = _mm_setzero_ps();
            math::Position3 s;
            s.v.m128_f32[0] = start[0];
            s.v.m128_f32[1] = start[1];
            s.v.m128_f32[2] = start[2];
            math::Position3 e;
            e.v.m128_f32[0] = end[0];
            e.v.m128_f32[1] = end[1];
            e.v.m128_f32[2] = end[2];
            trace_t tr;
            memset(&tr, 0, sizeof(tr));
            SV_Trace(&tr, &s, &zero, &zero, &e, &ctx, 0, 0, nullptr, 0,
                     0.0f);
            t = tr.fraction;
            start[0] = tr.endpos.v.m128_f32[0];
            start[1] = tr.endpos.v.m128_f32[1];
            start[2] = tr.endpos.v.m128_f32[2];
            sflags = tr.surfaceFlags;
        }
        float targetZ;
        if (t >= 1.0f)
        {
            targetZ = end[2];
            veh->phys.wheelZPos[i] = 0.0f;
        }
        else
        {
            targetZ = start[2];
            veh->phys.wheelZPos[i] = (float)((sflags >> 20) & 0x1F);
        }
        if (gravity != 0)
        {
            float newZ = veh->phys.wheelZVel[i] - (msec * 0.001f) * 800.0f;
            veh->phys.wheelZVel[i] = newZ;
            float v = veh->phys.wheelZPos[i] + newZ * (msec * 0.001f);
            if (targetZ > v)
            {
                v = targetZ;
                veh->phys.wheelZVel[i] = 0.0f;
            }
            veh->phys.wheelZPos[i] = v;
        }
        else
        {
            veh->phys.wheelZPos[i] = targetZ;
            veh->phys.wheelZVel[i] = 0.0f;
        }
        wheelPos[i][0] = start[0];
        wheelPos[i][1] = start[1];
        wheelPos[i][2] = veh->phys.wheelZPos[i];
        if (i < 4)
        {
            math::Position3 hp;
            hp.v.m128_f32[0] = start[0];
            hp.v.m128_f32[1] = start[1];
            hp.v.m128_f32[2] = start[2];
            math::Dir3 hn;
            hn.v.m128_f32[0] = 0.0f;
            hn.v.m128_f32[1] = 0.0f;
            hn.v.m128_f32[2] = 1.0f;
            UpdateWheelMarks(ent, i, veh->phys.wheelZVel[i] != 0.0f, hp,
                             hn);
        }
        float dz = (veh->phys.origin.v.m128_f32[2] - veh->phys.wheelZPos[i])
                 - veh->wheelRadius;
        if (dz > 15.0f)
            dz = 15.0f;
        else if (dz < -15.0f)
            dz = -15.0f;
        float steer = -dz;
        float angles[3] = { -veh->wheelPitch, 0.0f, 0.0f };
        if (i == TAG_WHEEL_FRONT_LEFT || i == TAG_WHEEL_FRONT_RIGHT)
            angles[1] = veh->current.mSteeringAngle;
        if (bone >= 0)
            G_DObjSetLocalTagInternal_0(&steer, angles, bone, ent, 0);
        VEH_UpdateWheelParticleEffects(ent, i);
    }
    float forward[3] = {
        (wheelPos[0][0] + wheelPos[1][0]) * 0.5f
            - (wheelPos[3][0] + wheelPos[2][0]) * 0.5f,
        (wheelPos[0][1] + wheelPos[1][1]) * 0.5f
            - (wheelPos[3][1] + wheelPos[2][1]) * 0.5f,
        (wheelPos[0][2] + wheelPos[1][2]) * 0.5f
            - (wheelPos[3][2] + wheelPos[2][2]) * 0.5f,
    };
    VectorNormalize(forward);
    float right[3] = {
        (wheelPos[3][0] + wheelPos[0][0]) * 0.5f
            - (wheelPos[2][0] + wheelPos[1][0]) * 0.5f,
        (wheelPos[3][1] + wheelPos[0][1]) * 0.5f
            - (wheelPos[2][1] + wheelPos[1][1]) * 0.5f,
        (wheelPos[3][2] + wheelPos[0][2]) * 0.5f
            - (wheelPos[2][2] + wheelPos[1][2]) * 0.5f,
    };
    VectorNormalize(right);
    float up[3];
    CrossProduct(forward, right, up);
    float planeD = up[0] * wheelPos[0][0] + up[1] * wheelPos[0][1]
                 + up[2] * wheelPos[0][2];
    for (int i = 1; i < numWheels; ++i)
    {
        float d = up[0] * wheelPos[i][0] + up[1] * wheelPos[i][1]
                + up[2] * wheelPos[i][2];
        if (d - planeD > info->suspensionTravel)
            planeD = d - info->suspensionTravel;
    }
    float rollAxis[3];
    CrossProduct(up, trans[2], rollAxis);
    VectorNormalize(rollAxis);
    float pitchAxis[3];
    CrossProduct(rollAxis, up, pitchAxis);
    VectorNormalize(pitchAxis);
    float ang[3];
    AxisToAngles((const float(*)[3])pitchAxis, ang);
    float dT = msec * 0.001f;
    float pitch = ang[0];
    float prevPitch = veh->phys.prevAngles.v.m128_f32[0];
    while (pitch - prevPitch > 180.0f) pitch -= 360.0f;
    while (pitch - prevPitch < -180.0f) pitch += 360.0f;
    float pStep = (pitch - prevPitch) * dT * 6.0f;
    veh->phys.angles.v.m128_f32[0] = AngleNormalize180(
        (fabsf(pitch - prevPitch) <= 0.005f
         || fabsf(pStep) > fabsf(pitch - prevPitch))
            ? pitch
            : prevPitch + pStep);
    float roll = ang[2];
    float prevRoll = veh->phys.prevAngles.v.m128_f32[2];
    while (roll - prevRoll > 180.0f) roll -= 360.0f;
    while (roll - prevRoll < -180.0f) roll += 360.0f;
    float rStep = (roll - prevRoll) * dT * 6.0f;
    veh->phys.angles.v.m128_f32[2] = AngleNormalize180(
        (fabsf(roll - prevRoll) <= 0.005f
         || fabsf(rStep) > fabsf(roll - prevRoll))
            ? roll
            : prevRoll + rStep);
    if (veh->phys.angles.v.m128_f32[0] < -60.0f)
        veh->phys.angles.v.m128_f32[0] = -60.0f;
    else if (veh->phys.angles.v.m128_f32[0] > 60.0f)
        veh->phys.angles.v.m128_f32[0] = 60.0f;
    if (veh->phys.angles.v.m128_f32[2] < -60.0f)
        veh->phys.angles.v.m128_f32[2] = -60.0f;
    else if (veh->phys.angles.v.m128_f32[2] > 60.0f)
        veh->phys.angles.v.m128_f32[2] = 60.0f;
    if (ent->active != 2)
    {
        veh->phys.origin.v.m128_f32[2] =
            (planeD - (up[0] * veh->phys.origin.v.m128_f32[0]
                       + up[1] * veh->phys.origin.v.m128_f32[1]))
            / up[2];
    }
    AnglesSubtract(*(const math::Position3*)&veh->phys.angles,
                   *(const math::Position3*)&veh->phys.prevAngles,
                   *(math::Position3*)&veh->phys.rotVel);
    float invD = 1.0f / dT;
    veh->phys.rotVel.v.m128_f32[0] *= invD;
    veh->phys.rotVel.v.m128_f32[1] *= invD;
    veh->phys.rotVel.v.m128_f32[2] *= invD;
    if (veh->engineSndLerp > 0.1f)
    {
        if (veh->mRumbleEffectHandle.mVal == 0)
        {
            Entity* owner = HandleDbToEnt(ent->r.mOwner);
            if (owner == nullptr
                || owner != EntityManager::sInst->GetPlayer(currCl))
            {
                veh->mRumbleEffectHandle = PostEffectEventVehicle(
                    ent, info->name, (EAction)40 /* kActionVEHICLE_BRAKE */);
            }
        }
    }
    else if (veh->mRumbleEffectHandle.mVal != 0)
    {
        EffectEventStopEmitting(veh->mRumbleEffectHandle);
        veh->mRumbleEffectHandle.mVal = 0;
    }
}

void Scr_Vehicle_Think(Entity* pSelf, int msec)
{
    if (pSelf->scr_vehicle == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 7885;
        AeAssert::gCurrentExpr = "pSelf->scr_vehicle";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    scr_vehicle_t* veh = pSelf->scr_vehicle;
    vehicle_info_t* info = s_vehicleInfos[veh->infoIdx];
    if (pSelf->active == 2 && s_clientThink == 0)
    {
        pSelf->nextthink = level.time;
        return;
    }
    VEH_Backup(pSelf);
    memset(&s_phys, 0, sizeof(s_phys));
    int eFlags = pSelf->s.eFlags;
    int contents = pSelf->r.contents;
    if (pSelf->takedamage == 0)
    {
        pSelf->s.eFlags = eFlags | 0x80;
        if (contents != 0)
        {
            pSelf->s.brushmodel = 0;
            pSelf->r.contents = 0;
            g_LinkEntity(pSelf);
            SV_UnlinkEntity(pSelf);
        }
        pSelf->nextthink = level.time + 1;
        return;
    }
    pSelf->s.eFlags = eFlags & 0xFFFFFF7F;
    if (contents == 0)
    {
        pSelf->s.brushmodel = 0;
        SV_SetBrushModel(pSelf);
        pSelf->r.contents = 0xA00000;
        g_LinkEntity(pSelf);
    }
    if (veh->playersAttached != 0)
    {
        veh->lastOccupantTime = level.time;
        VEH_UpdateControllers(pSelf, msec);
    }
    if (veh->mRBVeh == nullptr)
    {
        Entity* mObject = HandleDbToEnt(pSelf->r.mOwner);
        if (EntityManager::sInst->IsLocalPlayer(mObject))
            veh->mPhysicsOwner.mHandle.mVal = pSelf->r.mOwner.mHandle.mVal;
    }
    Entity* physOwner = HandleDbToEnt(veh->mPhysicsOwner);
    if (physOwner != nullptr
        && (physOwner->client == nullptr
            || physOwner->client->pers.connected != 2 /* CON_CONNECTED */)
        && veh->mPhysicsOwner.mHandle.mVal
               == physOwner->mHandle.mHandle.mVal)
    {
        veh->mPhysicsOwner.mHandle.mVal = 0;
    }
    if (veh->playersAttached != 0)
    {
        for (int i = 0; i < 11; ++i)
        {
            Entity* occupant = HandleDbToEnt(veh->seats[i].occupant);
            if (occupant == nullptr)
                continue;
            if (IsPlayerFullySeatedInVehicle(occupant))
            {
                if (info->type == 2
                    && EntityManager::sInst->IsLocalPlayer(occupant)
                    && lastGunnerCrouchMsgLocal < level.time - 1000)
                {
                    bool v18 =
                        cl[EntityManager::sInst->GetPlayerIndex(occupant)]
                            .stanceHeld
                        || veh->forceGunnerCrouchTime > level.time;
                    if (i == 1)
                    {
                        if (v18)
                        {
                            MultiplayerMgr::sInst->AttemptVehicleSeatChange(
                                pSelf, occupant, 6);
                            lastGunnerCrouchMsgLocal = level.time;
                        }
                    }
                    else if (i == 6 && !v18)
                    {
                        MultiplayerMgr::sInst->AttemptVehicleSeatChange(
                            pSelf, occupant, 1);
                        lastGunnerCrouchMsgLocal = level.time;
                    }
                }
            }
            else
            {
                veh->UpdateAnimRoute(pSelf, occupant);
            }
        }
        if (info->type == 2)
        {
            if (veh->mHatchOpen)
            {
                if (veh->LetHatchClose())
                {
                    PostEffectEventScriptCall(pSelf, "TANK_HATCH_CLOSE", false,
                                              PAK_ID_INVALID, false);
                    veh->mHatchOpen = false;
                }
            }
            else if (!veh->LetHatchClose())
            {
                PostEffectEventScriptCall(pSelf, "TANK_HATCH_OPEN", false,
                                          PAK_ID_INVALID, false);
                veh->mHatchOpen = true;
            }
        }
    }
    else if (info->type == 2 && !veh->mHatchOpen)
    {
        PostEffectEventScriptCall(pSelf, "TANK_HATCH_OPEN", false,
                                  PAK_ID_INVALID, false);
        veh->mHatchOpen = true;
    }
    Entity* owner = HandleDbToEnt(pSelf->r.mOwner);
    if (owner != nullptr)
    {
        if (EntityManager::sInst->IsLocalPlayer(owner))
        {
            if (veh->mMantleTime != 0 && veh->mMantleTime < level.time)
            {
                Entity* mantleEnt = HandleDbToEnt(veh->mMantleEntity);
                if (mantleEnt == nullptr)
                {
                    AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
                    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
                    AeAssert::gCurrentLine = 8062;
                    AeAssert::gCurrentExpr = "*veh->mMantleEntity";
                    if (!AeAssert::IsIgnored()
                        && AeAssert::Assert("Invalid mantle entity"))
                        __debugbreak();
                }
                G_Damage(pSelf, mantleEnt, mantleEnt, nullptr,
                         pSelf->r.currentOrigin.v.m128_f32, pSelf->health + 100,
                         160, 27, HITLOC_NONE, -1);
            }
            if (owner->health <= 0)
            {
                tlPrintf("=======================================GetOutOfVehicle cause owner has no health\n");
                MultiplayerMgr::sInst->GetOutOfVehicle(
                    pSelf, owner->client->ps.vehPos);
            }
        }
    }
    else if (pSelf->health > 0 && MultiplayerMgr::sInst->IsHost())
    {
        if (veh->mMantleTime != 0 && veh->mMantleTime < level.time)
        {
            Entity* mantleEnt = HandleDbToEnt(veh->mMantleEntity);
            if (mantleEnt == nullptr)
            {
                AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
                AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
                AeAssert::gCurrentLine = 8094;
                AeAssert::gCurrentExpr = "*veh->mMantleEntity";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert("Invalid mantle entity"))
                    __debugbreak();
            }
            G_Damage(pSelf, mantleEnt, mantleEnt, nullptr,
                     pSelf->r.currentOrigin.v.m128_f32, pSelf->health + 100,
                     160, 27, HITLOC_NONE, -1);
        }
        if (veh->playersAttached == 0 && veh->lastOccupantTime != 0)
        {
            int inactiveBlowupSeconds = info->inactiveBlowupSeconds;
            if (inactiveBlowupSeconds == 0)
                inactiveBlowupSeconds = 60;
            if (veh->lastOccupantTime
                < level.time - 1000 * inactiveBlowupSeconds)
            {
                G_Damage(pSelf, nullptr, nullptr, nullptr, nullptr,
                         pSelf->health - 1, 0, 20, HITLOC_NONE, -1);
                unsigned int handle = pSelf->mHandle.mHandle.mVal;
                pSelf->Notify(hash_const.killanimscript, &handle);
                veh->lastOccupantTime = 0;
            }
        }
    }
    vehicle_info_t* v36 = info;
    int v37 = msec;
    if (info->type != 5)
    {
        if (pSelf->active == 1)
        {
            VEH_UpdatePath(pSelf, msec);
        }
        else if (pSelf->active == 2)
        {
            if (*(char*)((char*)info + 0x21C) == 0)
                VEH_UpdateClient(pSelf, msec);
            else if (HandleDbToEnt(pSelf->r.mOwner) == nullptr)
                pSelf->speed = 0.0f;
        }
    }
    if (*(char*)((char*)info + 0x21C) != 0)
    {
        VEH_UpdateParticlesRBVeh(pSelf);
    }
    else if (info->type == 1 || info->type == 2)
    {
        if (Entity_has_zone_collision(pSelf))
            VEH_GroundPlant(pSelf, 1, v37);
        else
            VEH_StopWheelEffects(pSelf);
    }
    if (pSelf->speed < 0.0f)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 8166;
        AeAssert::gCurrentExpr = "pSelf->speed >= 0.0f";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    Entity* physOwner2 = HandleDbToEnt(veh->mPhysicsOwner);
    if (EntityManager::sInst->IsLocalPlayer(physOwner2)
        && (veh->mRBVeh == nullptr
            || (((rb_vehicle*)veh->mRBVeh)->m_flags & 0x300) != 0)
        && info->type != 5
        && pSelf->active == 1)
    {
        VEH_VerifyPosition(pSelf);
    }
    if (*(char*)((char*)info + 0x21C) == 0)
        VEH_SetPosition(pSelf, veh->phys.origin, veh->phys.angles, veh->phys.vel);
    if (pSelf->health > 0)
    {
        collision_context_t context;

        context.pass_entity1.mHandle.mVal = 0;
        context.pass_entity2.mHandle.mVal = 0;
        context.pass_owner1.mHandle.mVal = 0;
        context.pass_owner2.mHandle.mVal = 0;
        context.contentmask = -1;
        G_DoTouchTriggers(pSelf, pSelf->r.currentOrigin, nullptr, context);
    }
    if (g_vehicleDebug.integer != 0)
        VEH_DebugBox(veh->phys.origin, 4.0f, 1.0f, 1.0f, 0.0f);
    veh->barrelBlocked = 0;
    if (pSelf->active == 2 && HandleDbToEnt(pSelf->r.mOwner) != nullptr)
        VEH_UpdateWeapon(pSelf, v37);
    VEH_UpdateAim(pSelf);
    VEH_UpdateGunnerAim(pSelf);
    VEH_UpdateControllers(pSelf, v37);
    VEH_UpdateAltWeapon(pSelf, v37);
    VEH_UpdateGunnerWeapon(pSelf, v37);
    VEH_UpdateOverHeat(pSelf, v37);
    if (veh->joltTime > 0.0f)
    {
        float joltWave = veh->joltWave;
        float v46 = v37 * 0.001f;
        veh->joltTime -= v46;
        float v48;
        if (joltWave < 0.0f || joltWave > 90.0f
            || veh->joltTime <= 0.48000002f)
            v48 = v46 * 450.0f;
        else
            v48 = (v46 * 450.0f) * 2.5f;
        veh->joltWave = v48 + veh->joltWave;
    }
    VEH_UpdateSteering(pSelf);
    VEH_UpdateHatch(pSelf, v37);
    VEH_UpdateFollow(pSelf);
    VEH_UpdateShaderTime(pSelf, v37);
    VEH_UpdateSounds(pSelf, v37);
    if (veh->mRBVeh == nullptr)
        ChiefMammalInChargeOfVehicleDamageAndPushOut(pSelf);
    pSelf->nextthink = level.time + 1;
}

static float rate = 1.0f;          // @ 0xDD7FDC (g_scr_vehicle.cpp local)
static float s_sndLerpMin = 0.1f;  // @ 0xDD7FE0 (g_scr_vehicle.cpp local)
static float intensity_scale = 500.0f;  // @ 0xDD7F48
static float hit_offset = 30.0f;        // @ 0xDD7F4C

// ea: 0x0044DD00
void VEH_Backup(Entity* ent)
{
    scr_vehicle_t* scr_vehicle = ent->scr_vehicle;
    scr_vehicle->phys.prevOrigin.v.m128_f32[0] = ent->r.currentOrigin.v.m128_f32[0];
    scr_vehicle->phys.prevOrigin.v.m128_f32[1] = ent->r.currentOrigin.v.m128_f32[1];
    scr_vehicle->phys.prevOrigin.v.m128_f32[2] = ent->r.currentOrigin.v.m128_f32[2];
    scr_vehicle->phys.prevAngles.v.m128_f32[0] = ent->r.currentAngles.v.m128_f32[0];
    scr_vehicle->phys.prevAngles.v.m128_f32[1] = ent->r.currentAngles.v.m128_f32[1];
    scr_vehicle->phys.prevAngles.v.m128_f32[2] = ent->r.currentAngles.v.m128_f32[2];
    s_backup.pathPos = scr_vehicle->pathPos;
    s_backup.phys = scr_vehicle->phys;
}

// ea: 0x0044D8F0
void VEH_JoltBody(Entity* ent, const math::Position3& dir, float intensity,
                  float speedFrac, float decel)
{
    scr_vehicle_t* scr_vehicle = ent->scr_vehicle;
    if (scr_vehicle != nullptr)
    {
        rb_vehicle* mRBVeh = (rb_vehicle*)scr_vehicle->mRBVeh;
        vehicle_info_t* v8 = s_vehicleInfos[scr_vehicle->infoIdx];
        if (mRBVeh != nullptr && v8->type == 2)
        {
            math::Position3 hitp;
            hitp.v = _mm_setzero_ps();
            hitp.v.m128_f32[2] = hit_offset;
            math::Position3 hitd;
            hitd.v = dir.v;
            ApplyPhysics(ent, &hitp, (const math::Dir3*)&hitd,
                         intensity_scale * intensity, true, HITLOC_TORSO_UPR);
        }
        if (intensity < 0.0f)
            intensity = 0.0f;
        else if (intensity > 1.0f)
            intensity = 1.0f;
        float axis[3][3];
        AnglesToAxis(scr_vehicle->phys.angles, axis);
        scr_vehicle->joltDir[0] = (dir.v.m128_f32[0] * axis[0][0])
                                + (dir.v.m128_f32[1] * axis[0][1])
                                + (dir.v.m128_f32[2] * axis[0][2]);
        scr_vehicle->joltDir[1] = -((dir.v.m128_f32[0] * axis[1][0])
                                  + (dir.v.m128_f32[1] * axis[1][1])
                                  + (dir.v.m128_f32[2] * axis[1][2]));
        scr_vehicle->joltTime = 0.80000001f;
        scr_vehicle->joltWave = 0.0f;
        VectorNormalize2D(scr_vehicle->joltDir);
        scr_vehicle->joltDir[0] = (v8->maxBodyPitch * scr_vehicle->joltDir[0]) * intensity;
        scr_vehicle->joltDir[1] = (v8->maxBodyRoll * scr_vehicle->joltDir[1]) * intensity;
    }
    else
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)AeAssert::JRS;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 1699;
        AeAssert::gCurrentExpr = "veh";
        if (!AeAssert::IsIgnored())
        {
            const char* v6 = ent->mClassName.mBlock != nullptr
                                 ? (const char*)&ent->mClassName.mBlock[1]
                                 : defaultFileName;
            if (AeAssert::Assert("Non vehicle entity %s passed to VEH_JoltBody", v6))
                __debugbreak();
        }
    }
}

// ea: 0x004888D0
void Scr_Vehicle_Touch(Entity* pSelf, Entity* pOther, int /*unused*/)
{
    if (pOther->client != nullptr)
        return;
    actor_s* actor = pOther->actor;
    if (actor != nullptr && actor->eSubState >= 800 && actor->eSubState <= 804)
        return;
    scr_vehicle_t* veh = pSelf->scr_vehicle;
    vehicle_info_t* info = s_vehicleInfos[veh->infoIdx];
    if ((pOther->s.eType != 1 && pOther->s.eType != 11)
        || pOther->tagInfo != nullptr || info->collisionDamage <= 0.0f)
        return;
    float vel[3];
    vel[0] = veh->phys.vel.v.m128_f32[0];
    vel[1] = veh->phys.vel.v.m128_f32[1];
    vel[2] = veh->phys.vel.v.m128_f32[2];
    if (sqrtf(vel[0] * vel[0] + vel[1] * vel[1] + vel[2] * vel[2]) < 1.0f
        && G_TestEntityPosition(pOther, pOther->r.currentOrigin) == nullptr)
        return;
    float delta[3];
    delta[0] = veh->phys.origin.v.m128_f32[0] - veh->phys.prevOrigin.v.m128_f32[0];
    delta[1] = veh->phys.origin.v.m128_f32[1] - veh->phys.prevOrigin.v.m128_f32[1];
    delta[2] = veh->phys.origin.v.m128_f32[2] - veh->phys.prevOrigin.v.m128_f32[2];
    math::Position3 deltaAngles;
    AnglesSubtract(veh->phys.angles, veh->phys.prevAngles, deltaAngles);
    math::Dir3 moveDir;
    math::Dir3 dir = native_to_cdl_dir3(delta);
    if (VectorNormalize2(dir, moveDir) < 0.005f)
        return;
    bool pushed;
    if (pOther->actor != nullptr && pSelf->scr_vehicle != nullptr)
    {
        pushed = push_entity(pOther, pSelf);
    }
    else
    {
        math::Position3 amove;
        math::Position3 move;
        amove = native_to_cdl_pos3(&deltaAngles.v.m128_f32[0]);
        move = native_to_cdl_pos3(delta);
        const math::Position3* pAmove = &amove;
        const math::Position3* pMove = &move;
        pushed = G_TryPushingEntity(pOther, pSelf, *pMove, *pAmove) != 0;
    }
    if (!pushed)
    {
        Entity* owner = HandleDbToEnt(pSelf->r.mOwner);
        G_Damage(pOther, pSelf, owner, &moveDir.v.m128_f32[0],
                 &pOther->r.currentOrigin.v.m128_f32[0], 999999, 32, 20,
                 HITLOC_NONE, -1);
        return;
    }
    math::Dir3 moveDirToEnt;
    moveDirToEnt.v.m128_f32[0] =
        pOther->r.currentOrigin.v.m128_f32[0] - pSelf->r.currentOrigin.v.m128_f32[0];
    moveDirToEnt.v.m128_f32[1] =
        pOther->r.currentOrigin.v.m128_f32[1] - pSelf->r.currentOrigin.v.m128_f32[1];
    moveDirToEnt.v.m128_f32[2] = 0.0f;
    VectorNormalize(moveDirToEnt);
    float dot = moveDir.v.m128_f32[0] * moveDirToEnt.v.m128_f32[0]
              + moveDir.v.m128_f32[1] * moveDirToEnt.v.m128_f32[1]
              + moveDir.v.m128_f32[2] * moveDirToEnt.v.m128_f32[2];
    if (dot < 0.8f)
        return;
    if (pOther->client != nullptr && (pOther->client->ps.pm_flags & 1) != 0)
    {
        Entity* owner = HandleDbToEnt(pSelf->r.mOwner);
        G_Damage(pOther, pSelf, owner, &moveDir.v.m128_f32[0],
                 &pOther->r.currentOrigin.v.m128_f32[0], 999999, 32, 20,
                 HITLOC_NONE, -1);
        return;
    }
    float speedFrac = pSelf->speed / info->collisionSpeed;
    if (speedFrac > 1.0f)
        speedFrac = 1.0f;
    int damage = (int)(((dot - 0.8f) * speedFrac * 5.0000005f)
                       * info->collisionDamage);
    if (damage > 0)
    {
        Entity* owner = HandleDbToEnt(pSelf->r.mOwner);
        G_Damage(pOther, pSelf, owner, &moveDir.v.m128_f32[0],
                 &pOther->r.currentOrigin.v.m128_f32[0], damage, 0, 20,
                 HITLOC_NONE, -1);
    }
}

// ea: 0x00470210
void scr_vehicle_t::DebugRender()
{
    if (sRenderEntryPoints == 0)
        return;
    int* entryPoint = boneIndex.entryPoint;
    for (int i = 6; i != 0; --i)
    {
        if (*entryPoint >= 0)
        {
            Entity* ent = HandleDbToEnt(mEntity);
            DObjSkelMat mat;
            G_DObjGetWorldBoneIndexMatrix(ent, *entryPoint, &mat);
            math::Position3 pos;
            pos.v.m128_f32[0] = mat.origin[0];
            pos.v.m128_f32[1] = mat.origin[1];
            pos.v.m128_f32[2] = mat.origin[2];
            pos.v.m128_f32[3] = 0.0f;
            float red[] = { 1.0f, 0.0f, 0.0f, 1.0f };
            DebugRender::RenderSphere(pos, 3.0f, Color(red[0], red[1], red[2], red[3]));
            float blue[] = { 0.0f, 0.0f, 1.0f, 0.1f };
            DebugRender::RenderSphere(pos, 50.0f, Color(blue[0], blue[1], blue[2], blue[3]));
        }
        ++entryPoint;
    }
    Entity* ent = HandleDbToEnt(mEntity);
    float yellow[] = { 1.0f, 1.0f, 0.0f, 0.1f };
    DebugRender::RenderSphere(ent->r.currentOrigin, mUseRadius, Color(yellow[0], yellow[1], yellow[2], yellow[3]));
    if (s_vehicleInfos[infoIdx]->type == 2)
    {
        int seatBone = seats[7].boneIndex;
        if (seatBone >= 0)
        {
            Entity* gunnerEnt = HandleDbToEnt(mEntity);
            DObjSkelMat mat;
            G_DObjGetWorldBoneIndexMatrix(gunnerEnt, seatBone, &mat);
            math::Position3 pos;
            pos.v.m128_f32[0] = mat.origin[0];
            pos.v.m128_f32[1] = mat.origin[1];
            pos.v.m128_f32[2] = mat.origin[2];
            pos.v.m128_f32[3] = 0.0f;
            float white[] = { 1.0f, 0.0f, 0.0f, 1.0f };
            DebugRender::RenderSphere(pos, 5.0f, Color(white[0], white[1], white[2], white[3]));
        }
    }
}

// ea: 0x0044D6F0 (inline COMDAT; static helper for VEH_UpdatePath)
static float VEH_TrackAngle(float tgt, float cur, float rate, int msec)
{
    if (tgt - cur > 180.0f)
    {
        do
        {
            tgt -= 360.0f;
        } while (tgt - cur > 180.0f);
    }
    if (tgt - cur < -180.0f)
    {
        do
        {
            tgt += 360.0f;
        } while (tgt - cur < -180.0f);
    }
    float delta = tgt - cur;
    float step = (msec * 0.001f) * delta * rate;
    if (fabsf(delta) <= 0.005f || fabsf(step) > 0.005f)
        return AngleNormalize180(tgt);
    return AngleNormalize180(cur + step);
}

// ea: 0x0047F8B0
void VEH_UpdatePath(Entity* ent, int msec)
{
    scr_vehicle_t* scr_vehicle = ent->scr_vehicle;
    vehicle_info_t* info = s_vehicleInfos[scr_vehicle->infoIdx];
    float startSpeed = ent->speed;
    bool prevSpeed = false;
    if (scr_vehicle->pathPos.nodeIdx < 0)
        return;
    if (scr_vehicle->pathPos.speed < 0.0f)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 5472;
        AeAssert::gCurrentExpr = "veh->pathPos.speed >= 0.0f";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (scr_vehicle->manualSpeed < 0.0f)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 5473;
        AeAssert::gCurrentExpr = "veh->manualSpeed >= 0.0f";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (scr_vehicle->manualMode != 0)
    {
        float tgtSpeed = (scr_vehicle->manualMode == 2)
                             ? scr_vehicle->pathPos.speed
                             : scr_vehicle->manualSpeed;
        if (tgtSpeed <= ent->speed)
        {
            ent->speed -= (msec * 0.001f) * scr_vehicle->manualAccel;
            if (tgtSpeed > ent->speed)
                ent->speed = tgtSpeed;
        }
        else
        {
            ent->speed += (msec * 0.001f) * scr_vehicle->manualAccel;
            if (ent->speed > tgtSpeed)
                ent->speed = tgtSpeed;
        }
        if (scr_vehicle->manualMode == 2 && ent->speed == tgtSpeed)
        {
            scr_vehicle->manualMode = 0;
        }
        else
        {
            if (ent->speed < 0.0f || tgtSpeed < 0.0f)
            {
                AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
                AeAssert::gCurrentFile =
                    "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
                AeAssert::gCurrentLine = 5500;
                AeAssert::gCurrentExpr =
                    "ent->speed >= 0.0f && tgtSpeed >= 0.0f";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert("Bad vehicle speed."))
                    __debugbreak();
            }
            if (tgtSpeed > 0.0f)
                scr_vehicle->manualTime += ent->speed / tgtSpeed;
            scr_vehicle->pathPos.speed = ent->speed;
        }
    }
    bool overrideSpeed;
    if (scr_vehicle->manualMode == 0)
    {
        ent->speed = scr_vehicle->pathPos.speed;
        if (ent->speed < 0.0f)
        {
            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
            AeAssert::gCurrentLine = 5513;
            AeAssert::gCurrentExpr = "ent->speed >= 0.0f";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("Bad vehicle speed."))
                __debugbreak();
        }
        if (scr_vehicle->pathPos.speed > 0.0f)
            scr_vehicle->manualTime +=
                ent->speed / scr_vehicle->pathPos.speed;
        overrideSpeed = false;
    }
    else
    {
        overrideSpeed = true;
    }
    int steppedMsec = msec;
    if (msec > 50)
    {
        int numSteps = (msec - 51) / 50 + 1;
        steppedMsec = msec - 50 * numSteps;
        do
        {
            if (G_VehUpdatePathPos(ent, &scr_vehicle->pathPos, overrideSpeed,
                                   50, scr_vehicle->waitNode) != 0)
                prevSpeed = true;
            --numSteps;
        } while (numSteps != 0);
    }
    if (G_VehUpdatePathPos(ent, &scr_vehicle->pathPos, overrideSpeed,
                           steppedMsec, scr_vehicle->waitNode) != 0)
        prevSpeed = true;
    if (scr_vehicle->mRBVeh == nullptr)
    {
        scr_vehicle->phys.origin.v.m128_f32[0] =
            scr_vehicle->pathPos.origin[0];
        scr_vehicle->phys.origin.v.m128_f32[1] =
            scr_vehicle->pathPos.origin[1];
        scr_vehicle->phys.origin.v.m128_f32[2] =
            scr_vehicle->pathPos.origin[2];
        scr_vehicle->phys.angles.v.m128_f32[0] =
            scr_vehicle->pathPos.angles[0];
        scr_vehicle->phys.angles.v.m128_f32[1] =
            scr_vehicle->pathPos.angles[1];
        scr_vehicle->phys.angles.v.m128_f32[2] =
            scr_vehicle->pathPos.angles[2];
        scr_vehicle->phys.angles.v.m128_f32[0] = VEH_TrackAngle(
            scr_vehicle->pathPos.angles[0],
            scr_vehicle->phys.prevAngles.v.m128_f32[0], 6.0f, msec);
        scr_vehicle->phys.angles.v.m128_f32[1] = VEH_TrackAngle(
            scr_vehicle->pathPos.angles[1],
            scr_vehicle->phys.prevAngles.v.m128_f32[1], 4.0f, msec);
        scr_vehicle->phys.angles.v.m128_f32[2] = VEH_TrackAngle(
            scr_vehicle->pathPos.angles[2],
            scr_vehicle->phys.prevAngles.v.m128_f32[2], 6.0f, msec);
        if (g_vehicleDebug.integer != 0)
        {
            math::Position3 tmp;
            tmp = native_to_cdl_pos3(scr_vehicle->pathPos.lookPos);
            const math::Position3* pos = &tmp;
            VEH_DebugBox(*pos, 8.0f, 0.0f, 1.0f, 1.0f);
        }
        float invMsec = 1.0f / (msec * 0.001f);
        scr_vehicle->phys.vel.v.m128_f32[0] =
            (scr_vehicle->phys.origin.v.m128_f32[0]
             - scr_vehicle->phys.prevOrigin.v.m128_f32[0]) * invMsec;
        scr_vehicle->phys.vel.v.m128_f32[1] =
            (scr_vehicle->phys.origin.v.m128_f32[1]
             - scr_vehicle->phys.prevOrigin.v.m128_f32[1]) * invMsec;
        scr_vehicle->phys.vel.v.m128_f32[2] =
            (scr_vehicle->phys.origin.v.m128_f32[2]
             - scr_vehicle->phys.prevOrigin.v.m128_f32[2]) * invMsec;
        AnglesSubtract(*(const math::Position3*)&scr_vehicle->phys.angles,
                       *(const math::Position3*)&scr_vehicle->phys.prevAngles,
                       *(math::Position3*)&scr_vehicle->phys.rotVel);
        scr_vehicle->phys.rotVel.v.m128_f32[0] *= invMsec;
        scr_vehicle->phys.rotVel.v.m128_f32[1] *= invMsec;
        scr_vehicle->phys.rotVel.v.m128_f32[2] *= invMsec;
        if (scr_vehicle->pathPos.endOfPath != 0)
        {
            ent->speed = 0.0f;
            scr_vehicle->phys.rotVel.v.m128_f32[0] = 0.0f;
            scr_vehicle->phys.rotVel.v.m128_f32[1] = 0.0f;
            scr_vehicle->phys.rotVel.v.m128_f32[2] = 0.0f;
            scr_vehicle->phys.vel.v.m128_f32[0] = 0.0f;
            scr_vehicle->phys.vel.v.m128_f32[1] = 0.0f;
            scr_vehicle->phys.vel.v.m128_f32[2] = 0.0f;
        }
    }
    if (prevSpeed && scr_vehicle->waitNode > -1)
    {
        Scr_Notify(ent, hash_const.reached_wait_node, 0);
        ++scr_vehicle->numWaitNotify;
        if (scr_vehicle->numWaitNotify > 10)
        {
            scr_vehicle->waitNode = -1;
            scr_vehicle->numWaitNotify = -1;
        }
    }
    if (scr_vehicle->pathPos.endOfPath != 0)
    {
        Scr_Notify(ent, hash_const.reached_end_node, 0);
        rb_vehicle* mRBVeh = (rb_vehicle*)scr_vehicle->mRBVeh;
        if (mRBVeh != nullptr && (mRBVeh->m_flags & 0x200) != 0)
            mRBVeh->end_path();
    }
    float speedFrac = ent->speed / info->engineSndSpeed;
    if (speedFrac < 0.0f)
        speedFrac = 0.0f;
    else if (speedFrac > 1.0f)
        speedFrac = 1.0f;
    scr_vehicle->engineSndLerp = speedFrac;
    scr_vehicle->idleSndLerp = 1.0f - speedFrac;
    if (scr_vehicle->waitSpeed >= 0.0f
        && ((scr_vehicle->waitSpeed >= startSpeed
             && ent->speed >= scr_vehicle->waitSpeed)
            || (startSpeed >= scr_vehicle->waitSpeed
                && scr_vehicle->waitSpeed >= ent->speed)))
    {
        Scr_Notify(ent, hash_const.reached_wait_speed, 0);
    }
}

// ea: 0x0046CB00
void Svcmd_VehicleList_f()
{
    int v0 = 0;
    for (int i = 0; i < level.MaxVehicles; ++i)
    {
        unsigned int v3 = s_vehicles[i].mEntity.mHandle.mVal & 0xFFF;
        if (v3 < 0x540
            && s_vehicles[i].mEntity.mHandle.mVal >> 12
                   == EntityHandleDb::sInst.mElements[v3].mKey
            && EntityHandleDb::sInst.mElements[v3].mObject != nullptr)
            ++v0;
    }
    G_Printf("vehicles %d\n", v0);
}

// ea: 0x0046D2B0
void VEH_NetAltWeaponStatus(Entity* ent, int status)
{
    scr_vehicle_t* scr_vehicle = ent->scr_vehicle;
    Entity* mObject = HandleDbToEnt(ent->r.mOwner);
    if (!EntityManager::sInst->IsLocalPlayer(mObject))
        scr_vehicle->seats[0].firing = (status != 0);
}

// ea: 0x0044DB90 (file-local)
static Handle VEH_StartWheelEffect(Entity* ent, unsigned int wheel_tag_hash,
                                   int mat)
{
    vehicle_info_t* info = s_vehicleInfos[ent->scr_vehicle->infoIdx];
    if (info->type == 2)
        wheel_tag_hash = 0;
    return PostEffectEventVehicleWheel(ent, (const char*)info,
                                       (EAction)0x29 /* kActionVEHICLE_HORN */,
                                       (ECollisionMaterial)mat,
                                       wheel_tag_hash);
}

// ea: 0x0045C7F0
void VEH_UpdateWheelParticleEffects(Entity* ent, int wheelIndex)
{
    scr_vehicle_t* scr_vehicle = ent->scr_vehicle;
    vehicle_info_t* info = s_vehicleInfos[scr_vehicle->infoIdx];
    if (info->maxSpeed <= 88.0f)
        return;
    if (scr_vehicle->engineSndLerp > s_sndLerpMin
        && scr_vehicle->phys.wheelSurfType[wheelIndex] != 0)
    {
        if (scr_vehicle->mWheel_ParticleEffectHandle[wheelIndex].mVal == 0)
        {
            scr_vehicle->mWheel_ParticleEffectHandle[wheelIndex] =
                VEH_StartWheelEffect(ent, s_wheelTagHashes[wheelIndex],
                                     scr_vehicle->phys.wheelSurfType[wheelIndex]);
        }
        float v8 = (ent->speed - 88.0f) / (info->maxSpeed - 88.0f);
        if (v8 < 0.0f)
            v8 = 0.0f;
        else if (v8 > 1.0f)
            v8 = 1.0f;
        float scaleValue2 = (float)(fabs(scr_vehicle->phys.rotVel.v.m128_f32[1])
                                    / info->rotRate);
        float v7 = (scr_vehicle->brakeSndLerp > 0.5f) ? 1.0f : 0.0f;
        float v9 = (v8 <= scaleValue2) ? scaleValue2 : v8;
        if (v9 <= v7)
            v8 = v7;
        else if (v8 <= scaleValue2)
            v8 = scaleValue2;
        EffectEventAdjustEffect_Scale(scr_vehicle->mWheel_ParticleEffectHandle[wheelIndex],
                                      "EmissionRate", rate * v8);
    }
    else
    {
        if (scr_vehicle->mWheel_ParticleEffectHandle[wheelIndex].mVal != 0)
            EffectEventStopEmitting(
                scr_vehicle->mWheel_ParticleEffectHandle[wheelIndex]);
        scr_vehicle->mWheel_ParticleEffectHandle[wheelIndex].mVal = 0;
    }
}

// ea: 0x0045C3A0
void VEH_DebugBox(const math::Position3& pos, float width, float r, float g, float b)
{
    float color[4] = {1.0f,
                      pos.v.m128_f32[0] + width * 0.5f,
                      pos.v.m128_f32[1] + width * 0.5f,
                      pos.v.m128_f32[2] + width * 0.5f};
    float mins[3] = {pos.v.m128_f32[0] - width * 0.5f,
                     pos.v.m128_f32[1] - width * 0.5f,
                     pos.v.m128_f32[2] - width * 0.5f};
    float boxColor[3] = {r, g, b};
    G_DebugBox(&color[1], mins, boxColor, 1, 0, 0);
}

// ea: 0x0045CA30
void VEH_FillFollowHistoryBuffer(scr_vehicle_t* veh)
{
    float facing[3];
    AnglesToForward(veh->phys.angles.v.m128_f32, facing);
    VectorNormalizeFast(facing);
    int v1 = 117;
    int v2 = -36;
    do
    {
        for (int b = 0; b < 10; ++b)
        {
            float dist = (float)(v2 + 18 - 18 * b);
            float* dst = veh->follow->positionHistory[v1 / 3 - b];
            dst[0] = veh->phys.origin.v.m128_f32[0] + dist * facing[0];
            dst[1] = veh->phys.origin.v.m128_f32[1] + dist * facing[1];
            dst[2] = veh->phys.origin.v.m128_f32[2] + dist * facing[2];
        }
        v2 -= 180;
        v1 -= 30;
    } while (v2 >= -576);
    veh->follow->numHistoryBufferEntries = 40;
    veh->follow->historyBufferFront = 0;
}

// ea: 0x0047F630
void VEH_UpdateOverHeat(Entity* self, int msec)
{
    scr_vehicle_t* scr_vehicle = self->scr_vehicle;
    vehicle_info_t* info = s_vehicleInfos[scr_vehicle->infoIdx];
    for (int i = 0; i < 11; ++i)
    {
        vehicleSeat_t& seat = scr_vehicle->seats[i];
        if (seat.gunMounted)
        {
            if (seat.weapon != 0)
            {
                weaponFileInfo_t* InfoForWeapon = BG_GetInfoForWeapon(seat.weapon);
                if (seat.heat < 1.0f)
                {
                    if (seat.overheating && seat.heat <= 0.5f)
                    {
                        Handle v9;
                        v9.mVal = seat.overheatEffect.mVal;
                        seat.overheating = false;
                        if (v9.mVal != 0)
                        {
                            EffectEventSys::sInst->StopEffect(
                                Handle(v9.mVal), false);
                            seat.overheatEffect.mVal = 0;
                        }
                    }
                }
                else
                {
                    seat.overheating = true;
                    PostEffectEventWeapon(self, InfoForWeapon->szInternalName,
                                          (EAction)0x37 /* kActionMax|kActionWEAPON_PICKUP */);
                    Scr_Notify(self, hash_const.overheated, 0);
                    Handle v8;
                    v8.mVal = seat.overheatEffect.mVal;
                    if (v8.mVal != 0)
                        EffectEventSys::sInst->AdjustEffect_Scale(
                            Handle(v8.mVal), "EmissionRate", 200.0f);
                }
                float heat = seat.heat;
                if (heat <= 0.0f)
                    seat.heat = 0.0f;
                else
                    seat.heat = heat - (msec * InfoForWeapon->fCooldownRate) * 0.001f;
                Handle v11;
                v11.mVal = seat.overheatEffect.mVal;
                if (seat.heat <= 0.25f)
                {
                    if (v11.mVal != 0)
                    {
                        EffectEventSys::sInst->StopEffect(
                            Handle(v11.mVal), false);
                        seat.overheatEffect.mVal = 0;
                    }
                }
                else
                {
                    if (v11.mVal == 0)
                        seat.overheatEffect =
                            PostEffectEventWeapon(self, InfoForWeapon->szInternalName,
                                                  (EAction)0x36 /* kActionMax|kActionWEAPON_LAST_SHOT_EJECT */);
                    float scale = (seat.heat - 0.25f) * 1.333333333333333f * emissionRate_0;
                    EffectEventSys::sInst->AdjustEffect_Scale(
                        Handle(seat.overheatEffect.mVal),
                        "EmissionRate", scale);
                }
            }
        }
        else if (info->type == 1 && i == 0)
        {
            if (scr_vehicle->seats[0].heat < 1.0f)
            {
                if (scr_vehicle->seats[0].overheating
                    && scr_vehicle->seats[0].heat <= 0.5f)
                    scr_vehicle->seats[0].overheating = false;
            }
            else
            {
                scr_vehicle->seats[0].overheating = true;
            }
            float heat = scr_vehicle->seats[0].heat;
            if (heat <= 0.0f)
                scr_vehicle->seats[0].heat = 0.0f;
            else
                scr_vehicle->seats[0].heat = heat - msec * 0.00025000001f;
        }
    }
}
