// ============================================================================
// ai_stubs.cpp - mp_actors.o AI stubs (release MP build bodies)
// Generated from IDA decompiles; all bodies are the release-build stubs.
// ============================================================================

#include "game/logic/g_local.h"
#include "game/actor_types.h"
#include "game/game_types.h"
#include "game/client_types.h"
#include <stdint.h>

// ea: 0x0077BE50  (?FreeIndex@sentient_info_array@@QAEXH@Z)
    void sentient_info_array::FreeIndex(int)
{
}


// ea: 0x0077BE60  (?Actor_NextActor@@YIPAUactor_s@@PAU1@H@Z)
    actor_s* __fastcall Actor_NextActor(actor_s*, int)
{
    return nullptr;
}

// ea: 0x0077BE70  (?Actor_FirstActor@@YIPAUactor_s@@H@Z)
    actor_s* __fastcall Actor_FirstActor(int)
{
    return nullptr;
}

// ea: 0x0077BE80  (?Actor_GetEnt@@YIPAVEntity@@PAUactor_s@@@Z)
    Entity* __fastcall Actor_GetEnt(actor_s*)
{
    return nullptr;
}

// ea: 0x0077BE90  (?Actor_CheckArmor@@YAHPAUactor_s@@HH@Z)
    int Actor_CheckArmor(actor_s*, int, int)
{
    return 0;
}

// ea: 0x0077BEA0  (?G_FlushCorpses@@YAXXZ)
    void G_FlushCorpses(void)
{
}

// ea: 0x0077BEB0  (?Actor_Pain@@YAXPAVEntity@@0HQBMH1W4hitLocation_t@@@Z)
    void Actor_Pain(Entity*, Entity*, int, const float* const, int, const float* const, hitLocation_t)
{
}

// ea: 0x0077BEC0  (?Actor_Die@@YAXPAVEntity@@00HHHQBM1W4hitLocation_t@@@Z)
    void Actor_Die(Entity*, Entity*, Entity*, int, int, int, const float* const, const float* const, hitLocation_t)
{
}

// ea: 0x0077BED0  (?Actor_EntInfo@@YAXPAVEntity@@@Z)
    void Actor_EntInfo(Entity*)
{
}

// ea: 0x0077BEE0  (?Actor_Touch@@YAXPAVEntity@@0H@Z)
    void Actor_Touch(Entity*, Entity*, int)
{
}

// ea: 0x0077BEF0  (?Actor_Think@@YAXPAVEntity@@H@Z)
    void Actor_Think(Entity*, int)
{
}

// ea: 0x0077BF00  (?Actor_SetDesiredBodyAngle@@YIXPAUai_orient_t@@M@Z)
    void __fastcall Actor_SetDesiredBodyAngle(ai_orient_t*, float)
{
}

// ea: 0x0077BF10  (?Actor_DissociateGrenade@@YIXPAVEntity@@@Z)
    void __fastcall Actor_DissociateGrenade(Entity*)
{
}

// ea: 0x0077BF20  (?Actor_HitTarget@@YAXPAUweaponParms@@QAM1@Z)
    void Actor_HitTarget(weaponParms*, float* const, float* const)
{
}

// ea: 0x0077BF30  (?Actor_MissTarget@@YAXPAUweaponParms@@QAM1@Z)
    void Actor_MissTarget(weaponParms*, float* const, float* const)
{
}

// ea: 0x0077BF40  (?Actor_KillBroAnimScriptImmediate@@YIXPAUactor_s@@@Z)
    void __fastcall Actor_KillBroAnimScriptImmediate(actor_s*)
{
}

// ea: 0x0077BF50  (?Actor_DissociateEntity@@YAXPAUactor_s@@PAVEntity@@@Z)
    void Actor_DissociateEntity(actor_s*, Entity*)
{
}

// ea: 0x0077BF60  (?Actor_GrenadeBounced@@YIXPAVEntity@@0@Z)
    void __fastcall Actor_GrenadeBounced(Entity*, Entity*)
{
}

// ea: 0x0077BF70  (?Actor_CanAttackAll@@YIXPAUactor_s@@@Z)
    void __fastcall Actor_CanAttackAll(actor_s*)
{
}

// ea: 0x0077BF80  (?G_DropActorSpawnersToFloor@@YAXXZ)
    void G_DropActorSpawnersToFloor(void)
{
}

// ea: 0x0077BF90  (?Actor_OrientCorpseToGround@@YAXPAVEntity@@H@Z)
    void Actor_OrientCorpseToGround(Entity*, int)
{
}

// ea: 0x0077BFA0  (?Actor_CorpseThink@@YAXPAVEntity@@H@Z)
    void Actor_CorpseThink(Entity*, int)
{
}

// ea: 0x0077BFB0  (?Actor_Grenade_CheckGrenadeHintToss@@YIHPAUactor_s@@QBM1QAM_N@Z)
    int __fastcall Actor_Grenade_CheckGrenadeHintToss(actor_s*, const float* const, const float* const, float* const, bool)
{
    return 0;
}

// ea: 0x0077BFC0  (?Actor_NodeClaimRevoked@@YIXPAUactor_s@@@Z)
    void __fastcall Actor_NodeClaimRevoked(actor_s*)
{
}

// ea: 0x0077BFD0  (?Actor_CanSeePointEx@@YIMPAUactor_s@@QBMMMV?$DbLinkedHandle@VEntityHandleDb@@VEntity@@@@@Z)
    float __fastcall Actor_CanSeePointEx(actor_s*, const float* const, float, float, DbLinkedHandle<EntityHandleDb, Entity>)
{
    return 0.0f;
}

// ea: 0x0077BFE0  (?Actor_BroadcastPointEvent@@YIXPAVEntity@@W4ai_event_t@@HABVPosition3@math@@M@Z)
    void __fastcall Actor_BroadcastPointEvent(Entity*, ai_event_t, int, const math::Position3&, float)
{
}

// ea: 0x0077BFF0  (?Actor_BroadcastLineEvent@@YIXPAVEntity@@W4ai_event_t@@HQBM2M@Z)
    void __fastcall Actor_BroadcastLineEvent(Entity*, ai_event_t, int, const float* const, const float* const, float)
{
}

// ea: 0x0077C000  (?Actor_DissociateSentient@@YAXPAUactor_s@@PAUsentient_s@@W4team_t@@@Z)
    void Actor_DissociateSentient(actor_s*, sentient_s*, team_t)
{
}

