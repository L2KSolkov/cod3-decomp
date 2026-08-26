// Particle/effects compatibility translation unit. Implementations live in
// the owning APS sources.

#include <stdio.h>

// apsShrimpRenderer virtuals.
#include "apsShrimpRenderer.h"
// apsRenderNode / apsSimpleMeshRenderer / apsBounds / apsClient.
#include "apsRenderNode.h"
#include "apsGroup.h"
#include "apsSimpleMeshRenderer.h"
#include "apsInternal.h"
apsClient::~apsClient() {}

// apsEffect / apsCommon free artifacts.
#include "apsEffect.h"
#include "apsCommon.h"
