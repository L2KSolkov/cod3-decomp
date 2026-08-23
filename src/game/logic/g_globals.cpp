// ============================================================================
// g_globals.cpp - g.o data globals (level, debug/LOS state, shared constants)
// ============================================================================

#include "game/logic/g_local.h"

level_locals_t level;          // ?level@@3Ulevel_locals_t@@A @ 0xEC9650
SaveGameData gSaveGameData[4] = {};   // ?gSaveGameData@@3PAUSaveGameData@@A @ 0xF312F0
cvar_t* g_gameskill;           // ?g_gameskill@@3PAUcvar_t@@A (g.o)
cgGlobal_t cgGlobal;           // ?cgGlobal@@3UcgGlobal_t@@A @ 0xF5FE30
game_hudelem_s g_hudelems[16];  // ?g_hudelems@@3PAUgame_hudelem_s@@A @ 0xEA5580
scr_data_t g_scr_data;          // ?g_scr_data@@3Uscr_data_t@@A @ 0xEE58D0
int g_DOBJF_NOT_RENDERED_LAST_FRAME;  // ?g_DOBJF_NOT_RENDERED_LAST_FRAME@@3HA (core.o)
int g_MPAARTotalTime = 30;             // ?g_MPAARTotalTime@@3HA (mp.o)
kuju::knet::sTime g_MPAARTimer;         // ?g_MPAARTimer@@3VsTime@knet@kuju@@A @ 0xF99870
void* gDebugEntity;                     // ?gDebugEntity@@3PAXA @ 0xF3A5CC
class AbstractEffectParticle;
AbstractEffectParticle* gLastAbstractEffectParticle; // ?gLastAbstractEffectParticle@@3PAVAbstractEffectParticle@@A
vmCvar_t g_changelevel_time;            // ?g_changelevel_time@@3UvmCvar_t@@A
int g_bOptimize = 0;                  // ?g_bOptimize@@3HA (render.o)
int gParticleBatchGroup = 0;          // ?gParticleBatchGroup@@3HA (render.o)
int gDelayRenderForNFrames = 0;       // ?gDelayRenderForNFrames@@3HA (render.o)
vmCvar_t bg_viewheight_standing;   // ?bg_viewheight_standing@@3UvmCvar_t@@A (game.o)
vmCvar_t bg_viewheight_crouched;   // ?bg_viewheight_crouched@@3UvmCvar_t@@A (game.o @ 0x1334D40)
vmCvar_t bg_viewheight_prone;      // ?bg_viewheight_prone@@3UvmCvar_t@@A (game.o @ 0x132DF30)
vmCvar_t bg_meleeassistaspeed;     // ?bg_meleeassistaspeed@@3UvmCvar_t@@A (game.o @ 0x13336D0)
vmCvar_t g_gravity;                // ?g_gravity@@3UvmCvar_t@@A (g.o @ 0x1295240)
vmCvar_t g_debugBullets;           // ?g_debugBullets@@3UvmCvar_t@@A (g.o @ 0x12962C8)
vmCvar_t g_drawEntBBoxes;          // ?g_drawEntBBoxes@@3UvmCvar_t@@A (g.o @ 0x1296358)
vmCvar_t g_debugGrenades;          // ?g_debugGrenades@@3UvmCvar_t@@A (g.o @ 0x12963E8)
vmCvar_t g_debugProneCheckDepthCheck;  // ?g_debugProneCheckDepthCheck@@3UvmCvar_t@@A (g.o @ 0x1296AA8)
vmCvar_t g_debugProneCheck;        // ?g_debugProneCheck@@3UvmCvar_t@@A (g.o @ 0x1296D60)
vmCvar_t g_player_maxhealth;       // ?g_player_maxhealth@@3UvmCvar_t@@A (g.o @ 0x1296DF0)
vmCvar_t mp_friendlyfire;          // ?mp_friendlyfire@@3UvmCvar_t@@A (g.o @ 0x129B0C8)
vmCvar_t g_speed;                  // ?g_speed@@3UvmCvar_t@@A (g.o @ 0x129D518)
vmCvar_t g_reloading;              // ?g_reloading@@3UvmCvar_t@@A (g.o @ 0x129D9A0)
vmCvar_t memory_reportAepsStats;           // ?memory_reportAepsStats@@3UvmCvar_t@@A (game2.o @ 0x12F28B0)
vmCvar_t memory_displayAepsStats;          // ?memory_displayAepsStats@@3UvmCvar_t@@A (game2.o @ 0x12F2948)
vmCvar_t memory_reportBrocBackupStackPool; // ?memory_reportBrocBackupStackPool@@3UvmCvar_t@@A (game2.o @ 0x12F2A10)
vmCvar_t memory_showStatistics;            // ?memory_showStatistics@@3UvmCvar_t@@A (game2.o @ 0x12F2AC0)
vmCvar_t memory_reportCommonPool;          // ?memory_reportCommonPool@@3UvmCvar_t@@A (game2.o @ 0x12F2B50)
vmCvar_t memory_reportBrocPool;            // ?memory_reportBrocPool@@3UvmCvar_t@@A (game2.o @ 0x12F2BF0)
vmCvar_t sound_disableAllOtherSounds;      // ?sound_disableAllOtherSounds@@3UvmCvar_t@@A (game2.o @ 0x12F34B8)
vmCvar_t sound_showSoundStatForEntity;     // ?sound_showSoundStatForEntity@@3UvmCvar_t@@A (game2.o @ 0x12F3D58)
vmCvar_t ai_showFriendlyChains;     // ?ai_showFriendlyChains@@3UvmCvar_t@@A (g.o @ 0x1296F10)
vmCvar_t ai_showNearestNode;        // ?ai_showNearestNode@@3UvmCvar_t@@A (g.o @ 0x129D910)
vmCvar_t ai_showNodes;              // ?ai_showNodes@@3UvmCvar_t@@A (g.o @ 0x1295D18)
vmCvar_t ai_showNodesDist;          // ?ai_showNodesDist@@3UvmCvar_t@@A (g.o @ 0x129E268)
vmCvar_t bg_fallDamageMaxHeight;    // ?bg_fallDamageMaxHeight@@3UvmCvar_t@@A (game.o @ 0x13300E0)
vmCvar_t bg_fallDamageMinHeight;    // ?bg_fallDamageMinHeight@@3UvmCvar_t@@A (game.o @ 0x1334428)
vmCvar_t bg_ladder_yawcap;          // ?bg_ladder_yawcap@@3UvmCvar_t@@A (game.o @ 0x1334B00)
vmCvar_t bg_lmg_yawcap;             // ?bg_lmg_yawcap@@3UvmCvar_t@@A (game.o @ 0x132B838)
vmCvar_t bg_nofatigue;              // ?bg_nofatigue@@3UvmCvar_t@@A (game.o @ 0x1332E00)
vmCvar_t bg_prone_yawcap;           // ?bg_prone_yawcap@@3UvmCvar_t@@A (game.o @ 0x1332ED8)
vmCvar_t bg_foliagesnd_minspeed;
vmCvar_t bg_foliagesnd_maxspeed;
vmCvar_t bg_foliagesnd_slowinterval;
vmCvar_t bg_foliagesnd_fastinterval;
vmCvar_t bg_foliagesnd_resetinterval;
vmCvar_t g_debugDamage;             // ?g_debugDamage@@3UvmCvar_t@@A (g.o @ 0x12A0388)
vmCvar_t g_debugMove;               // ?g_debugMove@@3UvmCvar_t@@A (g.o @ 0x12955B0)
vmCvar_t g_drawSmokeGren;           // ?g_drawSmokeGren@@3UvmCvar_t@@A (g.o @ 0x12956D0)
vmCvar_t g_entinfo_maxdist;         // ?g_entinfo_maxdist@@3UvmCvar_t@@A (g.o @ 0x129D7F0)
vmCvar_t g_entinfo_scale;           // ?g_entinfo_scale@@3UvmCvar_t@@A (g.o @ 0x1294520)
vmCvar_t g_knockback;               // ?g_knockback@@3UvmCvar_t@@A (g.o @ 0x1296508)
vmCvar_t g_performanceTest;         // ?g_performanceTest@@3UvmCvar_t@@A (g.o @ 0x129D488)
vmCvar_t g_performanceTestCell;     // ?g_performanceTestCell@@3UvmCvar_t@@A (g.o @ 0x129DE20)
vmCvar_t g_performanceTestDelta;    // ?g_performanceTestDelta@@3UvmCvar_t@@A (g.o @ 0x129E140)
vmCvar_t g_performanceTestDeltaAngle;  // ?g_performanceTestDeltaAngle@@3UvmCvar_t@@A (g.o @ 0x129E020)
vmCvar_t g_vehControlMode;          // ?g_vehControlMode@@3UvmCvar_t@@A (g.o @ 0x129D248)
vmCvar_t mp_gametype;               // ?mp_gametype@@3UvmCvar_t@@A (g.o @ 0x129B548)
vmCvar_t mp_headIconDistAbovePlayer;    // ?mp_headIconDistAbovePlayer@@3UvmCvar_t@@A (g.o @ 0x1296A18)
vmCvar_t mp_headIconDistAboveVehicle;   // ?mp_headIconDistAboveVehicle@@3UvmCvar_t@@A (g.o @ 0x129B8A8)
vmCvar_t mp_headIconHeight;             // ?mp_headIconHeight@@3UvmCvar_t@@A (g.o @ 0x1295888)
vmCvar_t mp_headIconMinScreenSize;      // ?mp_headIconMinScreenSize@@3UvmCvar_t@@A (g.o @ 0x1295AD8)
vmCvar_t mp_headIconReviveMaxAlphaDist; // ?mp_headIconReviveMaxAlphaDist@@3UvmCvar_t@@A (g.o @ 0x129B818)
vmCvar_t mp_headIconReviveMinAlphaDist; // ?mp_headIconReviveMinAlphaDist@@3UvmCvar_t@@A (g.o @ 0x12A02F8)
vmCvar_t mp_itemIconDistAboveItem;      // ?mp_itemIconDistAboveItem@@3UvmCvar_t@@A (g.o @ 0x1295A40)
vmCvar_t mp_itemIconHeight;             // ?mp_itemIconHeight@@3UvmCvar_t@@A (g.o @ 0x1296238)
vmCvar_t mp_itemIconMaxAlphaDist;       // ?mp_itemIconMaxAlphaDist@@3UvmCvar_t@@A (g.o @ 0x1295368)
vmCvar_t mp_itemIconMinAlphaDist;       // ?mp_itemIconMinAlphaDist@@3UvmCvar_t@@A (g.o @ 0x12A0418)
vmCvar_t mp_itemIconMinScreenSize;      // ?mp_itemIconMinScreenSize@@3UvmCvar_t@@A (g.o @ 0x12946D0)
vmCvar_t mp_objectiveFarAlpha;          // ?mp_objectiveFarAlpha@@3UvmCvar_t@@A (g.o @ 0x129BDB8)
vmCvar_t mp_objectiveFarAlphaDist;      // ?mp_objectiveFarAlphaDist@@3UvmCvar_t@@A (g.o @ 0x129DD00)
vmCvar_t mp_objectiveMaxSize;           // ?mp_objectiveMaxSize@@3UvmCvar_t@@A (g.o @ 0x1296478)
vmCvar_t mp_objectiveMinSize;           // ?mp_objectiveMinSize@@3UvmCvar_t@@A (g.o @ 0x12947F0)
vmCvar_t mp_objectiveNearAlpha;         // ?mp_objectiveNearAlpha@@3UvmCvar_t@@A (g.o @ 0x129B938)
vmCvar_t mp_objectiveNearAlphaDist;     // ?mp_objectiveNearAlphaDist@@3UvmCvar_t@@A (g.o @ 0x1294490)
vmCvar_t mp_objectiveSize;              // ?mp_objectiveSize@@3UvmCvar_t@@A (g.o @ 0x129B278)
vmCvar_t sound_debug;               // ?sound_debug@@3UvmCvar_t@@A (g.o @ 0x129DD90)
vmCvar_t g_cheats;                  // ?g_cheats@@3UvmCvar_t@@A (g.o @ 0x129B668)
vmCvar_t g_developer;               // ?g_developer@@3UvmCvar_t@@A (g.o @ 0x129BB78)
vmCvar_t g_debug_sound_aliases;     // ?g_debug_sound_aliases@@3UvmCvar_t@@A (g.o @ 0x129B308)
vmCvar_t g_dumpAnims;               // ?g_dumpAnims@@3UvmCvar_t@@A (g.o @ 0x12957F8)
vmCvar_t g_listEntity;              // ?g_listEntity@@3UvmCvar_t@@A (g.o @ 0x129B428)
// --- plain int data sweep (g.o family) ---
int gStartTime;                  // ?gStartTime@@3HA (g.o)
int gScreenshotInProgress;       // ?gScreenshotInProgress@@3HA (g.o)
int gRenderMemGraph;             // ?gRenderMemGraph@@3HA (g.o)
int gRenderViewWeapon;           // ?gRenderViewWeapon@@3HA (g.o)
int g_blendType;                 // ?g_blendType@@3HA (g.o)
int g_freeze_movement;           // ?g_freeze_movement@@3HA (g.o)
bool gNANO_Animate;              // ?gNANO_Animate@@3_NA (g.o @ 0x11C4CF4)
int g_renderGameEntityStats;     // ?g_renderGameEntityStats@@3HA (g.o)
int g_renderPFXStats;            // ?g_renderPFXStats@@3HA (g.o)
int g_xanim_num;                 // ?g_xanim_num@@3HA (g.o)
int gRenderCG_2D;                // ?gRenderCG_2D@@3HA (g.o)
int level_time;                  // ?level_time@@3HA (g.o)
int g_bDObjInited;               // ?g_bDObjInited@@3HA (g.o)
int g_scr_data_debris_bro_func;  // ?g_scr_data_debris_bro_func@@3HA (g.o)
int gPakHeaps_m_size;            // ?gPakHeaps_m_size@@3HA (g.o)
int g_gameIsStartingUp;          // ?g_gameIsStartingUp@@3HA (g.o)
int g_disableVSync;              // ?g_disableVSync@@3HA (g.o)
int g_showGPUTimers;             // ?g_showGPUTimers@@3HA (g.o)
int gRenderWorld;                // ?gRenderWorld@@3HA (g.o)
int gRenderEntities;             // ?gRenderEntities@@3HA (g.o)
int gRenderInstanceGroups;       // ?gRenderInstanceGroups@@3HA (g.o)
int gRenderLightGlows;           // ?gRenderLightGlows@@3HA (g.o)
int gRenderFX;                   // ?gRenderFX@@3HA (g.o)
int gRenderSky;                  // ?gRenderSky@@3HA (g.o)
int gRenderLocalEntities;        // ?gRenderLocalEntities@@3HA (g.o)
int gRenderStatusBar;            // ?gRenderStatusBar@@3HA (g.o)
int gRenderDebug;                // ?gRenderDebug@@3HA (g.o)
int g_useRagsOnNormalDeaths;     // ?g_useRagsOnNormalDeaths@@3HA (g.o)
int gPhysicsFinder;              // ?gPhysicsFinder@@3HA (g.o)
int gDebugTrace;                 // ?gDebugTrace@@3HA (g.o)
int gDebugLocationalTrace;       // ?gDebugLocationalTrace@@3HA (g.o)
int g_displayPlayerPosition;     // ?g_displayPlayerPosition@@3HA (g.o)
int g_renderFPS;                 // ?g_renderFPS@@3HA (g.o)
int g_showPathNodeDensity;       // ?g_showPathNodeDensity@@3HA (g.o)
int g_showCulledParticles;       // ?g_showCulledParticles@@3HA (g.o)
int gThreadedParticles;          // ?gThreadedParticles@@3HA (g.o)
int g_renderSphere;              // ?g_renderSphere@@3HA (g.o)
int g_limitVisualRange;          // ?g_limitVisualRange@@3HA (g.o)
int g_displayPlayerStats;        // ?g_displayPlayerStats@@3HA (g.o)
int g_testInt;                   // ?g_testInt@@3HA (g.o)
int gNewEasyMaxHealth;           // ?gNewEasyMaxHealth@@3HA (g.o)
int gNewMediumMaxHealth;         // ?gNewMediumMaxHealth@@3HA (g.o)
int gNewHardMaxHealth;           // ?gNewHardMaxHealth@@3HA (g.o)
int g_showNumBadPaths;           // ?g_showNumBadPaths@@3HA (g.o)
int g_showLightGridDebugText;    // ?g_showLightGridDebugText@@3HA (g.o)
int g_LightGridDecruftifier;     // ?g_LightGridDecruftifier@@3HA (g.o)
int g_showLightGridDistribution; // ?g_showLightGridDistribution@@3HA (g.o)
int g_lightGridBlueErrors;       // ?g_lightGridBlueErrors@@3HA (g.o)
int g_showWeaponRange;           // ?g_showWeaponRange@@3HA (g.o)
int gAIBattleChatterDebug;       // ?gAIBattleChatterDebug@@3HA (g.o)
int g_displayCurrentSounds;      // ?g_displayCurrentSounds@@3HA (g.o)
int g_displayCurrentPrioritySounds;   // ?g_displayCurrentPrioritySounds@@3HA (g.o)
int g_displayCurrentSoundStreamsOnly; // ?g_displayCurrentSoundStreamsOnly@@3HA (g.o)
int g_useOnScreenSoundPosDebugging;   // ?g_useOnScreenSoundPosDebugging@@3HA (g.o)
int g_displaySoundRamUsage;      // ?g_displaySoundRamUsage@@3HA (g.o)
int gTakeScreenshot;             // ?gTakeScreenshot@@3HA (g.o)
int g_fps;                       // ?g_fps@@3HA (g.o)
int g_oceanDebug_Enable;         // ?g_oceanDebug_Enable@@3HA (g.o)
int g_oceanDebug_BankID;         // ?g_oceanDebug_BankID@@3HA (g.o)
int g_oceanDebug_DumpSettings;   // ?g_oceanDebug_DumpSettings@@3HA (g.o)
int g_oceanDebug_Layer2Enable;   // ?g_oceanDebug_Layer2Enable@@3HA (g.o)
int g_oceanDebug_Layer3Enable;   // ?g_oceanDebug_Layer3Enable@@3HA (g.o)
int g_oceanDebug_LightmapEnable; // ?g_oceanDebug_LightmapEnable@@3HA (g.o)
int g_requiredIndex;             // ?g_requiredIndex@@3HA (g.o)
int gCurCheckpoint;              // ?gCurCheckpoint@@3HA (g.o)
int gDebounce;                   // ?gDebounce@@3HA (g.o)
int gLensAlphaAmount;            // ?gLensAlphaAmount@@3HA (g.o)
int gNumStringsAlloc;            // ?gNumStringsAlloc@@3HA (g.o)
int gNumStringsFreed;            // ?gNumStringsFreed@@3HA (g.o)
int s_numNodes;                  // ?s_numNodes@@3HA (g.o)
int s_numVehicleInfos;           // ?s_numVehicleInfos@@3HA (g.o)
int s_clientThink;               // ?s_clientThink@@3HA (g.o)
int sLastSpinnerFrame;           // ?sLastSpinnerFrame@@3HA (g.o)
int iWeaponInfoSource;           // ?iWeaponInfoSource@@3HA (g.o)
int gEnableMeshFlash;            // ?gEnableMeshFlash (g.o; _NA in binary)
int gLockMeshList;               // ?gLockMeshList (g.o; _NA in binary)
int g_networkOwner = kMainThread; // ?g_networkOwner (g.o; EThreadOwner from IDA)
int gTurretState;                // ?gTurretState@@3HA (g.o)
int sEntryPointSeatAssociation[4];  // ?sEntryPointSeatAssociation@@3PAHA (g.o)
int itemRegistered[137];            // ?itemRegistered@@3PAHA (g.o @ 0xEA68A8)
int sEntryPointHintIndicies[6];     // ?sEntryPointHintIndicies@@3PAHA (g.o @ 0xECCCF4)
int dword_F037BC[256];              // ?dword_F037BC@@3PAHA (g.o)
int dword_F037C0[256];              // ?dword_F037C0@@3PAHA (g.o)
int dword_F037C4[256];              // ?dword_F037C4@@3PAHA (g.o)
int g_doShellShock[16];             // ?g_doShellShock@@3PAHA (g.o)
int dword_186A0;                    // ?dword_186A0@@3HA (game.o)
vehicle_node_t* s_nodes[64];        // ?s_nodes@@3PAPAUvehicle_node_t@@A (g.o @ 0x129E2F8)
scr_vehicle_t* s_vehicles = nullptr;  // ?s_vehicles@@3PAUscr_vehicle_t@@A (g.o @ 0x12A0604)
hitLoc g_hitLocs[64];               // ?g_hitLocs@@3PAUhitLoc@@A (g.o @ 0xDD76E0)
cspField_t s_vehicleFields[73];     // ?s_vehicleFields@@3PAUcspField_t@@A (g.o @ 0xDD6EF0)
turretInfo_t turretInfo[1];         // ?turretInfo@@3PAUturretInfo_t@@A (g.o @ 0xED9E08)
gitem_s bg_itemlist[138];           // ?bg_itemlist@@3PAUgitem_s@@A (game.o @ 0x13413C0)
sentient_s g_sentients[48];         // ?g_sentients@@3PAUsentient_s@@A (g.o)
Client g_clients[16];               // ?g_clients@@3PAUClient@@A (g.o)
CVarTable gameCvarTable[32];        // ?gameCvarTable@@3PAUCVarTable@@A (g.o @ 0x11C4CF8)
ClientCmdPair sClientCommand0List[24];  // ?sClientCommand0List@@3PAUClientCmdPair@@A (g.o .rdata)
ClientCmdPair sClientCommand1List[15];  // ?sClientCommand1List@@3PAUClientCmdPair@@A (g.o .rdata)
vehicleAnimMap_t* vehicleAnimMaps[6];  // ?vehicleAnimMaps@@3PAPAUvehicleAnimMap_t@@A (g.o @ 0xDD6E6C)
// --- bool/char/float scalar data sweep ---
bool gNoTargetEnabled;              // ?gNoTargetEnabled@@3_NA (g.o)
bool no_really_delete_it;           // ?no_really_delete_it@@3_NA (g.o)
bool gGodModeEnabled;               // ?gGodModeEnabled@@3_NA (g.o)
float brocLWM;                      // ?brocLWM@@3MA (g.o)
float mainLWM;                      // ?mainLWM@@3MA (g.o)
float delta;                        // ?delta@@3MA (g.o @ 0xDD7FE4)
float delta_0;                      // ?delta_0@@3MA (g.o)
float emissionRate;                 // ?emissionRate@@3MA (g.o @ 0xDD8220)
float r;                            // ?r@@3MA (g.o @ 0xDD8228)
float gTriggerLookAtOverride;       // ?gTriggerLookAtOverride@@3MA (g.o @ 0xDF4914)
float g_fHitLocDamageMult[19];      // ?g_fHitLocDamageMult@@3PAMA (g.o @ 0xEA5380)
float nglPerfInfo_FPS;              // ?nglPerfInfo_FPS@@3MA (ngl.o)
float alpha;                        // ?alpha@@3MA (render.o)
float barWidth;                     // ?barWidth@@3MA (render.o)
float bigHeapScale;                 // ?bigHeapScale@@3MA (render.o)
float fontScale;                    // ?fontScale@@3MA (render.o)
float left;                         // ?left@@3MA (render.o)
float spacing;                      // ?spacing@@3MA (render.o)
float textOffset;                   // ?textOffset@@3MA (render.o)
float textScale;                    // ?textScale@@3MA (render.o)
float tickWidth;                    // ?tickWidth@@3MA (render.o)
const math::Position3 actorMaxs = {};  // ?actorMaxs@@3VPosition3@math@@B (mp_actors.o)
const math::Position3 actorMins = {};  // ?actorMins@@3VPosition3@math@@B (mp_actors.o)
HashString sDamageStr_0;            // ?sDamageStr_0@@3VHashString@@A (g.o)
int gameCvarTableSize;              // ?gameCvarTableSize@@3HA (g.o)
int dword_DD67B8;                   // ?dword_DD67B8@@3HA (g.o @ 0xDD67B8)
int dword_DD67BC;                   // ?dword_DD67BC@@3HA (g.o @ 0xDD67BC)
int dword_DD67C0;                   // ?dword_DD67C0@@3HA (g.o @ 0xDD67C0)
int dword_DD67C4;                   // ?dword_DD67C4@@3HA (g.o @ 0xDD67C4)
int dword_DD67C8;                   // ?dword_DD67C8@@3HA (g.o @ 0xDD67C8)
int dword_EA53C8;                   // ?dword_EA53C8@@3HA (g.o @ 0xEA53C8)
int TAG_WHEEL_FRONT_LEFT = 0;       // ?TAG_WHEEL_FRONT_LEFT@@3HA (g.o)
int TAG_WHEEL_FRONT_RIGHT = 0;      // ?TAG_WHEEL_FRONT_RIGHT@@3HA (g.o)
int render = 0;                     // ?render@@3HA (g.o @ 0xDD725C)
int gLanguage;                      // ?gLanguage@@3HA (core_globals.h typedef twin; real enum in g_entity_misc)


