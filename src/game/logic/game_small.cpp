// ============================================================================
// game_small.cpp -- smallest game.o helpers (toggles, zoom, prone thunk)
// Batch 1: 7 smallest (16b + 6*32b) -- BG_CheckProne, ToggleRenderGeom etc.
// ============================================================================

#include "game/logic/g_local.h"

// ---------------------------------------------------------------------------
// Toggle / Zoom helpers -- ea: 0x6119F0 etc
// Original decompile shows CGBankManager* return but map says void (YAXXZ).
// Provide void to match map / audit.
// g_local.h defines CGBankManager::sInst as void* at +0x04 mDebugRenderMode
// ---------------------------------------------------------------------------
// ea: 0x006119F0
void ToggleRenderGeom() {
    ((CGBankManager*)CGBankManager::sInst)->mDebugRenderMode ^= 1;
}
// ea: 0x00611A10
void ToggleRenderPerf() {
    ((CGBankManager*)CGBankManager::sInst)->mDebugRenderMode ^= 2;
}
// ea: 0x00611A30
void ToggleGraph() {
    ((CGBankManager*)CGBankManager::sInst)->mDebugRenderMode ^= 4;
}
// ea: 0x00611A50
void ZoomIn() {
    ((CGBankManager*)CGBankManager::sInst)->scale -= 25.0f;
}
// ea: 0x00611A70
void ZoomOut() {
    ((CGBankManager*)CGBankManager::sInst)->scale += 25.0f;
}

// ---------------------------------------------------------------------------
// BG_CheckProne thunk -- ea: 0x6146F0 (16 bytes)
// Forwards to BG_CheckProneValid (4608b, ported separately).
// ---------------------------------------------------------------------------
// ea: 0x006146F0
int BG_CheckProne(
    DbLinkedHandle<EntityHandleDb, Entity> passEntity,
    const math::Position3& vPos,
    float fSize,
    float fHeight,
    float fYaw,
    float* pfTorsoHeight,
    float* pfTorsoPitch,
    float* pfWaistPitch,
    int bAlreadyProne,
    int bOnGround,
    const math::Dir3& vGroundNormal,
    void (*traceFunc)(trace_t*, const math::Position3&, const math::Position3&, const math::Position3&, const math::Position3&, const collision_context_t&),
    void (*boxTraceFunc)(trace_t*, const math::Position3&, const math::Position3&, const math::Position3&, const math::Position3&, const collision_context_t&),
    int (*pointcontents)(const math::Position3&, const collision_context_t&),
    proneCheckType_t proneCheckType,
    float prone_feet_dist)
{
    return BG_CheckProneValid(
        passEntity, &vPos, fSize, fHeight, fYaw,
        pfTorsoHeight, pfTorsoPitch, pfWaistPitch,
        bAlreadyProne, bOnGround, &vGroundNormal,
        (void (*)(trace_t*, const math::Position3*, const math::Position3*, const math::Position3*, const math::Position3*, const collision_context_t&))traceFunc,
        (void (*)(trace_t*, const math::Position3*, const math::Position3*, const math::Position3*, const math::Position3*, const collision_context_t&))boxTraceFunc,
        (int (*)(const math::Position3*, const collision_context_t&))pointcontents,
        proneCheckType, prone_feet_dist);
}