// ea: 0x0077C010  (?Actor_Alloc@@YAPAUactor_s@@W4TPakId@@@Z)
    actor_s* Actor_Alloc(TPakId)
{
    return nullptr;
}

// ea: 0x0077C020  (?ActorCorpse_Free@@YAXPAVEntity@@@Z)
    void ActorCorpse_Free(Entity*)
{
}

// ea: 0x0077C030  (?Actor_Free@@YAXPAUactor_s@@@Z)
    void Actor_Free(actor_s*)
{
}

// ea: 0x0077C040  (?Actor_FreeExpendable@@YAXXZ)
    void Actor_FreeExpendable(void)
{
}

// ea: 0x0077C050  (?G_InitActors@@YAXXZ)
    void G_InitActors(void)
{
}

// ea: 0x0077C060  (?G_GetActorIndex@@YAHPAUactor_s@@@Z)
    int G_GetActorIndex(actor_s*)
{
    return 0;
}

// ea: 0x0077C070  (?G_GetActorCorpseIndex@@YAHPAVEntity@@@Z)
    int G_GetActorCorpseIndex(Entity*)
{
    return 0;
}

// ea: 0x0077C080  (?Actor_GetCorpseAngles@@YAXV?$DbLinkedHandle@VEntityHandleDb@@VEntity@@@@HABVPosition3@math@@MPAM22@Z)
    void Actor_GetCorpseAngles(DbLinkedHandle<EntityHandleDb, Entity>, int, const math::Position3&, float, float*, float*, float*)
{
}

// ea: 0x0077C090  (?G_GetActorAnimTree@@YAPAVXAnimTree@@PAUactor_s@@@Z)
    XAnimTree* G_GetActorAnimTree(actor_s*)
{
    return nullptr;
}

// ea: 0x0077C0A0  (?G_GetActorCorpseAnimTree@@YAPAVXAnimTree@@PAVEntity@@@Z)
    XAnimTree* G_GetActorCorpseAnimTree(Entity*)
{
    return nullptr;
}

// ea: 0x0077C0B0  (?Actor_SetDefaultState@@YAXPAUactor_s@@@Z)
    void Actor_SetDefaultState(actor_s*)
{
}

// ea: 0x0077C0C0  (?Actor_SetDefaults@@YAXPAUactor_s@@@Z)
    void Actor_SetDefaults(actor_s*)
{
}

// ea: 0x0077C0D0  (?Actor_FinishSpawning@@YAXPAUactor_s@@@Z)
    void Actor_FinishSpawning(actor_s*)
{
}

// ea: 0x0077C0E0  (?Actor_FinishSpawningAll@@YAXXZ)
    void Actor_FinishSpawningAll(void)
{
}

// ea: 0x0077C0F0  (?Actor_InvalidateCache@@YAXPAUactor_s@@@Z)
    void Actor_InvalidateCache(actor_s*)
{
}

// ea: 0x0077C100  (?Actor_BroadcastTeamEvent@@YIXPAUsentient_s@@W4ai_event_t@@@Z)
    void __fastcall Actor_BroadcastTeamEvent(sentient_s*, ai_event_t)
{
}

// ea: 0x0077C110  (?Actor_GetSpreadMetric@@YAXPAUactor_s@@MPAM1PAUweaponFileInfo_t@@QAM3@Z)
    void Actor_GetSpreadMetric(actor_s*, float, float*, float*, weaponFileInfo_t*, float* const, float* const)
{
}

// ea: 0x0077C120  (?Actor_PointSatisfiesGoal@@YIHPAUactor_s@@QBMM@Z)
    int __fastcall Actor_PointSatisfiesGoal(actor_s*, const float* const, float)
{
    return 0;
}

// ea: 0x0077C130  (?Actor_NodeSatisfiesGoal@@YIHPAUactor_s@@PBUPathNode@PathNodes@@@Z)
    int __fastcall Actor_NodeSatisfiesGoal(actor_s*, const PathNodes::PathNode*)
{
    return 0;
}

// ea: 0x0077C140  (?Actor_UnclaimedNode@@YIHPAUactor_s@@PBUPathNode@PathNodes@@@Z)
    int __fastcall Actor_UnclaimedNode(actor_s*, const PathNodes::PathNode*)
{
    return 0;
}

// ea: 0x0077C150  (?Actor_IsAtGoal@@YIHPAUactor_s@@W4goalRadiusCheck@@@Z)
    int __fastcall Actor_IsAtGoal(actor_s*, goalRadiusCheck)
{
    return 0;
}

// ea: 0x0077C160  (?Actor_FindPathToGoal@@YIXPAUactor_s@@MHHH@Z)
    void __fastcall Actor_FindPathToGoal(actor_s*, float, int, int, int)
{
}

// ea: 0x0077C170  (?Actor_ExistsPathToGoal@@YIHPAUactor_s@@QAM@Z)
    int __fastcall Actor_ExistsPathToGoal(actor_s*, float* const)
{
    return 0;
}

// ea: 0x0077C180  (?Actor_FlagEnemyUnattackable@@YIXPAUactor_s@@@Z)
    void __fastcall Actor_FlagEnemyUnattackable(actor_s*)
{
}

// ea: 0x0077C190  (?Actor_FlagEnemyUnattackableForDuration@@YIXPAUactor_s@@H@Z)
    void __fastcall Actor_FlagEnemyUnattackableForDuration(actor_s*, int)
{
}

// ea: 0x0077C1A0  (?Actor_ForgetEnemy@@YIXPAUactor_s@@@Z)
    void __fastcall Actor_ForgetEnemy(actor_s*)
{
}

// ea: 0x0077C1B0  (?Actor_CaresAboutInfo@@YIHPAUactor_s@@PAUsentient_s@@@Z)
    int __fastcall Actor_CaresAboutInfo(actor_s*, sentient_s*)
{
    return 0;
}

// ea: 0x0077C1C0  (?Actor_UpdateThreat@@YIXPAUactor_s@@H@Z)
    void __fastcall Actor_UpdateThreat(actor_s*, int)
{
}

// ea: 0x0077C1D0  (?Actor_FillWeaponParms@@YAXPAUactor_s@@PAUweaponParms@@@Z)
    void Actor_FillWeaponParms(actor_s*, weaponParms*)
{
}

// ea: 0x0077C1E0  (?Actor_CanSeePointFrom@@YIMPAUactor_s@@QBM1MV?$DbLinkedHandle@VEntityHandleDb@@VEntity@@@@@Z)
    float __fastcall Actor_CanSeePointFrom(actor_s*, const float* const, const float* const, float, DbLinkedHandle<EntityHandleDb, Entity>)
{
    return 0.0f;
}