// ea: 0x004A7A10 (mp_actors.o; header inline)
bool IsVehicleSpotted(Entity* vehicle)  // ?IsVehicleSpotted@@YA_NPAVEntity@@@Z
{
    if (vehicle == nullptr || vehicle->scr_vehicle == nullptr)
        return false;
    int spotTime = *(int*)((char*)vehicle->scr_vehicle + 0x84);
    return spotTime != 0 && spotTime + 15000 >= level.time;
}

// ea: 0x004A7AF0 (mp_actors.o; actor.h inline)
bool Actor_IsMeleeInteractable(const actor_s* pSelf)  // ?Actor_IsMeleeInteractable@@YA_NPBUactor_s@@@Z
{
    return pSelf != nullptr && pSelf->pEnt != nullptr
        && (pSelf->pEnt->s.eFlags & 0x10000000) != 0;
}
cFreeList<trRefEntity> gRefEntFreeList;   // ?gRefEntFreeList@@3V?$cFreeList@VtrRefEntity@@@@A (g.o)
cFreeList<DObj> gDObjFreeList;            // ?gDObjFreeList@@3V?$cFreeList@VDObj@@@@A (g.o)
cFreeList<DSkel> gDSkelFreeList;           // ?gDSkelFreeList@@3V?$cFreeList@UDSkel@@@@A (g.o)
cFreeList<DSkelMax> gDSkelMaxFreeList;     // ?gDSkelMaxFreeList@@3V?$cFreeList@UDSkelMax@@@@A (g.o)
cFreeList<DSkel4> gDSkel4FreeList;         // ?gDSkel4FreeList@@3V?$cFreeList@UDSkel4@@@@A (g.o)
ae_vector<debug_sphere> debug_spheres;    // ?debug_spheres@@3V?$ae_vector@Udebug_sphere@@@@A (g.o @ 0xED2A98)
ae_vector<debug_aabb> debug_aabbs;        // ?debug_aabbs@@3V?$ae_vector@Udebug_aabb@@@@A (g.o)
ae_vector<DbLinkedHandle<EntityHandleDb, Entity>> dobjects;             // 0x12C4D14
ae_vector<DbLinkedHandle<EntityHandleDb, Entity>> del_pending_dobjects; // 0x12D4DC0
ae_vector<DbLinkedHandle<EntityHandleDb, Entity>> add_pending_dobjects; // 0x12C1FA4
const char* hintStrings[17];        // ?hintStrings (g.o .rdata)
const char* s_vehicleTypeNames[6];  // ?s_vehicleTypeNames (g.o)
const char* s_vehicleSubTypeNames[9];  // ?s_vehicleSubTypeNames (g.o)
const char* sEntryPointHintText[6];    // ?sEntryPointHintText (g.o)
char* g_scratchpadMem;              // ?g_scratchpadMem@@3PADA (game.o @ 0xEA81C0)
// --- sInst / singleton data sweep ---
void* AudioBankMgr_sInst;        // cl.o artifact (PAXA)
void* AnimBankManager_sInst;
void* CurveManager_sInst;
void* MultiplayerMgr_sInst;
void* SmokeGrenadeMgr_sInst;
void* SoundMediaMgr_sInst;
void* MusicMgr_sInst;
void* PadAliasMgr_sInst;
void* STBManager_sInst;
void* GdbFileManager_sInst;
void* BinFileManager_sInst;
void* DbTablesetMgr_sInst;
void* DynamicDecalMgr_sInst;
void* SceneManager_sInst;
void* StreamZoneManager_sInst;
void* TestFPS_sInst;
void* TimerRenderBars_sInst;
void* ScriptEventHandler_sAllocator;
PoolAllocator* WaitTilOutput_sAllocator;
PoolAllocator* EntityNotify_sAllocator;
void* EntityNotifySet_sAllocator;
void* RumbleEffectInstance_sAllocator;
void* CTitleFontRenderer_vftable;
void* cgCvarTable;
void* cvarTable;
void* cgsGlobal_media_whiteShader;
void* cgsGlobal_media_tracerShader;
void* cdscratch_vertex_format;
void* gShotProf;                 // ?gShotProf (g.o)
void* gBrocHeap;                 // ?gBrocHeap@@3PAVae_heap@@A (scr.o)
PoolAllocator* gBrocPool;                    // ?gBrocPool@@3PAVPoolAllocator@@A (scr.o @ 0x132A0E4)
PoolAllocator* gAeThreadBackupStackAllocator;  // ?gAeThreadBackupStackAllocator@@3PAVPoolAllocator@@A (core.o)
ConfigStringManager* ConfigStringManager::sInst;  // ?sInst@ConfigStringManager@@0PAV1@A
TestFPS* TestFPS::sInst;                     // ?sInst@TestFPS@@2PAV1@A
TimerRenderBars TimerRenderBars::sInst;      // ?sInst@TimerRenderBars@@0V1@A
TaskSys TaskSys::sInst;                      // ?sInst@TaskSys@@0V1@A

