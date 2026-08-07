// ============================================================================
// apsSuppliedActions — built-in particle action library (88 funcs)
// Source: c:\cod\code\tl\aeps\source\apsSuppliedActions.cpp
// Objects in aeps_xboxr particle/effects system ("APS")
//
// Each class here is one particle-modifier "Action" invoked per-frame across
// a group's particle range. Base classes:
//   apsVirtualBase          -> vtable-only root
//     apsAction             -> mParams[], mDomains[], iterationStyle, requiredFields
//       apsSourceAction     -> spawns new particles from 16 domains (position/velocity/etc.)
//         apsBurstAction    -> one-shot burst
//         apsRandomSpawnAction -> spawn on random schedule
//       (leaf actions ...)  -> alpha fade, scale, move, color shift, drag, attract, spawn, etc.
// ============================================================================
#ifndef COD3_AEPS_APSSUPPLIEDACTIONS_H
#define COD3_AEPS_APSSUPPLIEDACTIONS_H

#include <cstdint>

#include "apsMath.h"
#include "apsAction.h"  // apsVirtualBase, apsArray<T>, apsAction (real definitions)
#include "apsRetrieveVtable.h"

namespace math {
    struct Dir3_Packed { float x, y, z; };
}

// Forward decls of aeps core (declared in aeps core headers)
struct apsGroup;
struct apsEffect;
struct apsDomain;
struct apsPFD;

// ============================================================================
// apsQuaternion (defined in apsMath.h). apsSuppliedActions.o owns:
//   ctor(const Dir3&)  ea: 0x00809400
//   Set                 ea: 0x00809480
//   operator+=          ea: 0x008094B0
// ============================================================================

// ============================================================================
// Source actions — spawn new particles from configured domains
// ============================================================================
struct apsSourceAction : apsAction {
    apsSourceAction();                                              // ea: 0x00809B40
protected:
    apsSourceAction(int iNumParams, int iNumDomains,
                    unsigned int iRequiredParticleFields);          // ea: 0x00809B60
public:
    virtual void Act(unsigned char* iBegin, unsigned char* iEnd,
                     apsGroup* ioGroup, apsEffect* iEffect,
                     float iElapsedTime, float iTimeDelta);         // ea: 0x0080B710
    virtual int GetEmissionCount(apsGroup& ioGroup, float iTimeDelta); // ea: 0x0080C5C0

        unsigned int GetId() const { return 1400005441; }
    float GetVersion() const { return 1.0f; }
    APS_DECLARE_RETRIEVE(apsSourceAction, apsAction())};

struct apsBurstAction : apsSourceAction {
    apsBurstAction();                                               // ea: 0x00809B90
    virtual int GetEmissionCount(apsGroup& ioGroup, float iTimeDelta); // ea: 0x0080C640

        unsigned int GetId() const { return 1114796916; }
    float GetVersion() const { return 1.0f; }
    APS_DECLARE_RETRIEVE(apsBurstAction, apsSourceAction())};

struct apsRandomSpawnAction : apsSourceAction {
    apsRandomSpawnAction();                                         // ea: 0x00809BB0
    virtual int GetEmissionCount(apsGroup& ioGroup, float iTimeDelta); // ea: 0x0080C6C0

        unsigned int GetId() const { return 1381199982; }
    float GetVersion() const { return 1.0f; }
    APS_DECLARE_RETRIEVE(apsRandomSpawnAction, apsSourceAction())};

// ============================================================================
// Lifecycle — kill particles older than their max age
// ============================================================================
struct apsLifetimeAction : apsAction {
    apsLifetimeAction();                                            // ea: 0x00809BD0
    virtual void Act(unsigned char*, unsigned char*, apsGroup*, apsEffect*,
                     float, float);                                 // ea: 0x00809BF0

        unsigned int GetId() const { return 1281975909; }
    float GetVersion() const { return 1.0f; }
    APS_DECLARE_RETRIEVE(apsLifetimeAction, apsAction())};

// ============================================================================
// Alpha fade family
// ============================================================================
struct apsAlphaFadeAction : apsAction {
    apsAlphaFadeAction();                                           // ea: 0x00809CE0
    virtual void Act(unsigned char*, unsigned char*, apsGroup*, apsEffect*,
                     float, float);                                 // ea: 0x00BFBC60