// ea: 0x0077C1F0  (?Actor_CanSeePoint@@YIMPAUactor_s@@QBM@Z)
    float __fastcall Actor_CanSeePoint(actor_s*, const float* const)
{
    return 0.0f;
}

// ea: 0x0077C200  (?Actor_CanSeeEntityPoint@@YIMPAUactor_s@@QBMPBVEntity@@@Z)
    float __fastcall Actor_CanSeeEntityPoint(actor_s*, const float* const, const Entity*)
{
    return 0.0f;
}

// ea: 0x0077C210  (?Actor_CanSeeEntity@@YIMPAUactor_s@@PBVEntity@@@Z)
    float __fastcall Actor_CanSeeEntity(actor_s*, const Entity*)
{
    return 0.0f;
}

// ea: 0x0077C220  (?Actor_CanSeeEntityNoFov@@YIMPAUactor_s@@PBVEntity@@@Z)
    float __fastcall Actor_CanSeeEntityNoFov(actor_s*, const Entity*)
{
    return 0.0f;
}

// ea: 0x0077C230  (?Actor_CanSeeEntityEx@@YIMPAUactor_s@@PBVEntity@@MM@Z)
    float __fastcall Actor_CanSeeEntityEx(actor_s*, const Entity*, float, float)
{
    return 0.0f;
}

// ea: 0x0077C240  (?Actor_CanSeeSentient@@YIMPAUactor_s@@PAUsentient_s@@H@Z)
    float __fastcall Actor_CanSeeSentient(actor_s*, sentient_s*, int)
{
    return 0.0f;
}

// ea: 0x0077C250  (?Actor_CanSeeSentientNoFov@@YIMPAUactor_s@@PAUsentient_s@@@Z)
    float __fastcall Actor_CanSeeSentientNoFov(actor_s*, sentient_s*)
{
    return 0.0f;
}

// ea: 0x0077C260  (?Actor_CanSeeEnemy@@YIMPAUactor_s@@H@Z)
    float __fastcall Actor_CanSeeEnemy(actor_s*, int)
{
    return 0.0f;
}

// ea: 0x0077C270  (?Actor_CanSeeEnemyNoFov@@YIMPAUactor_s@@@Z)
    float __fastcall Actor_CanSeeEnemyNoFov(actor_s*)
{
    return 0.0f;
}

// ea: 0x0077C280  (?Actor_CanSeeGoal@@YIMPAUactor_s@@H@Z)
    float __fastcall Actor_CanSeeGoal(actor_s*, int)
{
    return 0.0f;
}

// ea: 0x0077C290  (?Actor_CanSeeSentientEx@@YIMPAUactor_s@@PAUsentient_s@@MMH@Z)
    float __fastcall Actor_CanSeeSentientEx(actor_s*, sentient_s*, float, float, int)
{
    return 0.0f;
}

// ea: 0x0077C2A0  (?Actor_CanSeeEnemyEx@@YIMPAUactor_s@@MMH@Z)
    float __fastcall Actor_CanSeeEnemyEx(actor_s*, float, float, int)
{
    return 0.0f;
}

// ea: 0x0077C2B0  (?Actor_CanSeeGoalEx@@YIMPAUactor_s@@MMH@Z)
    float __fastcall Actor_CanSeeGoalEx(actor_s*, float, float, int)
{
    return 0.0f;
}

// ea: 0x0077C2C0  (?Actor_CanShootFrom@@YIHPAUactor_s@@QBM1@Z)
    int __fastcall Actor_CanShootFrom(actor_s*, const float* const, const float* const)
{
    return 0;
}

// ea: 0x0077C2D0  (?Actor_CanShootEnemy@@YIHPAUactor_s@@@Z)
    int __fastcall Actor_CanShootEnemy(actor_s*)
{
    return 0;
}

// ea: 0x0077C2E0  (?Actor_CanHear@@YIHPAUactor_s@@QBMMM@Z)
    int __fastcall Actor_CanHear(actor_s*, const float* const, float, float)
{
    return 0;
}

// ea: 0x0077C2F0  (?Actor_UpdateSight@@YIXPAUactor_s@@@Z)
    void __fastcall Actor_UpdateSight(actor_s*)
{
}

// ea: 0x0077C300  (?Actor_Cover_PickAttackScript@@YI?BHPAUactor_s@@PAUPathNode@PathNodes@@PAUsentient_s@@@Z)
    const int __fastcall Actor_Cover_PickAttackScript(actor_s*, PathNodes::PathNode*, sentient_s*)
{
    return 0;
}

// ea: 0x0077C310  (?Actor_Cover_CanAttack@@YI?BHPAUactor_s@@PAUPathNode@PathNodes@@PAUsentient_s@@PAPAUscr_animscript_t@@_N@Z)
    const int __fastcall Actor_Cover_CanAttack(actor_s*, PathNodes::PathNode*, sentient_s*, scr_animscript_t**, bool)
{
    return 0;
}

// ea: 0x0077C320  (?Actor_Cover_FindCoverNearSelf@@YI?BHPAUactor_s@@H@Z)
    const int __fastcall Actor_Cover_FindCoverNearSelf(actor_s*, int)
{
    return 0;
}

// ea: 0x0077C330  (?Actor_Cover_FindCoverNearPoint@@YI?BHPAUactor_s@@QBM@Z)
    const int __fastcall Actor_Cover_FindCoverNearPoint(actor_s*, const float* const)
{
    return 0;
}

// ea: 0x0077C340  (?Actor_Cover_FindCoverFromPoint@@YI?BHPAUactor_s@@QBMM@Z)
    int __fastcall Actor_Cover_FindCoverFromPoint(actor_s*, const float* const, float)
{
    return 0;
}

// ea: 0x0077C350  (?Actor_Cover_IsValidCover@@YI?BHPAUactor_s@@PAUPathNode@PathNodes@@_N@Z)
    int __fastcall Actor_Cover_IsValidCover(actor_s*, PathNodes::PathNode*, bool)
{
    return 0;
}

// ea: 0x0077C360  (?Actor_Cover_FindNewCover@@YI?BHPAUactor_s@@@Z)
    int __fastcall Actor_Cover_FindNewCover(actor_s*)
{
    return 0;
}

// ea: 0x0077C370  (?Actor_PreThink@@YIXPAUactor_s@@H@Z)
    void __fastcall Actor_PreThink(actor_s*, int)
{
}

// ea: 0x0077C380  (?Actor_PostThink@@YIXPAUactor_s@@H@Z)
    void __fastcall Actor_PostThink(actor_s*, int)
{
}