// ea: 0x51DDB0
// IDA initializes the task handle database, post queue, and handler array
// before any handler registration initializers run.
TaskSys::TaskSys()
{
    struct TaskDbInit {
        unsigned int mFreeBits;
        struct Element {
            void* mObject;
            int mKey;
        } mElements[32];
        void* mDebugCallback;
    };
    static_assert(sizeof(TaskDbInit) == 0x108, "TaskDb layout mismatch");

    TaskDbInit* db = reinterpret_cast<TaskDbInit*>(mHandleDb);
    db->mFreeBits = 0;
    for (int i = 0; i < 32; ++i)
    {
        db->mElements[i].mObject = nullptr;
        db->mElements[i].mKey = 1;
        db->mFreeBits |= 1u << i;
    }
    db->mDebugCallback = nullptr;

    mPostQueue.m_size = 0;
    mPostQueue.m_head = &mPostQueue.m_end;
    mPostQueue.m_end = nullptr;
    mPostQueue.m_tail = &mPostQueue.m_head;
    memset(mTaskHandlers, 0, sizeof(mTaskHandlers));
    mTaskHandlersSize = 0;
}
void* DynamicDecalMgr::sInst;                // ?sInst@DynamicDecalMgr@@2PAV1@A
namespace StatusBar {
cvar_t* sStatusBarActive = nullptr;  // ?sStatusBarActive@StatusBar@@3PAUcvar_t@@A
}
ServerTime ServerTime::sInst;  // ?sInst@ServerTime@@0V1@A (game.o)
struct ServerTime_s {
    unsigned int mNumTicksElapsed;
    int          mTickMSec;
    float        mTickDelta;
    float        mTickDeltaInv;
    float        mElapsedTime;
};
ServerTime_s ServerTime_sInst;  // ?ServerTime_sInst@@3UServerTime_s@@A (common)
DebugThread g_debugThread;     // ?g_debugThread@@3VDebugThread@@A @ 0xDEB5A0
EntityHandleDb EntityHandleDb::sInst;  // ?sInst@EntityHandleDb@@0V1@A (g.o @ 0x12BB4E8)
SmokeGrenadeMgr* SmokeGrenadeMgr::sInst;  // ?sInst@SmokeGrenadeMgr@@2PAV1@A @ 0xF049B4
DestructibleBankManager* DestructibleBankManager::sInst = NULL;  // ?sInst@DestructibleBankManager@@2PAV1@A
PhysDataBankManager* PhysDataBankManager::sInst = NULL;          // ?sInst@PhysDataBankManager@@2PAV1@A
float gStickyBoxScaleEasy = 1.25f;   // @ 0xDF5A28 (sticky aim box scale, easy)
float gStickyBoxScaleNormal = 1.0f;  // @ 0xDF5A2C
float gStickyBoxScaleHard = 0.85f;   // @ 0xDF5A30
int    iGrenadeHudTweak = 900;       // @ 0xDF8E60
int    gInteractArmsWeaponIndex;     // @ 0xF4EBF4
float  gLastGrenadeTimeLeft;         // @ 0xF4EC08
float  gCurrentGrenadeTimeLeft;      // @ 0xF4EC0C
float  ratio;                        // @ 0xF4EC10
float  player_breath_fire_delay;     // @ 0xF4EC14
vmCvar_t bg_debugWeaponState;        // @ 0xF43070
vmCvar_t bg_debugWeaponAnim;         // @ 0xF43A68
vmCvar_t bg_meleeassistrange;        // @ 0xF3E988
vmCvar_t bg_meleeassistfov;          // @ 0xF44BC8
int      g_useOnScreenSoundDebugging;  // ?g_useOnScreenSoundDebugging@@3HA @ 0xF04994
float gExtraDistanceSticky = 0.0f;   // @ 0xF4EBEC
float tangent = 0.02f;               // @ 0xDF8DC4 (sticky aim cone tangent)
float accel_slow_factor = 0.5f;      // @ 0xDF8DC8
float clostDist = 0.7f;              // @ 0xDF8DCC
float xy = 15.0f;                    // @ 0xDF8DD0
float boundingMin = 24.0f;           // @ 0xDF8DD4
float depthScale = 150.0f;           // @ 0xDF8DD8
float player_breath_hold_time = 3.0f;    // @ 0xDF6B3C (breath hold seconds)
float player_breath_gasp_time = 4.5f;    // @ 0xDF6B40
float player_breath_hold_lerp = 6.0f;    // @ 0xDF6B44
float player_breath_gasp_lerp = 4.0f;    // @ 0xDF6B48
float player_breath_gasp_scale = 1.0f;   // @ 0xDF6B4C
char* pszGameDll;                    // ?pszGameDll@@3PADA @ 0xDF5A34 (debug prints)
int   iLastState;                    // @ 0xDF8C78 (PM_Weapon_PrintWeaponState)
int   iLastAnim;                     // @ 0xDF8C7C (PM_Weapon_PrintWeaponAnim)
char  gDisableLMGHipFire;            // @ 0xF4EBFC (LMG hip-fire toggle)
float* dword_F63B8C[4 * 6320];       // ?dword_F63B8C (game.o @ 0xF63B8C)
float emissionRate_0 = 1.0f;   // @ 0xDD8254 (vehicle gunner overheat emission rate)
float gTanAimConeSpread;       // @ 0xEB1118 (g_weapon.cpp Bullet_Endpos scratch)
float max_intensity = 120.0f;  // @ 0xDD7FD4
float max_dist2 = 176400.0f;   // @ 0xDD7FD8
float radius = 30.0f;          // @ 0xDD8208 (Weapon_Revive_Test revive radius)
float radius_1 = 30.0f;        // @ 0xDD8268 (Weapon_Melee melee range)
float abovehead_tresh = 100.0f;  // @ 0xDD820C (Player_GetActivateEnt)
float decal_radius = 3.0f;     // @ 0xDD8204 (bullet impact decal radius)
float decal_radius_0 = 3.0f;   // @ 0xDD826C (Bullet_Fire_Extended decal radius)
float fudge_0 = 0.01f;         // @ 0xDD812C (push_entity AABB fudge)
float radius_0 = 15.0f;        // @ 0xDD8264 (vehicle collision push radius)
float radius_2 = 40.0f;        // @ 0xDD8260 (ClientThink tunnel radius)
float udelta = 1.0f;           // @ 0xDD81FC (VEH_Slide sphere offsets)
float fdelta = 0.6f;           // @ 0xDD8200
float veh_radius = 75.0f;      // @ 0xDD673C (VEH_GroundTrace proximity radius)
float vehicleDeadZone = 20.0f; // @ 0xDD81F8 (VEH_UpdatePO input dead zone)
float delta_yaw_vel = 0.0f;     // @ 0xEB10FC (VEH_UpdatePO rotation state)
float helmetBounce = 0.65f;    // @ 0xDD8210 (SpawnHelmet phys data)
float helmetFriction = 0.65f;  // @ 0xDD8214
float helmetMass = 0.035f;     // @ 0xDD8218
int   timeToAdd = 20000;       // @ 0xDD821C (helmet self-free time)
vmCvar_t g_weaponAmmoPools;    // @ 0x01297030
vmCvar_t g_weaponRespawn;      // @ 0x01296988
PoolAllocator* Task::sAllocator;  // ?sAllocator@Task@@2PAVPoolAllocator@@A @ 0x012F3EA8
local_physic_s s_phys;         // g_scr_vehicle.cpp static scratch
const float s_invalidAngles[3] = { 3.1415927f, 3.1415927f, 3.1415927f };  // @ 0xDD7414
float dword_DD7418 = 3.1415927f;  // @ 0xDD7418
float dword_DD741C = 3.1415927f;  // @ 0xDD741C
float minPitch = 16.0f;      // @ 0xDD822C (tank gunner min pitch)
float deltaYAWmaxs = 140.0f; // @ 0xDD8238
float deltaYAWmins = -140.0f;// @ 0xDD8244

