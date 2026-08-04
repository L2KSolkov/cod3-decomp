// ============================================================================
// apsSuppliedActions.cpp — built-in particle actions (88 funcs)
// Reconstructed from codmp_xboxr.xbe (release build /O2)
// Source: c:\cod\code\tl\aeps\source\apsSuppliedActions.cpp
//
// Port strategy: stubs matching nsl_xboxr / nal_xboxr precedent.
//   - Class ABI is real: vtable slots, layout, ctor→base delegation.
//   - Method bodies are stubs pending a live particle-system harness.
//   - Anonymous-namespace helpers reconstructed inline where trivial.
//
// The ctor→apsAction(numParams,numDomains,style,requiredFields) pattern is
// verbatim from IDA — the (numParams,numDomains,requiredFields) triples are
// content-load-bearing (they drive per-particle memory layout via apsPFD),
// so every ctor reproduces its exact numbers.
// ============================================================================
#include "apsSuppliedActions.h"

// ============================================================================
// apsAction base — trivial stubs (real impl lives in aeps core once ported)
// ============================================================================
apsAction::apsAction()
    : mIterationStyle(eAsync), mRequiredParticleFields(0) {
    mParams.mElements = 0;  mParams.mCapacity = 0;  mParams.mSize = 0;
    mDomains.mElements = 0; mDomains.mCapacity = 0; mDomains.mSize = 0;
}
apsAction::apsAction(int, int, IterationStyle iStyle, unsigned int iRequired)
    : mIterationStyle(iStyle), mRequiredParticleFields(iRequired) {
    mParams.mElements = 0;  mParams.mCapacity = 0;  mParams.mSize = 0;
    mDomains.mElements = 0; mDomains.mCapacity = 0; mDomains.mSize = 0;
}
void*      apsAction::GetParamAddr(int) const { return 0; }
apsDomain* apsAction::GetDomain(int)         { return 0; }

// ============================================================================
// apsQuaternion
// ============================================================================
apsQuaternion::apsQuaternion(const math::Dir3&) : x(0.f), y(0.f), z(0.f), w(1.f) {}
void apsQuaternion::Set(float ix, float iy, float iz, float iw) {
    x = ix; y = iy; z = iz; w = iw;
}
apsQuaternion& apsQuaternion::operator*=(const apsQuaternion&) { return *this; }

// ============================================================================
// Source actions
// (numParams, numDomains, requiredFields) triples from IDA disasm.
// ============================================================================
apsSourceAction::apsSourceAction()
    : apsAction(5, 16, eSource, 0x200u) {}
apsSourceAction::apsSourceAction(int iNumParams, int iNumDomains,
                                 unsigned int iRequiredParticleFields)
    : apsAction(iNumParams, iNumDomains, eSource,
                iRequiredParticleFields | 0x200u) {}
void apsSourceAction::Act(unsigned char*, unsigned char*, apsGroup*, apsEffect*, float, float) {}
int  apsSourceAction::GetEmissionCount(apsGroup&, float) { return 0; }

apsBurstAction::apsBurstAction()
    : apsSourceAction(5, 16, 0x200u) {}
int  apsBurstAction::GetEmissionCount(apsGroup&, float) { return 0; }

apsRandomSpawnAction::apsRandomSpawnAction()
    : apsSourceAction(5, 17, 0x200u) {}
int  apsRandomSpawnAction::GetEmissionCount(apsGroup&, float) { return 0; }

// ============================================================================
// Lifetime — kills particles whose age > maxage.
// Full body is a real 25-line loop; not needed until particles actually spawn.
// ============================================================================
apsLifetimeAction::apsLifetimeAction()
    : apsAction(2, 0, eAsync, 0x8000600u) {}
void apsLifetimeAction::Act(unsigned char*, unsigned char*, apsGroup*, apsEffect*, float, float) {}

// ============================================================================
// Alpha fade family
// ============================================================================
apsAlphaFadeAction::apsAlphaFadeAction()
    : apsAction(2, 0, eAsync, 0x10u) {}
void apsAlphaFadeAction::Act(unsigned char*, unsigned char*, apsGroup*, apsEffect*, float, float) {}

apsAlphaFadeInOutAction::apsAlphaFadeInOutAction()
    : apsAction(5, 0, eAsync, 0x8008610u) {}
void apsAlphaFadeInOutAction::Act(unsigned char*, unsigned char*, apsGroup*, apsEffect*, float, float) {}

apsRandomAlphaFadeInOutAction::apsRandomAlphaFadeInOutAction()
    : apsAction(4, 0, eAsync, 0x8008610u) {}
