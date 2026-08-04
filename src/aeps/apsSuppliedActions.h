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

namespace math {
    class Dir3;
    class Mat43;
    class Vector4;
    struct Dir3_Packed { float x, y, z; };
}

// Forward decls of aeps core (declared in aeps core headers)
struct apsGroup;
struct apsEffect;
struct apsDomain;
struct apsPFD;

// ============================================================================
// apsVirtualBase / apsAction — from aeps core
// (declared here compact; full definitions belong in apsAction.h once ported)
// ============================================================================
struct apsVirtualBase {
    void* __vftable;
};

template <typename T>
struct apsArray {
    T*        mElements;
    short     mCapacity;
    short     mSize;
};

struct apsAction : apsVirtualBase {
    enum IterationStyle { eSource = 0, eAsync = 1, eSync = 2 };

    apsArray<float>      mParams;
    apsArray<apsDomain*> mDomains;
    IterationStyle       mIterationStyle;
    unsigned int         mRequiredParticleFields;

    apsAction();
    apsAction(int iNumParams, int iNumDomains, IterationStyle iIterationStyle,
              unsigned int iRequiredParticleFields);
    virtual ~apsAction() {}
    virtual void Act(unsigned char* iBegin, unsigned char* iEnd,
                     apsGroup* ioGroup, apsEffect* iEffect,
                     float iElapsedTime, float iTimeDelta) {}
    virtual unsigned int GetId() const { return 0; }
    virtual float GetVersion() const { return 0.0f; }

    void* GetParamAddr(int iIndex) const;
    apsDomain* GetDomain(int iIndex);
};

// ============================================================================
// apsQuaternion — 4-float quaternion used by Orient helpers
// ea: 0x004FD400 ctor / 0x004FD480 Set / 0x004FD4B0 operator*=
// ============================================================================
class apsQuaternion {
public:
    float x, y, z, w;

    apsQuaternion() : x(0.f), y(0.f), z(0.f), w(1.f) {}
    apsQuaternion(const math::Dir3& axisAngle);
    void Set(float ix, float iy, float iz, float iw);
    apsQuaternion& operator*=(const apsQuaternion& rhs);
};

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
};

struct apsBurstAction : apsSourceAction {
    apsBurstAction();                                               // ea: 0x00809B90
    virtual int GetEmissionCount(apsGroup& ioGroup, float iTimeDelta); // ea: 0x0080C640
};

struct apsRandomSpawnAction : apsSourceAction {
    apsRandomSpawnAction();                                         // ea: 0x00809BB0
    virtual int GetEmissionCount(apsGroup& ioGroup, float iTimeDelta); // ea: 0x0080C6C0
};

// ============================================================================
// Lifecycle — kill particles older than their max age
// ============================================================================
struct apsLifetimeAction : apsAction {
    apsLifetimeAction();                                            // ea: 0x00809BD0
    virtual void Act(unsigned char*, unsigned char*, apsGroup*, apsEffect*,
                     float, float);                                 // ea: 0x00809BF0
};

// ============================================================================
// Alpha fade family
// ============================================================================
struct apsAlphaFadeAction : apsAction {
    apsAlphaFadeAction();                                           // ea: 0x00809CE0
    virtual void Act(unsigned char*, unsigned char*, apsGroup*, apsEffect*,
                     float, float);                                 // ea: 0x00BFBC60
};

struct apsAlphaFadeInOutAction : apsAction {
    apsAlphaFadeInOutAction();                                      // ea: 0x00809D00
    virtual void Act(unsigned char*, unsigned char*, apsGroup*, apsEffect*,
                     float, float);                                 // ea: 0x00BFBEB0
};

struct apsRandomAlphaFadeInOutAction : apsAction {
    apsRandomAlphaFadeInOutAction();                                // ea: 0x00809D20
    virtual void Act(unsigned char*, unsigned char*, apsGroup*, apsEffect*,
                     float, float);                                 // ea: 0x00BFC160
};

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
};

struct apsLinearScaleSyncAction : apsLinearScaleAction {
    apsLinearScaleSyncAction();                                     // ea: 0x00809E10
private:
    virtual float GetScaleAmountForFrame(apsEffect& iEffect, float iTimeDelta) const;
};