// vehicle_rb_parameter field name -> offset table (ParseVehiclePhysicsConfigString)
vehicleVarConfig_t sVehicleVarConfig[27] = {  // @ 0xDD7608
    { "speed_max", 0x00 },           { "accel_max", 0x04 },
    { "reverse_scale", 0x08 },       { "steer_angle_max", 0x0C },
    { "steer_speed", 0x10 },         { "wheel_radius", 0x14 },
    { "susp_spring_k", 0x18 },       { "susp_damp_k", 0x1C },
    { "susp_adj", 0x20 },            { "susp_hard_limit", 0x24 },
    { "tire_fric_fwd", 0x28 },       { "tire_fric_side", 0x2C },
    { "tire_fric_brake", 0x30 },     { "tire_fric_hand_brake", 0x34 },
    { "body_mass", 0x38 },           { "mass_center_delta_x", 0x3C },
    { "mass_center_delta_y", 0x40 }, { "mass_center_delta_z", 0x44 },
    { "roll_stability", 0x48 },      { "roll_resistance", 0x4C },
    { "upright_strength", 0x50 },    { "tilt_fakey", 0x54 },
    { "peel_out_max_speed", 0x58 },  { "inertia_scale_x", 0x5C },
    { "tire_damp_coast", 0x60 },     { "tire_damp_brake", 0x64 },
    { "tire_damp_hand", 0x68 },
};