void apsRandomAlphaFadeInOutAction::Act(unsigned char*, unsigned char*, apsGroup*, apsEffect*, float, float) {}

// ============================================================================
// Scale actions
// ============================================================================
apsLinearScaleAction::apsLinearScaleAction()
    : apsAction(3, 0, eAsync, 2u) {}
apsLinearScaleAction::apsLinearScaleAction(int iNumParams, int iNumDomains,
                                           unsigned int iRequiredParticleFields)
    : apsAction(iNumParams, iNumDomains, eAsync,
                iRequiredParticleFields | 2u) {}
void  apsLinearScaleAction::Act(unsigned char*, unsigned char*, apsGroup*, apsEffect*, float, float) {}
float apsLinearScaleAction::GetScaleAmountForFrame(apsEffect&, float) const { return 0.0f; }

apsLinearScaleSyncAction::apsLinearScaleSyncAction()
    : apsLinearScaleAction(5, 0, 2u) {}
float apsLinearScaleSyncAction::GetScaleAmountForFrame(apsEffect&, float) const { return 0.0f; }

apsExponentialScaleAction::apsExponentialScaleAction()
    : apsAction(3, 0, eAsync, 2u) {}
void apsExponentialScaleAction::Act(unsigned char*, unsigned char*, apsGroup*, apsEffect*, float, float) {}

apsLinearScaleWidthAction::apsLinearScaleWidthAction()
    : apsAction(3, 0, eAsync, 4u) {}
void apsLinearScaleWidthAction::Act(unsigned char*, unsigned char*, apsGroup*, apsEffect*, float, float) {}

apsExponentialScaleWidthAction::apsExponentialScaleWidthAction()
    : apsAction(3, 0, eAsync, 4u) {}
void apsExponentialScaleWidthAction::Act(unsigned char*, unsigned char*, apsGroup*, apsEffect*, float, float) {}

apsLinearScaleHeightAction::apsLinearScaleHeightAction()
    : apsAction(3, 0, eAsync, 0x80u) {}
void apsLinearScaleHeightAction::Act(unsigned char*, unsigned char*, apsGroup*, apsEffect*, float, float) {}

apsExponentialScaleHeightAction::apsExponentialScaleHeightAction()
    : apsAction(3, 0, eAsync, 0x80u) {}
void apsExponentialScaleHeightAction::Act(unsigned char*, unsigned char*, apsGroup*, apsEffect*, float, float) {}

// ============================================================================
// Move / force / velocity actions
// ============================================================================
apsMoveAction::apsMoveAction()
    : apsAction(2, 0, eAsync, 0x14040u) {}
void apsMoveAction::Act(unsigned char*, unsigned char*, apsGroup*, apsEffect*, float, float) {}

apsObjectMoveAction::apsObjectMoveAction()
    : apsAction(2, 0, eAsync, 0x24021u) {}
void apsObjectMoveAction::Act(unsigned char*, unsigned char*, apsGroup*, apsEffect*, float, float) {}

apsPositionMoveAction::apsPositionMoveAction()
    : apsAction(2, 0, eAsync, 0x4001u) {}
void apsPositionMoveAction::Act(unsigned char*, unsigned char*, apsGroup*, apsEffect*, float, float) {}

apsMoveAtFixedVelocityAction::apsMoveAtFixedVelocityAction()
    : apsAction(6, 0, eAsync, 0x1u) {}
void apsMoveAtFixedVelocityAction::Act(unsigned char*, unsigned char*, apsGroup*, apsEffect*, float, float) {}

apsForceAction::apsForceAction()
    : apsAction(6, 0, eAsync, 0x4000u) {}
void apsForceAction::Act(unsigned char*, unsigned char*, apsGroup*, apsEffect*, float, float) {}

apsColorShiftAction::apsColorShiftAction()
    : apsAction(10, 0, eAsync, 0x608u) {}
void apsColorShiftAction::Act(unsigned char*, unsigned char*, apsGroup*, apsEffect*, float, float) {}

apsVelocityDragAction::apsVelocityDragAction()
    : apsAction(3, 0, eAsync, 0x4000u) {}
void apsVelocityDragAction::Act(unsigned char*, unsigned char*, apsGroup*, apsEffect*, float, float) {}

apsAngularVelocityDragAction::apsAngularVelocityDragAction()
    : apsAction(3, 0, eAsync, 0x10000u) {}
void apsAngularVelocityDragAction::Act(unsigned char*, unsigned char*, apsGroup*, apsEffect*, float, float) {}

apsVectorAngularVelocityDragAction::apsVectorAngularVelocityDragAction()
    : apsAction(3, 0, eAsync, 0x20000u) {}