// ea: 0x0077C390  (?Actor_IsStateOnStack@@YIHPAUactor_s@@W4ai_state_e@@@Z)
    int __fastcall Actor_IsStateOnStack(actor_s*, ai_state_e)
{
    return 0;
}

// ea: 0x0077C3A0  (?Actor_IsCurState@@YIHPAUactor_s@@W4ai_state_e@@@Z)
    int __fastcall Actor_IsCurState(actor_s*, ai_state_e)
{
    return 0;
}

// ea: 0x0077C3B0  (?Actor_PendingTransitionTo@@YIHPAUactor_s@@W4ai_state_e@@@Z)
    int __fastcall Actor_PendingTransitionTo(actor_s*, ai_state_e)
{
    return 0;
}

// ea: 0x0077C3C0  (?Actor_SetDoingMelee@@YIXPAUactor_s@@_N@Z)
    void __fastcall Actor_SetDoingMelee(actor_s*, bool)
{
}

// ea: 0x0077C3D0  (?Actor_SetMeleeInteractable@@YIXPAUactor_s@@_N@Z)
    void __fastcall Actor_SetMeleeInteractable(actor_s*, bool)
{
}

// ea: 0x0077C3E0  (?Actor_SetState@@YIXPAUactor_s@@W4ai_state_e@@@Z)
    void __fastcall Actor_SetState(actor_s*, ai_state_e)
{
}

// ea: 0x0077C3F0  (?Actor_PushState@@YIHPAUactor_s@@W4ai_state_e@@@Z)
    int __fastcall Actor_PushState(actor_s*, ai_state_e)
{
    return 0;
}

// ea: 0x0077C400  (?Actor_PopState@@YIXPAUactor_s@@@Z)
    void __fastcall Actor_PopState(actor_s*)
{
}

// ea: 0x0077C410  (?Actor_ForceState@@YIXPAUactor_s@@W4ai_state_e@@@Z)
    void __fastcall Actor_ForceState(actor_s*, ai_state_e)
{
}

// ea: 0x0077C420  (?VisCache_Flush@@YIXPAUvis_cache_t@@@Z)
    void __fastcall VisCache_Flush(vis_cache_t*)
{
}

// ea: 0x0077C430  (?VisCache_Copy@@YIXPAUvis_cache_t@@PBU1@@Z)
    void __fastcall VisCache_Copy(vis_cache_t*, const vis_cache_t*)
{
}

// ea: 0x0077C440  (?VisCache_Update@@YIXPAUvis_cache_t@@HM@Z)
    void __fastcall VisCache_Update(vis_cache_t*, int, float)
{
}

// ea: 0x0077C450  (?Actor_GetEyePosition@@YIXPAUactor_s@@QAM@Z)
    void __fastcall Actor_GetEyePosition(actor_s*, float* const)
{
}

// ea: 0x0077C460  (?Actor_GetEyeDirection@@YIXPAUactor_s@@QAM@Z)
    void __fastcall Actor_GetEyeDirection(actor_s*, float* const)
{
}

// ea: 0x0077C470  (?Actor_GetEyeOffset@@YIXPAUactor_s@@QAM@Z)
    void __fastcall Actor_GetEyeOffset(actor_s*, float* const)
{
}

// ea: 0x0077C480  (?Actor_GetMuzzleInfo@@YIHPAUactor_s@@QAM1@Z)
    int __fastcall Actor_GetMuzzleInfo(actor_s*, float* const, float* const)
{
    return 0;
}

// ea: 0x0077C490  (?Actor_UpdateLastKnownPos@@YIXPAUactor_s@@PAUsentient_s@@@Z)
    void __fastcall Actor_UpdateLastKnownPos(actor_s*, sentient_s*)
{
}

// ea: 0x0077C4A0  (?Actor_Grenade_CheckMinimumEnergyToss@@YIHPAUactor_s@@QBM1QAM@Z)
    int __fastcall Actor_Grenade_CheckMinimumEnergyToss(actor_s*, const float* const, const float* const, float* const)
{
    return 0;
}

// ea: 0x0077C4B0  (?Actor_Grenade_CheckMaximumEnergyToss@@YIHPAUactor_s@@QBM1HQAM@Z)
    int __fastcall Actor_Grenade_CheckMaximumEnergyToss(actor_s*, const float* const, const float* const, int, float* const)
{
    return 0;
}

// ea: 0x0077C4C0  (?Actor_Grenade_IsPointSafe@@YIHPAUactor_s@@QBM@Z)
    int __fastcall Actor_Grenade_IsPointSafe(actor_s*, const float* const)
{
    return 0;
}

// ea: 0x0077C4D0  (?Actor_Grenade_IsValidTrajectory@@YIHPAUactor_s@@QBM11@Z)
    int __fastcall Actor_Grenade_IsValidTrajectory(actor_s*, const float* const, const float* const, const float* const)
{
    return 0;
}

// ea: 0x0077C4E0  (?Actor_HasPath@@YIHPBUactor_s@@@Z)
    int __fastcall Actor_HasPath(const actor_s*)
{
    return 0;
}

// ea: 0x0077C4F0  (?Actor_InitPath@@YIXPAUactor_s@@@Z)
    void __fastcall Actor_InitPath(actor_s*)
{
}

// ea: 0x0077C500  (?Actor_ClearPath@@YIXPAUactor_s@@@Z)
    void __fastcall Actor_ClearPath(actor_s*)
{
}

// ea: 0x0077C510  (?Actor_GetFinalPathGoal@@YIXPAUactor_s@@QAM@Z)
    void __fastcall Actor_GetFinalPathGoal(actor_s*, float* const)
{
}

// ea: 0x0077C520  (?Actor_GetAnimDeltas@@YIXPAUactor_s@@QAM1@Z)
    void __fastcall Actor_GetAnimDeltas(actor_s*, float* const, float* const)
{
}

// ea: 0x0077C530  (?Actor_UpdateOriginAndAngles@@YIXPAUactor_s@@H@Z)
    void __fastcall Actor_UpdateOriginAndAngles(actor_s*, int)
{
}

// ea: 0x0077C540  (?Actor_PredictOriginAndAngles@@YIXPAUactor_s@@@Z)
    void __fastcall Actor_PredictOriginAndAngles(actor_s*)
{
}

// ea: 0x0077C550  (?Actor_PredictAnim@@YIXPAUactor_s@@@Z)
    void __fastcall Actor_PredictAnim(actor_s*)
{
}