// tag hash arrays (g_scr_vehicle.cpp data) @ .data 0xEE62CC.
// Values copied from the original binary's HashString::CalcHash output
// (DJB2 with lowercase folding, verified against tlFixedString ctor 0x4A53F0).
unsigned int s_wheelTagHashes[6] = {
    0xBA21B5C2, 0xFEC735B5, 0x34790A6A,
    0xC4091F5D, 0xE8DCBD08, 0x04E325BB,
};
unsigned int s_gunnerFlashTagHashes[4] = {
    0x2F400DFB, 0x1741CD8E, 0x2F400DFC, 0x1741CDB0,
};
unsigned int s_entryPointTagHashes[6] = {
    0x0F0D7E56, 0x3E80EE43, 0x3E7B6109,
    0xE3B27AB3, 0x5A7098C6, 0xB556B6B8,
};
unsigned int s_flashTagHashes[4] = {
    0x2B100B49, 0x1741CD4A, 0x2F400DFA, 0x1741CD6C,
};
unsigned int s_seatTagHashes[11] = {
    0x88D82607, 0x9010854A, 0xACCA8E14,
    0xACCA8E15, 0xACCA8E16, 0xACCA8E17,
    0x9010854A, 0xACCA8E14, 0x88D82607,
    0x9010854A, 0xACCA8E14,
};