        unsigned int GetId() const { return 1097614948; }
    float GetVersion() const { return 1.0f; }
    APS_DECLARE_RETRIEVE(apsAlphaFadeAction, apsAction())};

struct apsAlphaFadeInOutAction : apsAction {
    apsAlphaFadeInOutAction();                                      // ea: 0x00809D00
    virtual void Act(unsigned char*, unsigned char*, apsGroup*, apsEffect*,
                     float, float);                                 // ea: 0x00BFBEB0

        unsigned int GetId() const { return 1095125327; }
    float GetVersion() const { return 1.0f; }
    APS_DECLARE_RETRIEVE(apsAlphaFadeInOutAction, apsAction())};

struct apsRandomAlphaFadeInOutAction : apsAction {
    apsRandomAlphaFadeInOutAction();                                // ea: 0x00809D20
    virtual void Act(unsigned char*, unsigned char*, apsGroup*, apsEffect*,
                     float, float);                                 // ea: 0x00BFC160

        unsigned int GetId() const { return 1380337999; }
    float GetVersion() const { return 1.0f; }
    APS_DECLARE_RETRIEVE(apsRandomAlphaFadeInOutAction, apsAction())};

// ============================================================================
// Scale actions
// ============================================================================
struct apsLinearScaleAction : apsAction {
    apsLinearScaleAction();                                         // ea: 0x00809D40
protected:
    apsLinearScaleAction(int iNumParams, int iNumDomains,
                         unsigned int iRequiredParticleFields);     // ea: 0x00809D60
public:
    virtual void Act(unsigned char*, unsigned char*, apsGroup*, apsEffect*,
                     float, float);                                 // ea: 0x00809D90
private:
    virtual float GetScaleAmountForFrame(apsEffect& iEffect, float iTimeDelta) const;
public:
    unsigned int GetId() const { return 1282298723; }
    float GetVersion() const { return 1.0f; }
    APS_DECLARE_RETRIEVE(apsLinearScaleAction, apsAction())};

struct apsLinearScaleSyncAction : apsLinearScaleAction {
    apsLinearScaleSyncAction();                                     // ea: 0x00809E10
private:
    virtual float GetScaleAmountForFrame(apsEffect& iEffect, float iTimeDelta) const;
public:
    unsigned int GetId() const { return 1282626413; }
    float GetVersion() const { return 1.0f; }
    APS_DECLARE_RETRIEVE(apsLinearScaleSyncAction, apsLinearScaleAction())};

struct apsExponentialScaleAction : apsAction {
    apsExponentialScaleAction();                                    // ea: 0x00809E30
    virtual void Act(unsigned char*, unsigned char*, apsGroup*, apsEffect*,
                     float, float);                                 // ea: 0x00BFC4F0

        unsigned int GetId() const { return 1165513571; }
    float GetVersion() const { return 1.0f; }
    APS_DECLARE_RETRIEVE(apsExponentialScaleAction, apsAction())};

struct apsLinearScaleWidthAction : apsAction {
    apsLinearScaleWidthAction();                                    // ea: 0x00809E50
    virtual void Act(unsigned char*, unsigned char*, apsGroup*, apsEffect*,
                     float, float);                                 // ea: 0x00BFC5B0

        unsigned int GetId() const { return 1280533335; }
    float GetVersion() const { return 1.0f; }
    APS_DECLARE_RETRIEVE(apsLinearScaleWidthAction, apsAction())};

struct apsExponentialScaleWidthAction : apsAction {
    apsExponentialScaleWidthAction();                               // ea: 0x00809E70
    virtual void Act(unsigned char*, unsigned char*, apsGroup*, apsEffect*,
                     float, float);                                 // ea: 0x00BFC650

        unsigned int GetId() const { return 1163092823; }
    float GetVersion() const { return 1.0f; }
    APS_DECLARE_RETRIEVE(apsExponentialScaleWidthAction, apsAction())};