// ea: 0x0077C560  (?Actor_MoveAlongPathWithTeam@@YI?AW4ai_teammove_t@@PAUactor_s@@HH@Z)
    ai_teammove_t __fastcall Actor_MoveAlongPathWithTeam(actor_s*, int, int)
{
    return (ai_teammove_t)-1;
}

// ea: 0x0077C570  (?Actor_AtClaimNode@@YIHPAUactor_s@@@Z)
    int __fastcall Actor_AtClaimNode(actor_s*)
{
    return 0;
}

// ea: 0x0077C580  (?Actor_CheckCollisions@@YIXPAUactor_s@@@Z)
    void __fastcall Actor_CheckCollisions(actor_s*)
{
}

// ea: 0x0077C590  (?Actor_PathEncroachesPoint2D@@YIHPAUactor_s@@QBMM@Z)
    int __fastcall Actor_PathEncroachesPoint2D(actor_s*, const float* const, float)
{
    return 0;
}

// ea: 0x0077C5A0  (?Path_UpdateBadPlaceCount@@YAXPAXH@Z)
    void Path_UpdateBadPlaceCount(void*, int)
{
}

// ea: 0x0077C5B0  (?Path_RemoveBadPlace@@YAXABVstring@Broc@@@Z)
    void Path_RemoveBadPlace(const Broc::string&)
{
}

// ea: 0x0077C5C0  (?Path_MakeArcBadPlace@@YAXABVstring@Broc@@HHPAUBadPlaceArc@@@Z)
    void Path_MakeArcBadPlace(const Broc::string&, int, int, BadPlaceArc*)
{
}

// ea: 0x0077C5D0  (?Path_InitBadPlaces@@YAXXZ)
    void Path_InitBadPlaces(void)
{
}

// ea: 0x0077C5E0  (?Path_ShutdownBadPlaces@@YAXXZ)
    void Path_ShutdownBadPlaces(void)
{
}

// ea: 0x0077C5F0  (?Path_RunBadPlaces@@YAXXZ)
    void Path_RunBadPlaces(void)
{
}

// ea: 0x0077C600  (?Actor_Physics@@YAHPAUactor_physics_t@@@Z)
    int Actor_Physics(actor_physics_t*)
{
    return 0;
}

// ea: 0x0077C610  (?Actor_PostPhysics@@YAXPAUactor_physics_t@@@Z)
    void Actor_PostPhysics(actor_physics_t*)
{
}

// ea: 0x0077C620  (?Actor_Grenade_GetTossPositions@@YIHPAUactor_s@@QBM11QAM2@Z)
    int __fastcall Actor_Grenade_GetTossPositions(actor_s*, const float* const, const float* const, const float* const, float* const, float* const)
{
    return 0;
}

// ea: 0x0077C650  (?Actor_Grenade_IsSafeTarget@@YIHPAUactor_s@@QBMH@Z)
    int __fastcall Actor_Grenade_IsSafeTarget(actor_s*, const float* const, int)
{
    return 0;
}

// ea: 0x0077C660  (?Actor_GrenadePing@@YIXPAUactor_s@@PAVEntity@@@Z)
    void __fastcall Actor_GrenadePing(actor_s*, Entity*)
{
}

// ea: 0x0077C670  (?Actor_Grenade_Attach@@YIXPAUactor_s@@@Z)
    void __fastcall Actor_Grenade_Attach(actor_s*)
{
}

// ea: 0x0077C680  (?Actor_Grenade_Detach@@YIXPAUactor_s@@@Z)
    void __fastcall Actor_Grenade_Detach(actor_s*)
{
}

// ea: 0x0077C690  (?Actor_Grenade_DropIfHeld@@YIXPAUactor_s@@@Z)
    void __fastcall Actor_Grenade_DropIfHeld(actor_s*)
{
}

// ea: 0x0077C6A0  (?Actor_IsSuppressed@@YIHPAUactor_s@@@Z)
    int __fastcall Actor_IsSuppressed(actor_s*)
{
    return 0;
}

// ea: 0x0077C6B0  (?Actor_IsMoveSuppressed@@YIHPAUactor_s@@@Z)
    int __fastcall Actor_IsMoveSuppressed(actor_s*)
{
    return 0;
}

// ea: 0x0077C6C0  (?Actor_GetSuppressionPlanes@@YI?BHPAUactor_s@@QAY01MQAM@Z)
    int __fastcall Actor_GetSuppressionPlanes(actor_s*, float (*)[2], float* const)
{
    return 0;
}

// ea: 0x0077C6D0  (?Actor_GetSuppressionCount@@YI?BHPAUactor_s@@@Z)
    int __fastcall Actor_GetSuppressionCount(actor_s*)
{
    return 0;
}

// ea: 0x0077C6E0  (?Actor_InterruptPoint@@YAXPAUactor_s@@@Z)
    void Actor_InterruptPoint(actor_s*)
{
}

// ea: 0x0077C6F0  (?Actor_CurrentLookAtAnimYawMax@@YAMPAUactor_s@@@Z)
    float Actor_CurrentLookAtAnimYawMax(actor_s*)
{
    return 0.0f;
}

// ea: 0x0077C700  (?Actor_CurrentLookAtYawMax@@YAMPAUactor_s@@@Z)
    float Actor_CurrentLookAtYawMax(actor_s*)
{
    return 0.0f;
}

// ea: 0x0077C710  (?Path_Copy@@YIXPBUpath_t@@PAU1@@Z)
    void __fastcall Path_Copy(const path_t*, path_t*)
{
}

// ea: 0x0077C720  (?Path_Begin@@YIXPAUpath_t@@@Z)
    void __fastcall Path_Begin(path_t*)
{
}

// ea: 0x0077C730  (?Path_Clear@@YIXPAUpath_t@@@Z)
    void __fastcall Path_Clear(path_t*)
{
}

// ea: 0x0077C740  (?Path_Exists@@YI?BHPBUpath_t@@@Z)
    int __fastcall Path_Exists(const path_t*)
{
    return 0;
}

// ea: 0x0077C750  (?Path_CompleteLookahead@@YI?BHPBUpath_t@@@Z)
    int __fastcall Path_CompleteLookahead(const path_t*)
{
    return 0;
}

// ea: 0x0077C760  (?Path_AttemptedCompleteLookahead@@YI?BHPBUpath_t@@@Z)
    int __fastcall Path_AttemptedCompleteLookahead(const path_t*)
{
    return 0;
}