int g_drawDebugLos;            // @ 0xEB1108
int g_drawDebugEntityLos;      // @ 0xEB110C
int g_numLosHits;              // @ 0xEB1110
int g_numLosMisses;            // @ 0xEB1114

char line[256];                // @ 0xEF3448 (ConcatArgs scratch)
unsigned int g_HitLocConstNames[19];  // @ 0xEAEAD0 (BSS, filled at runtime)

// .data @ 0xDD7480 (verified against XBE bytes)
const char* entityTypeNames[18] = {
    "ET_GENERAL", "ET_PLAYER", "ET_ITEM", "ET_MISSILE", "ET_MOVER", "ET_PORTAL",
    "ET_INVISIBLE", "ET_SCRIPTMOVER", "ET_SOUND_BLEND", "ET_LOOP_FX", "ET_MG42",
    "ET_ACTOR", "ET_ACTOR_SPAWNER", "ET_ACTOR_CORPSE", "ET_VEHICLE",
    "ET_VEHICLE_CORPSE", "ET_VEHICLE_COLLMAP", "ET_PROP_COLLMAP",
};

// .data @ 0xDD7260 (verified against XBE bytes)
const char* gSpawnStrings[53] = {
    "sound_blend",
    "script_brushmodel",
    "script_model",
    "script_origin",
    "script_prop_collmap",
    "script_vehicle",
    "script_vehicle_collmap",
    "misc_model",
    "info_player_start",
    "info_null",
    "info_notnull",
    "info_notnull_big",
    "info_grenade_hint",
    "func_door",
    "func_static",
    "func_rotating",
    "func_bobbing",
    "func_pendulum",
    "func_group",
    "func_door_rotating",
    "trigger_use",
    "trigger_multiple",
    "trigger_friendlychain",
    "trigger_hurt",
    "trigger_once",
    "trigger_damage",
    "trigger_lookat",
    "trigger_mount",
    "light",
    "misc_mg42",
    "misc_turret",
    "props_skyportal",
    "corona",
    "spawn_intermission",
    "spawn_deathmatch",
    "spawn_teamdeathmatch",
    "spawn_ctf_allies_primary",
    "spawn_ctf_allies_secondary",
    "spawn_ctf_axis_primary",
    "spawn_ctf_axis_secondary",
    "spawn_single_ctf_allies",
    "spawn_single_ctf_axis",
    "spawn_hq_allies_primary",
    "spawn_hq_allies_secondary",
    "spawn_hq_axis_primary",
    "spawn_hq_axis_secondary",
    "hq_point",
    "spawn_dom_allies",
    "spawn_dom_axis",
    "spawn_war_allies",
    "spawn_war_axis",
    "spawn_sd_allies",
    "spawn_sd_axis",
};