struct apsExponentialScaleAction : apsAction {
    apsExponentialScaleAction();                                    // ea: 0x00809E30
    virtual void Act(unsigned char*, unsigned char*, apsGroup*, apsEffect*,
                     float, float);                                 // ea: 0x00BFC4F0
};

struct apsLinearScaleWidthAction : apsAction {
    apsLinearScaleWidthAction();                                    // ea: 0x00809E50
    virtual void Act(unsigned char*, unsigned char*, apsGroup*, apsEffect*,
                     float, float);                                 // ea: 0x00BFC5B0
};

struct apsExponentialScaleWidthAction : apsAction {
    apsExponentialScaleWidthAction();                               // ea: 0x00809E70
    virtual void Act(unsigned char*, unsigned char*, apsGroup*, apsEffect*,
                     float, float);                                 // ea: 0x00BFC650
};

struct apsLinearScaleHeightAction : apsAction {
    apsLinearScaleHeightAction();                                   // ea: 0x00809E90
    virtual void Act(unsigned char*, unsigned char*, apsGroup*, apsEffect*,
                     float, float);                                 // ea: 0x00BFC710
};

struct apsExponentialScaleHeightAction : apsAction {
    apsExponentialScaleHeightAction();                              // ea: 0x00809EB0
    virtual void Act(unsigned char*, unsigned char*, apsGroup*, apsEffect*,
                     float, float);                                 // ea: 0x00BFC7C0
};

// ============================================================================
// Move / force / velocity actions
// ============================================================================
struct apsMoveAction : apsAction {
    apsMoveAction();                                                // ea: 0x00809ED0
    virtual void Act(unsigned char*, unsigned char*, apsGroup*, apsEffect*,
                     float, float);                                 // ea: 0x00809EF0
};

struct apsObjectMoveAction : apsAction {
    apsObjectMoveAction();                                          // ea: 0x0080A0C0
    virtual void Act(unsigned char*, unsigned char*, apsGroup*, apsEffect*,
                     float, float);                                 // ea: 0x0080A0E0
};

struct apsPositionMoveAction : apsAction {
    apsPositionMoveAction();                                        // ea: 0x0080A400
    virtual void Act(unsigned char*, unsigned char*, apsGroup*, apsEffect*,
                     float, float);                                 // ea: 0x0080A420
};

struct apsMoveAtFixedVelocityAction : apsAction {
    apsMoveAtFixedVelocityAction();                                 // ea: 0x0080A550
    virtual void Act(unsigned char*, unsigned char*, apsGroup*, apsEffect*,
                     float, float);                                 // ea: 0x00BFC880
};

struct apsForceAction : apsAction {
    apsForceAction();                                               // ea: 0x0080A570
    virtual void Act(unsigned char*, unsigned char*, apsGroup*, apsEffect*,
                     float, float);                                 // ea: 0x00BFCAB0
};

struct apsColorShiftAction : apsAction {
    apsColorShiftAction();                                          // ea: 0x0080A590
    virtual void Act(unsigned char*, unsigned char*, apsGroup*, apsEffect*,
                     float, float);                                 // ea: 0x00BFCD20
};

struct apsVelocityDragAction : apsAction {
    apsVelocityDragAction();                                        // ea: 0x0080A5B0
    virtual void Act(unsigned char*, unsigned char*, apsGroup*, apsEffect*,
                     float, float);                                 // ea: 0x00BFD0E0
};

struct apsAngularVelocityDragAction : apsAction {
    apsAngularVelocityDragAction();                                 // ea: 0x0080A5D0
    virtual void Act(unsigned char*, unsigned char*, apsGroup*, apsEffect*,
                     float, float);                                 // ea: 0x00BFD1E0
};

struct apsVectorAngularVelocityDragAction : apsAction {
    apsVectorAngularVelocityDragAction();                           // ea: 0x0080A5F0
    virtual void Act(unsigned char*, unsigned char*, apsGroup*, apsEffect*,
                     float, float);                                 // ea: 0x00BFD2C0
};