struct apsLinearScaleHeightAction : apsAction {
    apsLinearScaleHeightAction();                                   // ea: 0x00809E90
    virtual void Act(unsigned char*, unsigned char*, apsGroup*, apsEffect*,
                     float, float);                                 // ea: 0x00BFC710

        unsigned int GetId() const { return 1280533320; }
    float GetVersion() const { return 1.0f; }
    APS_DECLARE_RETRIEVE(apsLinearScaleHeightAction, apsAction())};

struct apsExponentialScaleHeightAction : apsAction {
    apsExponentialScaleHeightAction();                              // ea: 0x00809EB0
    virtual void Act(unsigned char*, unsigned char*, apsGroup*, apsEffect*,
                     float, float);                                 // ea: 0x00BFC7C0

        unsigned int GetId() const { return 1163092808; }
    float GetVersion() const { return 1.0f; }
    APS_DECLARE_RETRIEVE(apsExponentialScaleHeightAction, apsAction())};

// ============================================================================
// Move / force / velocity actions
// ============================================================================
struct apsMoveAction : apsAction {
    apsMoveAction();                                                // ea: 0x00809ED0
    virtual void Act(unsigned char*, unsigned char*, apsGroup*, apsEffect*,
                     float, float);                                 // ea: 0x00809EF0

        unsigned int GetId() const { return 1299150437; }
    float GetVersion() const { return 1.0f; }
    APS_DECLARE_RETRIEVE(apsMoveAction, apsAction())};

struct apsObjectMoveAction : apsAction {
    apsObjectMoveAction();                                          // ea: 0x0080A0C0
    virtual void Act(unsigned char*, unsigned char*, apsGroup*, apsEffect*,
                     float, float);                                 // ea: 0x0080A0E0

        unsigned int GetId() const { return 1330474870; }
    float GetVersion() const { return 1.0f; }
    APS_DECLARE_RETRIEVE(apsObjectMoveAction, apsAction())};

struct apsPositionMoveAction : apsAction {
    apsPositionMoveAction();                                        // ea: 0x0080A400
    virtual void Act(unsigned char*, unsigned char*, apsGroup*, apsEffect*,
                     float, float);                                 // ea: 0x0080A420

        unsigned int GetId() const { return 1349733750; }
    float GetVersion() const { return 1.0f; }
    APS_DECLARE_RETRIEVE(apsPositionMoveAction, apsAction())};

struct apsMoveAtFixedVelocityAction : apsAction {
    apsMoveAtFixedVelocityAction();                                 // ea: 0x0080A550
    virtual void Act(unsigned char*, unsigned char*, apsGroup*, apsEffect*,
                     float, float);                                 // ea: 0x00BFC880

        unsigned int GetId() const { return 1296463958; }
    float GetVersion() const { return 1.0f; }
    APS_DECLARE_RETRIEVE(apsMoveAtFixedVelocityAction, apsAction())};

struct apsForceAction : apsAction {
    apsForceAction();                                               // ea: 0x0080A570
    virtual void Act(unsigned char*, unsigned char*, apsGroup*, apsEffect*,
                     float, float);                                 // ea: 0x00BFCAB0

        unsigned int GetId() const { return 1181708899; }
    float GetVersion() const { return 1.0f; }
    APS_DECLARE_RETRIEVE(apsForceAction, apsAction())};

struct apsColorShiftAction : apsAction {
    apsColorShiftAction();                                          // ea: 0x0080A590
    virtual void Act(unsigned char*, unsigned char*, apsGroup*, apsEffect*,
                     float, float);                                 // ea: 0x00BFCD20

        unsigned int GetId() const { return 1131172712; }
    float GetVersion() const { return 1.0f; }
    APS_DECLARE_RETRIEVE(apsColorShiftAction, apsAction())};

struct apsVelocityDragAction : apsAction {
    apsVelocityDragAction();                                        // ea: 0x0080A5B0
    virtual void Act(unsigned char*, unsigned char*, apsGroup*, apsEffect*,
                     float, float);                                 // ea: 0x00BFD0E0

        unsigned int GetId() const { return 1449935986; }
    float GetVersion() const { return 1.0f; }
    APS_DECLARE_RETRIEVE(apsVelocityDragAction, apsAction())};