// ea: 0x0077C770  (?Path_UsesObstacleNegotiation@@YI?BHPBUpath_t@@@Z)
    int __fastcall Path_UsesObstacleNegotiation(const path_t*)
{
    return 0;
}

// ea: 0x0077C780  (?Path_AllowsObstacleNegotiation@@YI?BHPBUpath_t@@@Z)
    int __fastcall Path_AllowsObstacleNegotiation(const path_t*)
{
    return 0;
}

// ea: 0x0077C790  (?Path_GetObstacleNegotiationScript@@YIXPBUpath_t@@PAUscr_animscript_t@@PAM@Z)
    void __fastcall Path_GetObstacleNegotiationScript(const path_t*, scr_animscript_t*, float*)
{
}

// ea: 0x0077C7A0  (?Path_GetFinalGoal@@YIXPBUpath_t@@QAM@Z)
    void __fastcall Path_GetFinalGoal(const path_t*, float* const)
{
}

// ea: 0x0077C7B0  (?Path_NeedsReevaluation@@YIHPAUpath_t@@QBMMH@Z)
    int __fastcall Path_NeedsReevaluation(path_t*, const float* const, float, int)
{
    return 0;
}

// ea: 0x0077C7C0  (?Sentient_GetScarinessForDistance@@YAMPAUsentient_s@@0M@Z)
    float Sentient_GetScarinessForDistance(sentient_s*, sentient_s*, float)
{
    return 0.0f;
}

// ea: 0x0077C7D0  (?Actor_Shoot@@YAXPAUactor_s@@MPAY02MW4enumLastShot@@@Z)
    void Actor_Shoot(actor_s*, float, float (*)[3], enumLastShot)
{
}

// ea: 0x0077C7E0  (?Actor_Melee@@YAXPAUactor_s@@@Z)
    void Actor_Melee(actor_s*)
{
}

// ea: 0x0077C7F0  (?PointCouldSeeSpawn@@YAHABVPosition3@math@@0V?$DbLinkedHandle@VEntityHandleDb@@VEntity@@@@1@Z)
    int PointCouldSeeSpawn(const math::Position3&, const math::Position3&, DbLinkedHandle<EntityHandleDb, Entity>, DbLinkedHandle<EntityHandleDb, Entity>)
{
    return 0;
}

// ea: 0x0077C800  (?SpawnActor@@YAPAVEntity@@PAV1@ABVstring@Broc@@W4enumForceSpawn@@W4TPakId@@@Z)
    Entity* SpawnActor(Entity*, const Broc::string&, enumForceSpawn, TPakId)
{
    return nullptr;
}

// ea: 0x0077C810  (?SP_actor_spawner@@YAHPAVEntity@@@Z)
    int SP_actor_spawner(Entity*)
{
    return 0;
}

// ea: 0x0077C820  (?Actor_InitMove@@YIXPAUactor_s@@@Z)
    void __fastcall Actor_InitMove(actor_s*)
{
}

// ea: 0x0077C830  (?Actor_DoMove@@YIXPAUactor_s@@@Z)
    void __fastcall Actor_DoMove(actor_s*)
{
}

// ea: 0x0077C840  (?Actor_UpdateProneInformation@@YIXPAUactor_s@@H@Z)
    void __fastcall Actor_UpdateProneInformation(actor_s*, int)
{
}

// ea: 0x0077C850  (?actor_controller@@YAXPAVEntity@@QAH@Z)
    void actor_controller(Entity*, int* const)
{
}

// ea: 0x0077C860  (?Actor_GetPotentialNodeList@@YIXPAUactor_s@@HQAMMMP6IH0PBUPathNode@PathNodes@@@ZP6IM02@Z@Z)
    void __fastcall Actor_GetPotentialNodeList(actor_s*, int, float* const, float, float, int (__fastcall*)(actor_s*, const PathNodes::PathNode*), float (__fastcall*)(actor_s*, const PathNodes::PathNode*))
{
}

// ea: 0x0077C870  (?Path_AttemptDodge@@YIHPAUpath_t@@QBM11HV?$DbLinkedHandle@VEntityHandleDb@@VEntity@@@@2HH@Z)
    int __fastcall Path_AttemptDodge(path_t*, const float* const, const float* const, const float* const, int, DbLinkedHandle<EntityHandleDb, Entity>, DbLinkedHandle<EntityHandleDb, Entity>, int, int)
{
    return 0;
}

// ea: 0x0077C880  (?Path_MayFaceEnemy@@YIHPAUpath_t@@QAM1@Z)
    int __fastcall Path_MayFaceEnemy(path_t*, float* const, float* const)
{
    return 0;
}

// ea: 0x0077C890  (?Path_PredictionTrace@@YIHQBM0V?$DbLinkedHandle@VEntityHandleDb@@VEntity@@@@1HQAM@Z)
    int __fastcall Path_PredictionTrace(const float* const, const float* const, DbLinkedHandle<EntityHandleDb, Entity>, DbLinkedHandle<EntityHandleDb, Entity>, int, float* const)
{
    return 0;
}

// ea: 0x0077C8A0  (?Path_ContainsBadPlaceLink@@YIHPAUpath_t@@W4team_t@@@Z)
    int __fastcall Path_ContainsBadPlaceLink(path_t*, team_t)
{
    return 0;
}

// ea: 0x0077C8B0  (?EdgesCrissCross2D@@YA?BHQBM000@Z)
    int EdgesCrissCross2D(const float* const, const float* const, const float* const, const float* const)
{
    return 0;
}

// ea: 0x0077C8C0  (?Actor_SetDesiredLookAngles@@YIXPAUai_orient_t@@MM@Z)
    void __fastcall Actor_SetDesiredLookAngles(ai_orient_t*, float, float)
{
}

// ea: 0x0077C8D0  (?Actor_SetDesiredAngles@@YIXPAUai_orient_t@@MM@Z)
    void __fastcall Actor_SetDesiredAngles(ai_orient_t*, float, float)
{
}

// ea: 0x0077C8E0  (?Actor_SetLookAngles@@YIXPAUactor_s@@MM@Z)
    void __fastcall Actor_SetLookAngles(actor_s*, float, float)
{
}

// ea: 0x0077C8F0  (?Actor_SetBodyAngle@@YIXPAUactor_s@@M@Z)
    void __fastcall Actor_SetBodyAngle(actor_s*, float)
{
}

// ea: 0x0077C900  (?Actor_ChangeAngles@@YIXPAUactor_s@@MM@Z)
    void __fastcall Actor_ChangeAngles(actor_s*, float, float)
{
}