void apsVectorAngularVelocityDragAction::Act(unsigned char*, unsigned char*, apsGroup*, apsEffect*, float, float) {}

apsWorldPlaneReflectionAction::apsWorldPlaneReflectionAction()
    : apsAction(2, 1, eAsync, 0x40u) {}
void apsWorldPlaneReflectionAction::Act(unsigned char*, unsigned char*, apsGroup*, apsEffect*, float, float) {}

apsUVAFrameAnimAction::apsUVAFrameAnimAction()
    : apsAction(3, 0, eAsync, 0x20000u) {}
void apsUVAFrameAnimAction::Act(unsigned char*, unsigned char*, apsGroup*, apsEffect*, float, float) {}

// ============================================================================
// Angle-tracking actions
// ============================================================================
apsAngleTrackVelocityAction::apsAngleTrackVelocityAction()
    : apsAction(0, 0, eAsync, 0x40u) {}
void apsAngleTrackVelocityAction::Act(unsigned char*, unsigned char*, apsGroup*, apsEffect*, float, float) {}

apsAngleTrackElementXAction::apsAngleTrackElementXAction()
    : apsAction(0, 0, eAsync, 0x1000u) {}
void apsAngleTrackElementXAction::Act(unsigned char*, unsigned char*, apsGroup*, apsEffect*, float, float) {}

apsAngleTrackElementYAction::apsAngleTrackElementYAction()
    : apsAction(0, 0, eAsync, 0x1000u) {}
void apsAngleTrackElementYAction::Act(unsigned char*, unsigned char*, apsGroup*, apsEffect*, float, float) {}

apsAngleTrackElementZAction::apsAngleTrackElementZAction()
    : apsAction(0, 0, eAsync, 0x1000u) {}
void apsAngleTrackElementZAction::Act(unsigned char*, unsigned char*, apsGroup*, apsEffect*, float, float) {}

// ============================================================================
// Attractor actions
// ============================================================================
apsPointAttractorAction::apsPointAttractorAction()
    : apsAction(3, 1, eAsync, 0x40u) {}
void apsPointAttractorAction::Act(unsigned char*, unsigned char*, apsGroup*, apsEffect*, float, float) {}

apsLineAttractorAction::apsLineAttractorAction()
    : apsAction(3, 2, eAsync, 0x40u) {}
void apsLineAttractorAction::Act(unsigned char*, unsigned char*, apsGroup*, apsEffect*, float, float) {}

apsDecayLineAttractorAction::apsDecayLineAttractorAction()
    : apsAction(4, 2, eAsync, 0x40u) {}
void apsDecayLineAttractorAction::Act(unsigned char*, unsigned char*, apsGroup*, apsEffect*, float, float) {}

apsKappaTauAction::apsKappaTauAction()
    : apsAction(2, 0, eAsync, 0x40u) {}
void apsKappaTauAction::Act(unsigned char*, unsigned char*, apsGroup*, apsEffect*, float, float) {}

// ============================================================================
// Spawn (nested effects)
// ============================================================================
apsSpawnAction::apsSpawnAction()
    : apsAction(3, 0, eAsync, 0x14040u) {}
void apsSpawnAction::Act(unsigned char*, unsigned char*, apsGroup*, apsEffect*, float, float) {}

apsSpawnOnDeathAction::apsSpawnOnDeathAction()
    : apsAction(2, 0, eAsync, 0x14040u) {}
void apsSpawnOnDeathAction::Act(unsigned char*, unsigned char*, apsGroup*, apsEffect*, float, float) {}

// ============================================================================
// Trajectory (spline)
// ============================================================================
apsTrajectoryAction::apsTrajectoryAction()
    : apsAction(4, 0, eAsync, 0x14040u) {}
void         apsTrajectoryAction::Act(unsigned char*, unsigned char*, apsGroup*, apsEffect*, float, float) {}
unsigned int apsTrajectoryAction::GetSplineInfo(float*&, float&, float&,
                                                math::Dir3&, math::Dir3&) { return 0; }

// ============================================================================
// Env-collide (world raycast)
// ============================================================================
apsEnvCollideAction::apsEnvCollideAction()
    : apsAction(3, 0, eAsync, 0x14040u) {}
void apsEnvCollideAction::Act(unsigned char*, unsigned char*, apsGroup*, apsEffect*, float, float) {}

// ============================================================================
// File-scope global — gCheckSplineData (0x014CFF50 in .data)
// ea: 0x00BFA640 CheckSplineData(float*) references this.
// ============================================================================
int gCheckSplineData = 0;
