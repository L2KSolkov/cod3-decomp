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
// apsRenderNode / apsSimpleMeshRenderer / apsBounds / apsClient (stubs)
#include "apsRenderNode.h"
#include "apsGroup.h"
#include "apsSimpleMeshRenderer.h"
#include "apsInternal.h"
void apsRenderNode::Render() {}
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