// ea: 0x0077C910  (?Actor_UpdateLookAngles@@YIXPAUactor_s@@H@Z)
    void __fastcall Actor_UpdateLookAngles(actor_s*, int)
{
}

// ea: 0x0077C920  (?Actor_UpdateBodyAngle@@YIXPAUactor_s@@H@Z)
    void __fastcall Actor_UpdateBodyAngle(actor_s*, int)
{
}

// ea: 0x0077C930  (?Actor_FaceVector@@YIXPAUai_orient_t@@QBMM@Z)
    void __fastcall Actor_FaceVector(ai_orient_t*, const float* const, float)
{
}

// ea: 0x0077C940  (?Actor_FaceEnemy@@YIXPAUactor_s@@PAUai_orient_t@@M@Z)
    void __fastcall Actor_FaceEnemy(actor_s*, ai_orient_t*, float)
{
}

// ea: 0x0077C950  (?Actor_DecideOrientation@@YIXPAUactor_s@@@Z)
    void __fastcall Actor_DecideOrientation(actor_s*)
{
}

// ea: 0x0077C960  (?Actor_SetOrientMode@@YIXPAUactor_s@@W4ai_orient_mode_t@@@Z)
    void __fastcall Actor_SetOrientMode(actor_s*, ai_orient_mode_t)
{
}

// ea: 0x0077C970  (?Actor_Get@@YIPAUactor_s@@V?$DbLinkedHandle@VEntityHandleDb@@VEntity@@@@@Z)
    actor_s* __fastcall Actor_Get(DbLinkedHandle<EntityHandleDb, Entity>)
{
    return nullptr;
}

// ea: 0x0077C980  (?Path_GetMaxScariness@@YIMPAUpath_t@@PAUactor_s@@@Z)
    float __fastcall Path_GetMaxScariness(path_t*, actor_s*)
{
    return 0.0f;
}

// ea: 0x0077C990  (?Path_TrimToBravery@@YIHPAUpath_t@@PAUactor_s@@@Z)
    int __fastcall Path_TrimToBravery(path_t*, actor_s*)
{
    return 0;
}

// ea: 0x0077C9A0  (?Path_AllowedStancesForPath@@YI?AW4ai_stance_e@@PAUpath_t@@@Z)
    ai_stance_e __fastcall Path_AllowedStancesForPath(path_t*)
{
    return (ai_stance_e)7;
}

// ea: 0x0077C9B0  (?Path_ApproxDist@@YIMPAUpath_t@@@Z)
    float __fastcall Path_ApproxDist(path_t*)
{
    return 0.0f;
}

// ea: 0x0077C9C0  (?Actor_Exposed_StartReacquire@@YAXPAUactor_s@@W4canChangeState_t@@@Z)
    void Actor_Exposed_StartReacquire(actor_s*, canChangeState_t)
{
}

// ea: 0x0077C9D0  (?Actor_UseTurret@@YIHPAUactor_s@@PAVEntity@@@Z)
    int __fastcall Actor_UseTurret(actor_s*, Entity*)
{
    return 0;
}

// ea: 0x0077C9E0  (?Actor_StopUseTurret@@YIXPAUactor_s@@@Z)
    void __fastcall Actor_StopUseTurret(actor_s*)
{
}

// ea: 0x0077C9F0  (?Actor_IsUsingTurret@@YIHPAUactor_s@@@Z)
    int __fastcall Actor_IsUsingTurret(actor_s*)
{
    return 0;
}

// ea: 0x0077CA00  (?Path_GetMovementDir2D@@YIXPAUpath_t@@QAM@Z)
    void __fastcall Path_GetMovementDir2D(path_t*, float* const)
{
}

// ea: 0x0077CA10  (?G_GetActorFriendlyIndex@@YAHPAVEntity@@@Z)
    int G_GetActorFriendlyIndex(Entity*)
{
    return 0;
}

// ea: 0x0077CA20  (?G_GetFriendlyIndexActor@@YAPAVEntity@@H@Z)
    Entity* G_GetFriendlyIndexActor(int)
{
    return nullptr;
}

// ea: 0x0077CA30  (?G_GetNonPVSFriendlyInfo@@YAHQAMV?$DbLinkedHandle@VEntityHandleDb@@VEntity@@@@@Z)
    int G_GetNonPVSFriendlyInfo(float* const, DbLinkedHandle<EntityHandleDb, Entity>)
{
    return 0;
}

// ea: 0x0077CA40  (?Path_EncroachesPoint2D@@YIHPAUpath_t@@QBM1M@Z)
    int __fastcall Path_EncroachesPoint2D(path_t*, const float* const, const float* const, float)
{
    return 0;
}

// ea: 0x0077CA50  (?Path_DistanceGreaterThan@@YIHPAUpath_t@@M@Z)
    int __fastcall Path_DistanceGreaterThan(path_t*, float)
{
    return 0;
}

// ea: 0x0077CA60  (?Path_AddTrimmedAmount@@YIXPAUpath_t@@QBM@Z)
    void __fastcall Path_AddTrimmedAmount(path_t*, const float* const)
{
}

// ea: 0x0077CA70  (?Path_UpdateLookahead@@YIXPAUpath_t@@QBMHH@Z)
    void __fastcall Path_UpdateLookahead(path_t*, const float* const, int, int)
{
}

// ea: 0x0077CA80  (?Path_GetForwardStartPos@@YIHPAUpath_t@@QBMQAM@Z)
    int __fastcall Path_GetForwardStartPos(path_t*, const float* const, float* const)
{
    return 0;
}

// ea: 0x0077CA90  (?Path_IsTrimmed@@YIHPAUpath_t@@@Z)
    int __fastcall Path_IsTrimmed(path_t*)
{
    return 0;
}

// ea: 0x0077CAA0  (?Path_DebugDraw@@YIXPAUpath_t@@QBMH@Z)
    void __fastcall Path_DebugDraw(path_t*, const float* const, int)
{
}

// ea: 0x0077CAB0  (?Path_FailedLookahead@@YIHPAUpath_t@@@Z)
    int __fastcall Path_FailedLookahead(path_t*)
{
    return 0;
}

// ea: 0x0077CAC0  (?Path_FindPath@@YI?BHPAUpath_t@@W4team_t@@QBM2H@Z)
    int __fastcall Path_FindPath(path_t*, team_t, const float* const, const float* const, int)
{
    return 0;
}

// ea: 0x0077CAD0  (?Path_FindPathFrom@@YI?BHPAUpath_t@@W4team_t@@PAUPathNode@PathNodes@@QBM3H@Z)
    int __fastcall Path_FindPathFrom(path_t*, team_t, PathNodes::PathNode*, const float* const, const float* const, int)
{
    return 0;
}