struct apsAngularVelocityDragAction : apsAction {
    apsAngularVelocityDragAction();                                 // ea: 0x0080A5D0
    virtual void Act(unsigned char*, unsigned char*, apsGroup*, apsEffect*,
                     float, float);                                 // ea: 0x00BFD1E0

        unsigned int GetId() const { return 1096172658; }
    float GetVersion() const { return 1.0f; }
    APS_DECLARE_RETRIEVE(apsAngularVelocityDragAction, apsAction())};

struct apsVectorAngularVelocityDragAction : apsAction {
    apsVectorAngularVelocityDragAction();                           // ea: 0x0080A5F0
    virtual void Act(unsigned char*, unsigned char*, apsGroup*, apsEffect*,
                     float, float);                                 // ea: 0x00BFD2C0

        unsigned int GetId() const { return 1447122500; }
    float GetVersion() const { return 1.0f; }
    APS_DECLARE_RETRIEVE(apsVectorAngularVelocityDragAction, apsAction())};

struct apsWorldPlaneReflectionAction : apsAction {
    apsWorldPlaneReflectionAction();                                // ea: 0x0080A610
    virtual void Act(unsigned char*, unsigned char*, apsGroup*, apsEffect*,
                     float, float);                                 // ea: 0x00BFD3C0

        unsigned int GetId() const { return 1464881766; }
    float GetVersion() const { return 1.0f; }
    APS_DECLARE_RETRIEVE(apsWorldPlaneReflectionAction, apsAction())};

struct apsUVAFrameAnimAction : apsAction {
    apsUVAFrameAnimAction();                                        // ea: 0x0080A630
    virtual void Act(unsigned char*, unsigned char*, apsGroup*, apsEffect*,
                     float, float);                                 // ea: 0x00BFD7B0

        unsigned int GetId() const { return 1431717490; }
    float GetVersion() const { return 1.0f; }
    APS_DECLARE_RETRIEVE(apsUVAFrameAnimAction, apsAction())};

// ============================================================================
// Angle-tracking actions
// ============================================================================
struct apsAngleTrackVelocityAction : apsAction {
    apsAngleTrackVelocityAction();                                  // ea: 0x0080A650
    virtual void Act(unsigned char*, unsigned char*, apsGroup*, apsEffect*,
                     float, float);                                 // ea: 0x0080A830

        unsigned int GetId() const { return 1097754452; }
    float GetVersion() const { return 1.0f; }
    APS_DECLARE_RETRIEVE(apsAngleTrackVelocityAction, apsAction())};

struct apsAngleTrackElementXAction : apsAction {
    apsAngleTrackElementXAction();                                  // ea: 0x0080AB00
    virtual void Act(unsigned char*, unsigned char*, apsGroup*, apsEffect*,
                     float, float);                                 // ea: 0x00BFDA20

        unsigned int GetId() const { return 1096041816; }
    float GetVersion() const { return 1.0f; }
    APS_DECLARE_RETRIEVE(apsAngleTrackElementXAction, apsAction())};

struct apsAngleTrackElementYAction : apsAction {
    apsAngleTrackElementYAction();                                  // ea: 0x0080AB20
    virtual void Act(unsigned char*, unsigned char*, apsGroup*, apsEffect*,
                     float, float);                                 // ea: 0x00BFDB60

        unsigned int GetId() const { return 1096041817; }
    float GetVersion() const { return 1.0f; }
    APS_DECLARE_RETRIEVE(apsAngleTrackElementYAction, apsAction())};

struct apsAngleTrackElementZAction : apsAction {
    apsAngleTrackElementZAction();                                  // ea: 0x0080AB40
    virtual void Act(unsigned char*, unsigned char*, apsGroup*, apsEffect*,
                     float, float);                                 // ea: 0x00BFDCA0

        unsigned int GetId() const { return 1096041818; }
    float GetVersion() const { return 1.0f; }
    APS_DECLARE_RETRIEVE(apsAngleTrackElementZAction, apsAction())};