HashString gSpawnHashes[53];       // @ 0xED9D30 (BSS, filled by prepare_spawns)
const char* g_key;                 // @ 0xEA6418
const char* g_value;               // @ 0xEA62F0
HashString classname_hash;         // @ 0xEE6270
bool dont_delete;                  // @ 0xEB111C
bool gCareAboutCheckpoint;         // @ 0xDD74C8
math::Position3 playerMaxs;        // @ 0xEC9640
math::Position3 playerMins;        // @ 0xEC9620
vmCvar_t g_bounds_width;           // @ 0xEA6CA8
vmCvar_t g_bounds_height_standing; // @ 0xEA7368
cFreeList<Entity> gEntFreeList;    // @ 0xF50D04

str_const_t str_const;             // @ 0xECBD30 (runtime-filled)
HashString hash_const_info_player_deathmatch;  // ?hash_const_info_player_deathmatch (g.o)
hash_const_t hash_const;  // ?hash_const@@3Uhash_const_t@@A (g.o; runtime-filled)

// .rdata @ 0xCD67AE - 2 NUL bytes then "sv_cheats" (verified vs XBE bytes)
const char defaultFileName[] = "\0\0sv_cheats";

// .rdata @ 0xD0155C / 0xD0156C (verified against XBE bytes)
extern const float colorRed[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
extern const float colorGreen[4] = { 0.0f, 1.0f, 0.0f, 1.0f };
extern const float colorBlue[4] = { 0.0f, 0.0f, 1.0f, 1.0f };       // ?colorBlue@@3QBMB (game.o @ 0x10F0A8C)
extern const float colorYellow[4] = { 1.0f, 1.0f, 0.0f, 1.0f };     // ?colorYellow@@3QBMB (game.o @ 0x10F0A9C)
extern const float colorMagenta[4] = { 1.0f, 0.0f, 1.0f, 1.0f };    // ?colorMagenta@@3QBMB (game.o @ 0x10F0ACC)
extern const float colorCyan[4] = { 0.0f, 1.0f, 1.0f, 1.0f };       // ?colorCyan@@3QBMB (game.o @ 0x10F0ADC)
extern const float colorMdCyan[4] = { 0.0f, 0.5f, 0.5f, 1.0f };     // ?colorMdCyan@@3QBMB (game.o @ 0x10F0AFC)
