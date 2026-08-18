// AUTO-GENERATED STUBS — Particle/effects system (aeps_xboxr)
// 0 non-inline functions to port
// When ported, functions move from here to their real .cpp files.

#include <stdio.h>

#define COD3_UNIMPLEMENTED(lib) \
    fprintf(stderr, "COD3 UNIMPLEMENTED: %s\n", lib)

void __cod3_stub_aeps(void) {
    COD3_UNIMPLEMENTED("aeps");
}

// apsShrimpRenderer virtuals (apsShrimpRenderer.o; stubs, port later)
#include "apsShrimpRenderer.h"
float apsShrimpRenderer::GetChanceToRemove() const
{
    return 0.0f;
}
bool apsShrimpRenderer::GetMeshRadius(float& oRadius) const
{
    (void)oRadius;
    return false;
}

// apsRenderNode / apsSimpleMeshRenderer / apsBounds / apsClient (stubs)
#include "apsRenderNode.h"
#include "apsGroup.h"
#include "apsSimpleMeshRenderer.h"
#include "apsInternal.h"
void apsRenderNode::Render() {}
float apsSimpleMeshRenderer::GetChanceToRemove() const
{
    return 0.0f;
}
apsSphere apsBounds::Sphere() const
{
    apsSphere s = {};
    return s;
}
apsClient::~apsClient() {}
bool apsClient::GetLightInfoAtPosition(const math::Dir3& iPosition,
                                       apsLight::LightInfo& oInfo)
{
    (void)iPosition; (void)oInfo;
    return false;
}

// apsEffect / apsCommon free artifacts (render.o; stubs, port later)
#include "apsEffect.h"
#include "apsCommon.h"