// ============================================================================
// Attractors / physics-like fields
// ============================================================================
struct apsPointAttractorAction : apsAction {
    apsPointAttractorAction();                                      // ea: 0x0080AB60
    virtual void Act(unsigned char*, unsigned char*, apsGroup*, apsEffect*,
                     float, float);                                 // ea: 0x00BFDDE0

        unsigned int GetId() const { return 1346466930; }
    float GetVersion() const { return 1.0f; }
    APS_DECLARE_RETRIEVE(apsPointAttractorAction, apsAction())};

struct apsLineAttractorAction : apsAction {
    apsLineAttractorAction();                                       // ea: 0x0080AB80
    virtual void Act(unsigned char*, unsigned char*, apsGroup*, apsEffect*,
                     float, float);                                 // ea: 0x00BFE0D0

        unsigned int GetId() const { return 1279358066; }
    float GetVersion() const { return 1.0f; }
    APS_DECLARE_RETRIEVE(apsLineAttractorAction, apsAction())};

struct apsDecayLineAttractorAction : apsAction {
    apsDecayLineAttractorAction();                                  // ea: 0x0080ABA0
    virtual void Act(unsigned char*, unsigned char*, apsGroup*, apsEffect*,
                     float, float);                                 // ea: 0x00BFE550

        unsigned int GetId() const { return 1145848180; }
    float GetVersion() const { return 1.0f; }
    APS_DECLARE_RETRIEVE(apsDecayLineAttractorAction, apsAction())};

struct apsKappaTauAction : apsAction {
    apsKappaTauAction();                                            // ea: 0x0080ABC0
    virtual void Act(unsigned char*, unsigned char*, apsGroup*, apsEffect*,
                     float, float);                                 // ea: 0x00BFEA00

        unsigned int GetId() const { return 1263821173; }
    float GetVersion() const { return 1.0f; }
    APS_DECLARE_RETRIEVE(apsKappaTauAction, apsAction())};

// ============================================================================
// Spawn actions (nested effects)
// ============================================================================
struct apsSpawnAction : apsAction {
    apsSpawnAction();                                               // ea: 0x0080ABE0
    virtual void Act(unsigned char*, unsigned char*, apsGroup*, apsEffect*,
                     float, float);                                 // ea: 0x00BFF150

        unsigned int GetId() const { return 1399865699; }
    float GetVersion() const { return 1.0f; }
    APS_DECLARE_RETRIEVE(apsSpawnAction, apsAction())};

struct apsSpawnOnDeathAction : apsAction {
    apsSpawnOnDeathAction();                                        // ea: 0x0080B100
    virtual void Act(unsigned char*, unsigned char*, apsGroup*, apsEffect*,
                     float, float);                                 // ea: 0x00BFF3B0

        unsigned int GetId() const { return 1399866469; }
    float GetVersion() const { return 1.0f; }
    APS_DECLARE_RETRIEVE(apsSpawnOnDeathAction, apsAction())};

// ============================================================================
// Trajectory (spline-following)
// ============================================================================
struct apsTrajectoryAction : apsAction {
    apsTrajectoryAction();                                          // ea: 0x0080B120
    virtual void Act(unsigned char*, unsigned char*, apsGroup*, apsEffect*,
                     float, float);                                 // ea: 0x00BFF5E0
private:
    unsigned int GetSplineInfo(float*& oPoints, float& oT, float& oScale,
                               math::Dir3& oAxis, math::Dir3& oAxisRate);

public:
    unsigned int GetId() const { return 1416782186; }
    float GetVersion() const { return 1.0f; }
    APS_DECLARE_RETRIEVE(apsTrajectoryAction, apsAction())};

// ============================================================================
// Env collision (raycast against world geometry)
// ============================================================================
struct apsEnvCollideAction : apsAction {
    apsEnvCollideAction();                                          // ea: 0x0080B320
    virtual void Act(unsigned char*, unsigned char*, apsGroup*, apsEffect*,
                     float, float);                                 // ea: 0x0080B340

        unsigned int GetId() const { return 1165378412; }
    float GetVersion() const { return 1.0f; }
    APS_DECLARE_RETRIEVE(apsEnvCollideAction, apsAction())};

#endif // COD3_AEPS_APSSUPPLIEDACTIONS_H