struct apsWorldPlaneReflectionAction : apsAction {
    apsWorldPlaneReflectionAction();                                // ea: 0x0080A610
    virtual void Act(unsigned char*, unsigned char*, apsGroup*, apsEffect*,
                     float, float);                                 // ea: 0x00BFD3C0
};

struct apsUVAFrameAnimAction : apsAction {
    apsUVAFrameAnimAction();                                        // ea: 0x0080A630
    virtual void Act(unsigned char*, unsigned char*, apsGroup*, apsEffect*,
                     float, float);                                 // ea: 0x00BFD7B0
};

// ============================================================================
// Angle-tracking actions
// ============================================================================
struct apsAngleTrackVelocityAction : apsAction {
    apsAngleTrackVelocityAction();                                  // ea: 0x0080A650
    virtual void Act(unsigned char*, unsigned char*, apsGroup*, apsEffect*,
                     float, float);                                 // ea: 0x0080A830
};

struct apsAngleTrackElementXAction : apsAction {
    apsAngleTrackElementXAction();                                  // ea: 0x0080AB00
    virtual void Act(unsigned char*, unsigned char*, apsGroup*, apsEffect*,
                     float, float);                                 // ea: 0x00BFDA20
};

struct apsAngleTrackElementYAction : apsAction {
    apsAngleTrackElementYAction();                                  // ea: 0x0080AB20
    virtual void Act(unsigned char*, unsigned char*, apsGroup*, apsEffect*,
                     float, float);                                 // ea: 0x00BFDB60
};

struct apsAngleTrackElementZAction : apsAction {
    apsAngleTrackElementZAction();                                  // ea: 0x0080AB40
    virtual void Act(unsigned char*, unsigned char*, apsGroup*, apsEffect*,
                     float, float);                                 // ea: 0x00BFDCA0
};

// ============================================================================
// Attractors / physics-like fields
// ============================================================================
struct apsPointAttractorAction : apsAction {
    apsPointAttractorAction();                                      // ea: 0x0080AB60
    virtual void Act(unsigned char*, unsigned char*, apsGroup*, apsEffect*,
                     float, float);                                 // ea: 0x00BFDDE0
};

struct apsLineAttractorAction : apsAction {
    apsLineAttractorAction();                                       // ea: 0x0080AB80
    virtual void Act(unsigned char*, unsigned char*, apsGroup*, apsEffect*,
                     float, float);                                 // ea: 0x00BFE0D0
};

struct apsDecayLineAttractorAction : apsAction {
    apsDecayLineAttractorAction();                                  // ea: 0x0080ABA0
    virtual void Act(unsigned char*, unsigned char*, apsGroup*, apsEffect*,
                     float, float);                                 // ea: 0x00BFE550
};

struct apsKappaTauAction : apsAction {
    apsKappaTauAction();                                            // ea: 0x0080ABC0
    virtual void Act(unsigned char*, unsigned char*, apsGroup*, apsEffect*,
                     float, float);                                 // ea: 0x00BFEA00
};

// ============================================================================
// Spawn actions (nested effects)
// ============================================================================
struct apsSpawnAction : apsAction {
    apsSpawnAction();                                               // ea: 0x0080ABE0
    virtual void Act(unsigned char*, unsigned char*, apsGroup*, apsEffect*,
                     float, float);                                 // ea: 0x00BFF150
};

struct apsSpawnOnDeathAction : apsAction {
    apsSpawnOnDeathAction();                                        // ea: 0x0080B100
    virtual void Act(unsigned char*, unsigned char*, apsGroup*, apsEffect*,
                     float, float);                                 // ea: 0x00BFF3B0
};

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
};

// ============================================================================
// Env collision (raycast against world geometry)
// ============================================================================
struct apsEnvCollideAction : apsAction {
    apsEnvCollideAction();                                          // ea: 0x0080B320
    virtual void Act(unsigned char*, unsigned char*, apsGroup*, apsEffect*,
                     float, float);                                 // ea: 0x0080B340
};

#endif // COD3_AEPS_APSSUPPLIEDACTIONS_H