// ea: 0x0077CAE0  (?Path_FindPathFromTo@@YI?BHPAUpath_t@@W4team_t@@PAUPathNode@PathNodes@@QBM23H@Z)
    int __fastcall Path_FindPathFromTo(path_t*, team_t, PathNodes::PathNode*, const float* const, PathNodes::PathNode*, const float* const, int)
{
    return 0;
}

// ea: 0x0077CAF0  (?Path_FindPathNotCrossPlanes@@YI?BHPAUpath_t@@W4team_t@@QBM2QAY01MQAMHH@Z)
    int __fastcall Path_FindPathNotCrossPlanes(path_t*, team_t, const float* const, const float* const, float (*)[2], float* const, int, int)
{
    return 0;
}

// ea: 0x0077CB00  (?Path_FindPathFromNotCrossPlanes@@YI?BHPAUpath_t@@W4team_t@@PAUPathNode@PathNodes@@QBM3QAY01MQAMHH@Z)
    int __fastcall Path_FindPathFromNotCrossPlanes(path_t*, team_t, PathNodes::PathNode*, const float* const, const float* const, float (*)[2], float* const, int, int)
{
    return 0;
}

// ea: 0x0077CB10  (?Path_FindPathFromToNotCrossPlanes@@YI?BHPAUpath_t@@W4team_t@@PAUPathNode@PathNodes@@QBM23QAY01MQAMHH@Z)
    int __fastcall Path_FindPathFromToNotCrossPlanes(path_t*, team_t, PathNodes::PathNode*, const float* const, PathNodes::PathNode*, const float* const, float (*)[2], float* const, int, int)
{
    return 0;
}

// ea: 0x0077CB20  (?Path_FindPathAway@@YI?BHPAUpath_t@@W4team_t@@QBM2MH@Z)
    int __fastcall Path_FindPathAway(path_t*, team_t, const float* const, const float* const, float, int)
{
    return 0;
}

// ea: 0x0077CB30  (?Path_FindPathFromAway@@YI?BHPAUpath_t@@W4team_t@@PAUPathNode@PathNodes@@QBM3MH@Z)
    int __fastcall Path_FindPathFromAway(path_t*, team_t, PathNodes::PathNode*, const float* const, const float* const, float, int)
{
    return 0;
}

// ea: 0x0077CB40  (?Path_FindPathAwayNotCrossPlanes@@YI?BHPAUpath_t@@W4team_t@@QBM2MQAY01MQAMHH@Z)
    int __fastcall Path_FindPathAwayNotCrossPlanes(path_t*, team_t, const float* const, const float* const, float, float (*)[2], float* const, int, int)
{
    return 0;
}

// ea: 0x0077CB50  (?Path_FindPathFromAwayNotCrossPlanes@@YI?BHPAUpath_t@@W4team_t@@PAUPathNode@PathNodes@@QBM3MQAY01MQAMHH@Z)
    int __fastcall Path_FindPathFromAwayNotCrossPlanes(path_t*, team_t, PathNodes::PathNode*, const float* const, const float* const, float, float (*)[2], float* const, int, int)
{
    return 0;
}

// ea: 0x0077CB60  (?Path_FindPathNear@@YI?BHPAUpath_t@@W4team_t@@QBM2MH@Z)
    int __fastcall Path_FindPathNear(path_t*, team_t, const float* const, const float* const, float, int)
{
    return 0;
}

// ea: 0x0077CB70  (?Path_FindPathNearNotCrossPlanes@@YI?BHPAUpath_t@@W4team_t@@QBM2MQAY01MQAMHH@Z)
    int __fastcall Path_FindPathNearNotCrossPlanes(path_t*, team_t, const float* const, const float* const, float, float (*)[2], float* const, int, int)
{
    return 0;
}

// ea: 0x0077CB80  (?Path_FindPathInCylinderNear@@YI?BHPAUpath_t@@W4team_t@@QBM22MMMH@Z)
    int __fastcall Path_FindPathInCylinderNear(path_t*, team_t, const float* const, const float* const, const float* const, float, float, float, int)
{
    return 0;
}

// ea: 0x0077CB90  (?Path_FindPathInCylinderNearNotCrossPlanes@@YI?BHPAUpath_t@@W4team_t@@QBM22MMMQAY01MQAMHH@Z)
    int __fastcall Path_FindPathInCylinderNearNotCrossPlanes(path_t*, team_t, const float* const, const float* const, const float* const, float, float, float, float (*)[2], float* const, int, int)
{
    return 0;
}

// ea: 0x0077CBA0  (?Path_FindPathFromInCylinder@@YI?BHPAUpath_t@@W4team_t@@PAUPathNode@PathNodes@@QBM33MMH@Z)
    int __fastcall Path_FindPathFromInCylinder(path_t*, team_t, PathNodes::PathNode*, const float* const, const float* const, const float* const, float, float, int)
{
    return 0;
}

// ea: 0x0077CBB0  (?Path_FindPathFromInCylinderNotCrossPlanes@@YI?BHPAUpath_t@@W4team_t@@PAUPathNode@PathNodes@@QBM33MMQAY01MQAMHH@Z)
    int __fastcall Path_FindPathFromInCylinderNotCrossPlanes(path_t*, team_t, PathNodes::PathNode*, const float* const, const float* const, const float* const, float, float, float (*)[2], float* const, int, int)
{
    return 0;
}

// ea: 0x0077CBC0  (?Initialize@BadPathManager@@QAEXXZ)
    void BadPathManager::Initialize(void)
{
}

// ea: 0x0077CBD0  (?SP_info_grenade_hint@@YAXPAVEntity@@@Z)
    void SP_info_grenade_hint(Entity*)
{
}

// ea: 0x0077CBE0  (?SP_actor@@YAHPAVEntity@@@Z)
    int SP_actor(Entity*)
{
    return 0;
}

// ea: 0x0077DCD0  (?Sentient_SetEnemy@@YIXPAUsentient_s@@0H@Z)
    void __fastcall Sentient_SetEnemy(sentient_s*, sentient_s*, int)
{
}

// ea: 0x0077E3F0  (?G_FreePathnodesScriptInfo@@YAXXZ)
    void G_FreePathnodesScriptInfo(void)
{
}

// ea: 0x0077E5A0  (?Path_DrawDebugNearestNode@@YAXQBMH@Z)
    void Path_DrawDebugNearestNode(const float* const, int)
{
}
