// ============================================================================
// cg_main.cpp - client game main entry + register/console plumbing (cg.o)
// ============================================================================

#include "game/cg/cg_local.h"
#include "game/cvar_types.h"
#include "game/game_types.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Minimal view of SoundDevice (full class in game/sv/sv_stubs.h).
class SoundDevice { public: static SoundDevice* sInst; void FrameAdvance(float delta); };

class MusicMgr {
public:
    void Play(const char* name);
    void Stop(float fadeOutTime);
};


// Minimal view of RumbleManager (full class in core/core_systems.h).
class RumbleManager {
public:
    static RumbleManager* Inst(int instance);  // ?Inst@RumbleManager@@SAPAV1@H@Z (g.o)
};


extern const char* CL_GetConfigString(int index);  // ?CL_GetConfigString@@YAPBDH@Z (cl.o)


extern int currCl;
extern int dword_F6A290[4 * 802];
extern int dword_F6295C[4 * 1580];
extern int dword_F64184[4 * 1580];
extern int dword_F64188[4 * 1580];
extern int dword_F64140[4 * 1580];
extern int dword_F63BB4[4 * 1580];
extern int dword_F63BAC[4 * 1580];
extern int dword_F63CF4[4 * 1580];
extern int dword_F641E0[4 * 1580];
extern int dword_F641E4[4 * 1580];
extern int dword_F641E8[4 * 1580];
extern int dword_F641EC[4 * 1580];
enum netsrc_t {
    NS_CLIENT = 0,
    NS_SERVER = 1,
};

extern void CL_AddCgameCommand(const char* cmdName, void (*function)());
extern char* Info_ValueForKey(const char* s, const char* key);
extern void Cmd_ArgvBuffer(int arg, char* buffer, int bufferLength);
extern int Q_stricmp(const char* s1, const char* s2);
extern void Com_Printf(const char* fmt, ...);
struct vmCvar_t;
extern void Cvar_Register(vmCvar_t* vmCvar, const char* varName,
                          const char* defaultValue, int flags);
extern void Cvar_Update(vmCvar_t* vmCvar);
extern void Cvar_Set(const char* var_name, const char* value);
extern void Cvar_VariableStringBuffer(const char* var_name, char* buffer,
                                      int bufsize);
extern const char defaultFileName[];
extern void Cvar_VMSet(vmCvar_t* vmCvar, const char* value);
enum errorParm_t;
extern void Com_Error(errorParm_t code, const char* fmt, ...);
extern void CG_Init();
extern void CG_InitServerCommandHashVals();
extern void CG_InitLocalEntities();
extern void CG_ParseFog();
extern void CG_ParseObjectiveChange(int iNum);
extern void CG_CloseScriptMenu();
extern void CG_RegisterServerShader(int num);
extern const char* CG_ConfigString(unsigned int index);
extern int trap_R_RegisterShaderNoMip(const char* name, int imagetype);
extern void CG_RegisterWeapon(int weaponNum);
extern void CG_FreeWeapons();
extern void FX_InitFX();
extern void SCR_UpdateScreen();
extern void* RE_RegisterModel(void* result, const char* name, int pakId,
                              int imageType);
extern TPakId CurPakId();
extern void CG_ConfigStringModifiedInternal(int num);
extern void CG_StartShakeCamera(float p, int duration, const float* src,
                                float radius, int client);
extern void CG_DrawActiveFrame(int serverTime, int demoPlayback,
                               int cubemapShot, int cubemapSize,
                               int animFrametime);
extern void CG_DObjCalcPose(Entity* entity, void* obj, int* partBits);
extern void CG_SaveEntity();
extern void CG_LoadEntity();
extern void CG_General(Entity* entity);
extern void CG_LockLightingOrigin(Entity* ent, refEntity_t* refEnt);
extern void RE_AddRefEntityToScene(void* ent, int iCellNum);
extern void AnglesToAxis(const float* const angles,
                         float (*const axis)[3]);
extern void SoundDevice_DampenAllSounds(void* sInst, float level);
extern void SoundDevice_UndampenAllSounds(void* sInst);
extern void SoundDevice_StopAllSounds(void* sInst);
extern void* MusicMgr_sInst;
extern void Com_FreeWeaponInfoMemory(int iSource, int bRestart);
extern void RumbleManager_Reset(void* mgr);
extern nglTexture* GetTextureData(const char* name, int image_type,
                                   const char* fromPak);
extern nglTexture* cgsGlobal_media_whiteShader;
extern void* cgsGlobal_media_tracerShader;
extern int CG_RegisterItems();
extern int Cmd_Argc();
extern void Cbuf_AddText(const char* text);
extern void Cbuf_ExecuteText(int exec_when, const char* text);
extern char* va(const char* fmt, ...);
extern const char* SEH_LocalizeTextMessage(const char* message,
                                           const char* messageType);
enum print_msg_type_t;
extern void CL_ConsolePrint(print_msg_type_t type, const char* text,
                            int duration, int linewidth, int flags);
extern void CG_PriorityCenterPrint(const char* str, float y, int charWidth,
                                   int priority);
extern void CG_GameMessage(const char* msg, int flags);
extern void CG_ObjMessage(const char* msg);
extern void CG_OpenScriptMenu();
extern void CG_LocalSound();
extern int dword_F610E8;
extern int dword_F610EC;
extern int dword_F610F0;
extern int dword_F610F4;
extern int dword_F610F8;
extern int dword_F610FC;
extern int dword_F61100;
extern int dword_F61104;
extern int dword_F61108;
extern int dword_F6110C;
extern int dword_F61110;
extern int dword_F61114;
extern int dword_F61118;
extern int dword_F6111C;
extern int dword_F61120;
extern int dword_F61124;
extern int dword_F61128;
extern int dword_F6112C;
extern int dword_F61130;
extern int dword_F62948[4 * 1580];
extern int dword_F64154[4 * 1580];
extern int dword_F64158[4 * 1580];
extern int dword_F6415C[4 * 1580];
extern int dword_F64160[4 * 1580];
// IDA gitem_s layout: 0x34-byte records at bg_itemlist (0xF51EC0).
struct gitem_s
{
    unsigned int classname_hash;
    char* classname;
    char* pickup_sound;
    char* world_model[2];
    char* icon;
    char* ammoicon;
    char* pickup_name;
    int quantity;
    int giType;
    int giTag;
    int giAmmoIndex;
    int giClipIndex;
};
extern struct gitem_s bg_itemlist[];
extern int cg_aWeaponSelect[4];
extern int cg_aWeaponSelectTime[4];
itemInfo_t cg_items[256];             // ?cg_items@@3PAUitemInfo_t@@A (cg.o)
weaponInfo_s cg_weapons[92];          // ?cg_weapons@@3PAUweaponInfo_s@@A (cg.o)
extern void* cgCvarTable;      // cvarTable_t[170]
extern void* cvarTable;        // cvarTable_t[170]

struct cg_t {
    unsigned char data[0x18B0];
};
cg_t cg[2] = {};  // ?cg@@3PAUcg_t@@A (cg.o @ 0x1351E40)

vmCvar_t cg_thirdPerson;               // ?cg_thirdPerson@@3UvmCvar_t@@A (cg.o @ 0x134B400)
vmCvar_t cg_thirdPersonLock;           // ?cg_thirdPersonLock@@3UvmCvar_t@@A (cg.o @ 0x134A8D0)
vmCvar_t cg_thirdPersonRange;          // ?cg_thirdPersonRange@@3UvmCvar_t@@A (cg.o @ 0x134E6E8)
vmCvar_t cg_thirdPersonAngle;          // ?cg_thirdPersonAngle@@3UvmCvar_t@@A (cg.o @ 0x1350908)
vmCvar_t cg_widescreen;                // ?cg_widescreen@@3UvmCvar_t@@A (cg.o @ 0x134C188)
vmCvar_t cg_norender;                  // ?cg_norender@@3UvmCvar_t@@A (cg.o @ 0x134E148)
vmCvar_t cg_redFlashTime;              // ?cg_redFlashTime@@3UvmCvar_t@@A (cg.o @ 0x134B9A8)
vmCvar_t cg_centertime;                 // ?cg_centertime@@3UvmCvar_t@@A
vmCvar_t cg_camerashake;               // ?cg_camerashake@@3UvmCvar_t@@A (cg.o @ 0x134CC40)
vmCvar_t cg_drawGun;                   // ?cg_drawGun@@3UvmCvar_t@@A (cg.o @ 0x134CE80)
vmCvar_t cg_mpDebugAnimEntity;         // ?cg_mpDebugAnimEntity@@3UvmCvar_t@@A (cg.o @ 0x134C850)
vmCvar_t hud_healthOverlay_pulseStart;                 // ?hud_healthOverlay_pulseStart@@3UvmCvar_t@@A (cg.o @ 0x134BBE8)
vmCvar_t hud_healthOverlay_phaseOne_pulseDuration;     // ?hud_healthOverlay_phaseOne_pulseDuration@@3UvmCvar_t@@A (cg.o @ 0x134EFD0)
vmCvar_t hud_healthOverlay_phaseTwo_toAlphaMultiplier; // ?hud_healthOverlay_phaseTwo_toAlphaMultiplier@@3UvmCvar_t@@A (cg.o @ 0x134B1C0)
vmCvar_t hud_healthOverlay_phaseTwo_pulseDuration;     // ?hud_healthOverlay_phaseTwo_pulseDuration@@3UvmCvar_t@@A (cg.o @ 0x134EB70)
vmCvar_t hud_healthOverlay_phaseThree_toAlphaMultiplier; // ?hud_healthOverlay_phaseThree_toAlphaMultiplier@@3UvmCvar_t@@A (cg.o @ 0x1350278)
vmCvar_t hud_healthOverlay_phaseThree_pulseDuration;     // ?hud_healthOverlay_phaseThree_pulseDuration@@3UvmCvar_t@@A (cg.o @ 0x1350428)
vmCvar_t hud_healthOverlay_phaseEnd_toAlpha;             // ?hud_healthOverlay_phaseEnd_toAlpha@@3UvmCvar_t@@A (cg.o @ 0x134F758)
vmCvar_t hud_healthOverlay_phaseEnd_pulseDuration;       // ?hud_healthOverlay_phaseEnd_pulseDuration@@3UvmCvar_t@@A (cg.o @ 0x134CD60)
vmCvar_t hud_healthOverlay_regenPauseTime;               // ?hud_healthOverlay_regenPauseTime@@3UvmCvar_t@@A (cg.o @ 0x134E388)
vmCvar_t fs_debug_vm;                  // ?fs_debug_vm@@3UvmCvar_t@@A (cg.o @ 0x134FB48)
vmCvar_t cg_altTankCam;                // ?cg_altTankCam@@3UvmCvar_t@@A (cg.o @ 0x134B130)
vmCvar_t cg_bobAmplitudeDucked;        // ?cg_bobAmplitudeDucked@@3UvmCvar_t@@A (cg.o @ 0x134AD50)
vmCvar_t cg_bobAmplitudeProne;         // ?cg_bobAmplitudeProne@@3UvmCvar_t@@A (cg.o @ 0x134E268)
vmCvar_t cg_bobAmplitudeStanding;      // ?cg_bobAmplitudeStanding@@3UvmCvar_t@@A (cg.o @ 0x134DE78)
vmCvar_t cg_bobMax;                     // ?cg_bobMax@@3UvmCvar_t@@A
vmCvar_t cg_draw2D;                    // ?cg_draw2D@@3UvmCvar_t@@A (cg.o @ 0x134CA00)
vmCvar_t cg_drawStatus;                // ?cg_drawStatus@@3UvmCvar_t@@A (cg.o @ 0x134BD98)
vmCvar_t cg_forceCrosshair;            // ?cg_forceCrosshair@@3UvmCvar_t@@A (cg.o @ 0x134DD58)
vmCvar_t cg_fov;                       // ?cg_fov@@3UvmCvar_t@@A (cg.o @ 0x13504B8)
vmCvar_t cg_viewsize;                  // ?cg_viewsize@@3UvmCvar_t@@A (cg.o)
vmCvar_t cg_letterbox;                 // ?cg_letterbox@@3UvmCvar_t@@A (cg.o)
vmCvar_t cg_gameBoldMessageWidth;      // ?cg_gameBoldMessageWidth@@3UvmCvar_t@@A (cg.o @ 0x134F908)
vmCvar_t cg_gameMessageWidth;          // ?cg_gameMessageWidth@@3UvmCvar_t@@A (cg.o @ 0x134E658)
vmCvar_t cg_hudAlpha;                  // ?cg_hudAlpha@@3UvmCvar_t@@A (cg.o @ 0x134F180)
vmCvar_t cg_nopredict;                 // ?cg_nopredict@@3UvmCvar_t@@A (cg.o @ 0x134F2A0)
vmCvar_t cg_showmiss;                  // ?cg_showmiss@@3UvmCvar_t@@A
vmCvar_t cg_errorDecay;                // ?cg_errorDecay@@3UvmCvar_t@@A
vmCvar_t cg_shellshockblur;            // ?cg_shellshockblur@@3UvmCvar_t@@A (cg.o @ 0x134C8E0)
vmCvar_t cg_skybox;                    // ?cg_skybox@@3UvmCvar_t@@A (cg.o @ 0x134C970)
vmCvar_t cg_stanceTemp;                // ?cg_stanceTemp@@3UvmCvar_t@@A (cg.o @ 0x134FC68)
vmCvar_t cg_weaponCycleDelay;          // ?cg_weaponCycleDelay@@3UvmCvar_t@@A (cg.o @ 0x134E418)
vmCvar_t pmove_fixed;                  // ?pmove_fixed@@3UvmCvar_t@@A (cg.o @ 0x134FCF8)
vmCvar_t pmove_msec;                   // ?pmove_msec@@3UvmCvar_t@@A (cg.o @ 0x134BB58)
vmCvar_t cg_drawPosition;              // ?cg_drawPosition@@3UvmCvar_t@@A (cg.o @ 0x134ADE0)
vmCvar_t cg_developer;                 // ?cg_developer@@3UvmCvar_t@@A (cg.o @ 0x1350548)
vmCvar_t cg_deadscreen_backdrop;         // ?cg_deadscreen_backdrop@@3UvmCvar_t@@A (cg.o @ 0x134B888)
vmCvar_t cg_deadscreen_levelname;        // ?cg_deadscreen_levelname@@3UvmCvar_t@@A (cg.o @ 0x134B2E0)
vmCvar_t cg_victoryscreen_backdrop;      // ?cg_victoryscreen_backdrop@@3UvmCvar_t@@A (cg.o @ 0x134F3F8)
vmCvar_t cg_victoryscreen_levelname;     // ?cg_victoryscreen_levelname@@3UvmCvar_t@@A (cg.o @ 0x134B5B0)
vmCvar_t cg_crosshairAlpha;              // ?cg_crosshairAlpha@@3UvmCvar_t@@A (cg.o @ 0x134CB20)
vmCvar_t cg_crosshairDynamic;            // ?cg_crosshairDynamic@@3UvmCvar_t@@A (cg.o @ 0x134EA50)
vmCvar_t cg_hudCompassSize;              // ?cg_hudCompassSize@@3UvmCvar_t@@A (cg.o @ 0x134FA28)
vmCvar_t cg_drawTimer;                   // ?cg_drawTimer@@3UvmCvar_t@@A (cg.o @ 0x134C458)
vmCvar_t cg_minicon;                     // ?cg_minicon@@3UvmCvar_t@@A (cg.o @ 0x134D0C0)
vmCvar_t cg_subtitles;                   // ?cg_subtitles@@3UvmCvar_t@@A (cg.o @ 0x134C2A8)
vmCvar_t cg_drawpaused;                  // ?cg_drawpaused@@3UvmCvar_t@@A (cg.o @ 0x134FE18)
vmCvar_t cg_railTrailTime;               // ?cg_railTrailTime@@3UvmCvar_t@@A (cg.o @ 0x134C218)
vmCvar_t cg_tracerChance;                // ?cg_tracerChance@@3UvmCvar_t@@A (cg.o @ 0x134E0B8)
vmCvar_t cg_addentities;                 // ?cg_addentities@@3UvmCvar_t@@A (cg.o @ 0x134B760)
vmCvar_t cg_debugEvents;                 // ?cg_debugEvents@@3UvmCvar_t@@A (cg.o @ 0x134E8A0)
vmCvar_t cg_hudCompassSpringyPointers;   // ?cg_hudCompassSpringyPointers@@3UvmCvar_t@@A (cg.o @ 0x134BFD8)
vmCvar_t cg_hudDamageIconTime;           // ?cg_hudDamageIconTime@@3UvmCvar_t@@A (cg.o @ 0x134FEA8)
vmCvar_t cg_viewKickScale;               // ?cg_viewKickScale@@3UvmCvar_t@@A (cg.o @ 0x134F638)
vmCvar_t cg_viewKickMax;                 // ?cg_viewKickMax@@3UvmCvar_t@@A (cg.o @ 0x134DCC8)
vmCvar_t cg_shock_viewKickFadeTime;      // ?cg_shock_viewKickFadeTime@@3UvmCvar_t@@A (cg.o @ 0x134AF00)
vmCvar_t cg_shock_viewKickPeriod;        // ?cg_shock_viewKickPeriod@@3UvmCvar_t@@A (cg.o @ 0x134DDE8)
vmCvar_t cg_shock_viewKickRadius;        // ?cg_shock_viewKickRadius@@3UvmCvar_t@@A (cg.o @ 0x134F368)
vmCvar_t cg_shock_sound;                 // ?cg_shock_sound@@3UvmCvar_t@@A (cg.o @ 0x134C580)
vmCvar_t cg_shock_soundFadeInTime;       // ?cg_shock_soundFadeInTime@@3UvmCvar_t@@A (cg.o @ 0x134B520)
vmCvar_t cg_shock_soundFadeOutTime;      // ?cg_shock_soundFadeOutTime@@3UvmCvar_t@@A (cg.o @ 0x1350758)
vmCvar_t cg_shock_soundLoopFadeTime;     // ?cg_shock_soundLoopFadeTime@@3UvmCvar_t@@A (cg.o @ 0x134E930)
vmCvar_t cg_shock_soundLoopEndDelay;     // ?cg_shock_soundLoopEndDelay@@3UvmCvar_t@@A (cg.o @ 0x134E778)
vmCvar_t cg_shock_soundRoomType;         // ?cg_shock_soundRoomType@@3UvmCvar_t@@A (cg.o @ 0x1350308)
vmCvar_t cg_shock_soundWetLevel;         // ?cg_shock_soundWetLevel@@3UvmCvar_t@@A (cg.o @ 0x134BF48)
vmCvar_t cg_shock_soundModEndDelay;      // ?cg_shock_soundModEndDelay@@3UvmCvar_t@@A (cg.o @ 0x134CF10)
vmCvar_t cg_shock_volume_auto;           // ?cg_shock_volume_auto@@3UvmCvar_t@@A (cg.o @ 0x134B490)
vmCvar_t cg_shock_volume_menu;           // ?cg_shock_volume_menu@@3UvmCvar_t@@A (cg.o @ 0x134ED20)
vmCvar_t cg_shock_volume_weapon;         // ?cg_shock_volume_weapon@@3UvmCvar_t@@A (cg.o @ 0x134A9F0)
vmCvar_t cg_shock_volume_voice;          // ?cg_shock_volume_voice@@3UvmCvar_t@@A (cg.o @ 0x134AB10)
vmCvar_t cg_shock_volume_item;           // ?cg_shock_volume_item@@3UvmCvar_t@@A (cg.o @ 0x134FD88)
vmCvar_t cg_shock_volume_body;           // ?cg_shock_volume_body@@3UvmCvar_t@@A (cg.o @ 0x134E028)
vmCvar_t cg_shock_volume_local;          // ?cg_shock_volume_local@@3UvmCvar_t@@A (cg.o @ 0x134DF08)
vmCvar_t cg_shock_volume_music;          // ?cg_shock_volume_music@@3UvmCvar_t@@A (cg.o @ 0x134E9C0)
vmCvar_t cg_shock_volume_announcer;      // ?cg_shock_volume_announcer@@3UvmCvar_t@@A (cg.o @ 0x134B250)
vmCvar_t cg_shock_volume_shellshock;     // ?cg_shock_volume_shellshock@@3UvmCvar_t@@A (cg.o @ 0x134AF90)
vmCvar_t cg_shock_mouse;                 // ?cg_shock_mouse@@3UvmCvar_t@@A (cg.o @ 0x1350398)
vmCvar_t cg_shock_mouse_fadeTime;        // ?cg_shock_mouse_fadeTime@@3UvmCvar_t@@A (cg.o @ 0x134BC78)
vmCvar_t cg_shock_mouse_maxpitchspeed;   // ?cg_shock_mouse_maxpitchspeed@@3UvmCvar_t@@A (cg.o @ 0x134EF40)
vmCvar_t cg_shock_mouse_maxyawspeed;     // ?cg_shock_mouse_maxyawspeed@@3UvmCvar_t@@A (cg.o @ 0x134A960)
vmCvar_t cg_shock_mouse_sensitivityscale; // ?cg_shock_mouse_sensitivityscale@@3UvmCvar_t@@A (cg.o @ 0x13507E8)
vmCvar_t cg_bobWeaponLag;                // ?cg_bobWeaponLag@@3UvmCvar_t@@A (cg.o @ 0x134FAB8)
vmCvar_t cg_bobWeaponAmplitude;          // ?cg_bobWeaponAmplitude@@3UvmCvar_t@@A (cg.o @ 0x134EC90)
vmCvar_t cg_bobWeaponMax;                // ?cg_bobWeaponMax@@3UvmCvar_t@@A (cg.o @ 0x134F7E8)
vmCvar_t cg_bobWeaponRollAmplitude;      // ?cg_bobWeaponRollAmplitude@@3UvmCvar_t@@A (cg.o @ 0x134DB28)
vmCvar_t cg_gun_move_minspeed;           // ?cg_gun_move_minspeed@@3UvmCvar_t@@A (cg.o @ 0x134C068)
vmCvar_t cg_gun_move_f;                  // ?cg_gun_move_f@@3UvmCvar_t@@A (cg.o @ 0x134EDB0)
vmCvar_t cg_gun_move_r;                  // ?cg_gun_move_r@@3UvmCvar_t@@A (cg.o @ 0x134EC00)
vmCvar_t cg_gun_move_u;                  // ?cg_gun_move_u@@3UvmCvar_t@@A (cg.o @ 0x1350638)
vmCvar_t cg_gun_move_rate;               // ?cg_gun_move_rate@@3UvmCvar_t@@A (cg.o @ 0x134D950)
vmCvar_t cg_gun_ofs_f;                   // ?cg_gun_ofs_f@@3UvmCvar_t@@A (cg.o @ 0x1350998)
vmCvar_t cg_gun_ofs_r;                   // ?cg_gun_ofs_r@@3UvmCvar_t@@A (cg.o @ 0x134E808)
vmCvar_t cg_gun_ofs_u;                   // ?cg_gun_ofs_u@@3UvmCvar_t@@A (cg.o @ 0x134C0F8)
vmCvar_t cg_gun_rot_minspeed;            // ?cg_gun_rot_minspeed@@3UvmCvar_t@@A (cg.o @ 0x134FF38)
vmCvar_t cg_gun_rot_y;                   // ?cg_gun_rot_y@@3UvmCvar_t@@A (cg.o @ 0x134E538)
vmCvar_t cg_gun_rot_p;                   // ?cg_gun_rot_p@@3UvmCvar_t@@A (cg.o @ 0x134F5A8)
vmCvar_t cg_gun_rot_r;                   // ?cg_gun_rot_r@@3UvmCvar_t@@A (cg.o @ 0x134CDF0)
vmCvar_t cg_gun_rot_rate;                // ?cg_gun_rot_rate@@3UvmCvar_t@@A (cg.o @ 0x134B918)
vmCvar_t cg_viewKickDeflectTime;         // ?cg_viewKickDeflectTime@@3UvmCvar_t@@A (cg.o @ 0x134AA80)
vmCvar_t cg_viewKickReturnTime;          // ?cg_viewKickReturnTime@@3UvmCvar_t@@A (cg.o @ 0x134BEB8)

struct consoleCommand_t {
    const char* cmd;
    void (*function)();
};

// console command table (commandsList / off_D0CD44)
static const consoleCommand_t sCommandsList[] = {
    {"viewpos", nullptr},
    {nullptr, nullptr},
};

extern char buffer_0[256];
static int (*syscall_)(int, ...) = nullptr;
int (*syscall)(int, ...) = nullptr;  // ?syscall@@3P6AHHZZA (cg.o @ 0xDF9D70)

// ea: 0x0068C860
void cg_dllEntry(int (*syscallptr)(int, ...))
{
    syscall_ = syscallptr;
}

// ea: 0x0068C8F0
int CG_UI_Popup(const char*)
{
    return 1;
}

// ea: 0x00687C30
void CG_InitConsoleCommands()
{
    const char* cmd = "viewpos";
    if (cmd != nullptr)
    {
        const consoleCommand_t* v1 = sCommandsList;
        do
        {
            CL_AddCgameCommand(cmd, v1->function);
            cmd = v1[1].cmd;
            ++v1;
        } while (cmd != nullptr);
    }
    CL_AddCgameCommand("startCamera", nullptr);
    CL_AddCgameCommand("stopCamera", nullptr);
    CL_AddCgameCommand("setCameraOrigin", nullptr);
    CL_AddCgameCommand("ai_history", nullptr);
    CL_AddCgameCommand("levelshot", nullptr);
    CL_AddCgameCommand("stats", nullptr);
}

// ea: 0x0068B3D0
void CG_RegisterCvars()
{
    for (int i = 0; i < 170; ++i)
    {
        // cvar table rows: { vmCvar*, name, defaultValue, flags }
        void** row = &((void**)cgCvarTable)[4 * i];
        Cvar_Register((vmCvar_t*)row[0], (const char*)row[1],
                      (const char*)row[2], (int)row[3]);
    }
}

// ea: 0x0068B400
int CG_UpdateCvars()
{
    int result = 0;
    for (int i = 0; i < 170; ++i)
    {
        void** row = &((void**)cvarTable)[4 * i];
        if (row != nullptr)
        {
            if (row[0] != nullptr)
                Cvar_Update((vmCvar_t*)row[0]);
        }
        else
        {
            CG_ASSERT("cv", "c:\\cod\\code\\game\\cg_main.cpp", 924);
        }
        result = i;
    }
    return result;
}

// ea: 0x0068B480
void CG_Printf(const char* msg, ...)
{
    char text[1024];
    va_list ap;
    va_start(ap, msg);
    vsprintf(text, msg, ap);
    va_end(ap);
    Com_Printf(text);
}

// ea: 0x0068B4B0
void CG_Error(const char* msg, ...)
{
    char text[1024];
    va_list ap;
    va_start(ap, msg);
    vsprintf(text, msg, ap);
    va_end(ap);
    Com_Error((errorParm_t)2 /* ERR_DROP */, text);
}

// ea: 0x0068B580
char* CG_Argv(int arg)
{
    Cmd_ArgvBuffer(arg, buffer_0, 256);
    return buffer_0;
}

// ea: 0x0068BC40
void CG_LocalSound()
{
    const int argc = Cmd_Argc();
    if (argc == 2)
    {
        Cmd_ArgvBuffer(1, buffer_0, 256);
        const int index = atoi(buffer_0);
        if (index <= 0 || index > 64)
        {
            CG_Printf("ERROR: CG_LocalSound called with index %i (should be in range[1,%i])\n",
                      index, 64);
        }
        else
        {
            // The release routine intentionally only resolves the config
            // string here; playback is handled by the client sound system.
            (void)CG_ConfigString(static_cast<unsigned int>(index + 161));
        }
    }
    else
    {
        CG_Printf("ERROR: CG_LocalSound called with %i args (should be 2)\n",
                  argc);
    }
}

// ea: 0x00698090
void CG_OpenScriptMenu()
{
    Cmd_ArgvBuffer(1, buffer_0, 256);
    const int index = atoi(buffer_0);
    if (index != 0)
    {
        Com_Printf("Server tried to open a bad script menu index: %i\n",
                   index);
        Cbuf_AddText(va("cmd mr %i bad\n", index));
        return;
    }

    const char* menu = CG_ConfigString(627);
    if (*menu != '\0')
    {
        if (Cmd_Argc() > 2 && CG_Argv(2) != nullptr)
            (void)CG_Argv(2);
        Cvar_Set("ui_newScriptMenu", menu);
        Cvar_Set("ui_newScriptMenuIndex", va("%i", 0));
    }
    else
    {
        Com_Printf("Server tried to open a non-loaded script menu index: %i\n",
                   0);
        Cbuf_AddText(va("cmd mr %i bad\n", 0));
    }
}

// ea: 0x006A3860
void CG_ServerCommand()
{
    Cmd_ArgvBuffer(0, buffer_0, 256);
    if (buffer_0[0] == '\0')
        return;

    const unsigned int hash = HashString::CalcHash(buffer_0);
    if (hash == static_cast<unsigned int>(dword_F610EC))
    {
        Cmd_ArgvBuffer(1, buffer_0, 256);
        CG_ConfigStringModifiedInternal(atoi(buffer_0));
    }
    else if (hash == static_cast<unsigned int>(dword_F610F8))
    {
        Cmd_ArgvBuffer(1, buffer_0, 256);
        const char* message =
            SEH_LocalizeTextMessage(buffer_0, "bold game message");
        CL_ConsolePrint(static_cast<print_msg_type_t>(4), message, 0,
                        cg_gameBoldMessageWidth.integer, 0);
    }
    else if (hash == static_cast<unsigned int>(dword_F610F0))
    {
        const char* message =
            SEH_LocalizeTextMessage(CG_Argv(1), "server print");
        CG_Printf("%s\n", message);
    }
    else if (hash == static_cast<unsigned int>(dword_F6111C))
    {
        SoundDevice_DampenAllSounds(
            SoundDevice::sInst, static_cast<float>(atof(CG_Argv(1))));
    }
    else if (hash == static_cast<unsigned int>(dword_F61120))
    {
        const int duration = atoi(CG_Argv(3));
        const int start = atoi(CG_Argv(2));
        const int alphaByte = static_cast<int>(atof(CG_Argv(1)) * 255.0);
        const int base = 1580 * currCl;
        reinterpret_cast<float*>(&dword_F64154[base])[0] =
            static_cast<float>(alphaByte) * 0.0039215689f;
        dword_F6415C[base] = start;
        dword_F64160[base] = duration;
        if (start + duration <= cgGlobal.time)
            dword_F64158[base] = dword_F64154[base];
    }
    else if (hash == static_cast<unsigned int>(dword_F610E8))
    {
        CG_PriorityCenterPrint(CG_Argv(1), 360.0f, 8, 0);
    }
    else if (hash == static_cast<unsigned int>(dword_F610F4))
    {
        const char* message =
            SEH_LocalizeTextMessage(CG_Argv(1), "game message");
        CG_GameMessage(message, 0);
    }
    else if (hash == static_cast<unsigned int>(dword_F610FC))
    {
        // object_update is deliberately a no-op in the release client.
    }
    else if (hash == static_cast<unsigned int>(dword_F61100))
    {
        const char* message =
            SEH_LocalizeTextMessage(CG_Argv(1), "game message");
        CG_ObjMessage(message);
    }
    else if (hash == static_cast<unsigned int>(dword_F61104)
             || hash == static_cast<unsigned int>(dword_F61108))
    {
        SoundDevice_UndampenAllSounds(SoundDevice::sInst);
    }
    else if (hash == static_cast<unsigned int>(dword_F6110C))
    {
        dword_F62948[1580 * currCl] = 1;
    }
    else if (hash == static_cast<unsigned int>(dword_F61110))
    {
        Cbuf_ExecuteText(0, va("screenshotHigh savegame %s", CG_Argv(1)));
    }
    else if (hash == static_cast<unsigned int>(dword_F61114))
    {
        reinterpret_cast<MusicMgr*>(MusicMgr_sInst)->Play(CG_Argv(1));
    }
    else if (hash == static_cast<unsigned int>(dword_F61118))
    {
        reinterpret_cast<MusicMgr*>(MusicMgr_sInst)->Stop(0.0f);
    }
    else if (hash == static_cast<unsigned int>(dword_F61124))
    {
        CG_ParseFog();
    }
    else if (hash == static_cast<unsigned int>(dword_F61128))
    {
        CG_LocalSound();
    }
    else if (hash == static_cast<unsigned int>(dword_F6112C))
    {
        CG_OpenScriptMenu();
    }
    else if (hash == static_cast<unsigned int>(dword_F61130))
    {
        CG_CloseScriptMenu();
    }
    else
    {
        CG_Printf("Unknown client game command: %s\n", buffer_0);
    }
}

// ea: 0x0068B600
int CG_StartAmbient()
{
    const char* ConfigString = CL_GetConfigString(3);
    Info_ValueForKey(ConfigString, "n");
    const char* v1 = Info_ValueForKey(ConfigString, "t");
    return atoi(v1);
}

// ea: 0x0068B630
char* CG_GetMenuBuffer()
{
    CG_ASSERT("0", "c:\\cod\\code\\game\\cg_main.cpp", 1252);
    return nullptr;
}

// ea: 0x0068B680
int CG_Asset_Parse()
{
    return 0;
}

// ea: 0x0068B690
void CG_ParseMenu()
{
    CG_ASSERT("0", "c:\\cod\\code\\game\\cg_main.cpp", 1289);
}

// ea: 0x0068B6E0
int CG_Load_Menu()
{
    CG_ASSERT("0", "c:\\cod\\code\\game\\cg_main.cpp", 1323);
    return 0;
}

// ea: 0x0068B730
void CG_LoadMenus()
{
    CG_ASSERT("0", "c:\\cod\\code\\game\\cg_main.cpp", 1356);
}

// ea: 0x0068B8E0
void CG_AssetCache()
{
}

// ea: 0x0068BBB0
void CG_RegisterServerShaders()
{
    for (int i = 725; i < 980; ++i)
        CG_RegisterServerShader(i);
}

// ea: 0x0068A3A0
void CG_RegisterServerShader(int num)
{
    if (num < 724 || num >= 980)
        return;
    const char* shader = CG_ConfigString(static_cast<unsigned int>(num));
    if (shader != nullptr && shader[0] != '\0')
        trap_R_RegisterShaderNoMip(shader, 5);
}

// ea: 0x0068BBD0
void CG_CloseScriptMenu()
{
    Cvar_Set("ui_scriptMenu", "");
    Cvar_Set("ui_scriptMenuIndex", "-1");
    Cvar_Set("ui_newScriptMenu", "");
    Cvar_Set("ui_newScriptMenuIndex", "-1");
    Cvar_Set("ui_waitingScriptMenu", "");
    Cvar_Set("ui_waitingScriptMenuIndex", "-1");
    Cvar_Set("ui_waitingScriptMenuNoMouse", "0");
}

// ea: 0x00698190
void CG_CheckOpenWaitingScriptMenu()
{
    char value[256];
    Cvar_VariableStringBuffer("ui_waitingScriptMenu", value,
                              static_cast<int>(sizeof(value)));
    if (value[0] == '\0')
        return;

    Cvar_Set("ui_newScriptMenu", value);
    Cvar_VariableStringBuffer("ui_waitingScriptMenuIndex", value,
                              static_cast<int>(sizeof(value)));
    Cvar_Set("ui_newScriptMenuIndex", value);
    Cvar_VariableStringBuffer("ui_waitingScriptMenuNoMouse", value,
                              static_cast<int>(sizeof(value)));
    (void)atoi(value);

    Cvar_Set("ui_waitingScriptMenu", defaultFileName);
    Cvar_Set("ui_waitingScriptMenuIndex", "-1");
    Cvar_Set("ui_waitingScriptMenuNoMouse", "0");
}

// ea: 0x0068F5E0
void CG_RegisterItemVisuals(int itemNum)
{
    unsigned char* item = &((unsigned char*)cg_items)[8 * itemNum];
    if (item[0] == 0)
    {
        gitem_s* gitem = &bg_itemlist[itemNum];
        const char* icon = gitem->icon;
        item[0] = 0;
        if (icon != nullptr)
            *(void**)(item + 4) = GetTextureData(icon, 5, "mp_frontEnd");
        if (gitem->giType == 1 /* IT_WEAPON */)
            CG_RegisterWeapon(gitem->giTag);
        item[0] = 1;
    }
}

// ea: 0x00697B50
void CG_RegisterGraphics()
{
    FX_InitFX();
    SCR_UpdateScreen();

    cgsGlobal.media.tracerShader =
        GetTextureData("gfx/misc/tracer", 0, "mp_frontEnd");
    cgsGlobal_media_tracerShader = cgsGlobal.media.tracerShader;
    cgsGlobal.media.damageShader =
        GetTextureData("hit_direction", 0, "mp_frontEnd");
    cgsGlobal.media.lowHealthOverlay =
        GetTextureData("overlay_low_health", 0, "mp_frontEnd");
    cgsGlobal.media.checkbox_clear =
        GetTextureData("ui/assets/checkbox_clear", 0, "mp_frontEnd");
    cgsGlobal.media.checkbox_checked =
        GetTextureData("ui/assets/checkbox_checked", 0, "mp_frontEnd");
    cgsGlobal.media.checkbox_fail =
        GetTextureData("ui/assets/checkbox_fail", 0, "mp_frontEnd");
    cgsGlobal.media.backTileShader =
        GetTextureData("gfx/2d/backtile", 0, "mp_frontEnd");
    cgsGlobal.media.noWeapon = GetTextureData("noweapon", 0, "mp_frontEnd");

    cgsGlobal.media.mYourTeamIcons[0] =
        GetTextureData("i_head_rank_1_w.tga", 0, "mp_frontEnd");
    cgsGlobal.media.mYourTeamIcons[1] =
        GetTextureData("i_head_rank_2_w.tga", 0, "mp_frontEnd");
    cgsGlobal.media.mYourTeamIcons[2] =
        GetTextureData("i_head_rank_3_w.tga", 0, "mp_frontEnd");
    cgsGlobal.media.mYourTeamIcons[4] =
        GetTextureData("goldflag.tga", 0, "mp_frontEnd");
    cgsGlobal.media.mYourTeamIcons[3] =
        GetTextureData("i_head_VOIP_w.tga", 0, "mp_frontEnd");
    cgsGlobal.media.mOtherTeamIcons[0] =
        GetTextureData("rank1_red.tga", 0, "mp_frontEnd");
    cgsGlobal.media.mOtherTeamIcons[1] =
        GetTextureData("rank2_red.tga", 0, "mp_frontEnd");
    cgsGlobal.media.mOtherTeamIcons[2] =
        GetTextureData("rank3_red.tga", 0, "mp_frontEnd");
    cgsGlobal.media.mOtherTeamIcons[3] =
        GetTextureData("redflag.tga", 0, "mp_frontEnd");
    cgsGlobal.media.mStatusDead =
        GetTextureData("status_dead.tga", 0, "mp_frontEnd");
    cgsGlobal.media.mMedicDeadWorld =
        GetTextureData("i_downed_friend_w", 0, "mp_frontEnd");

    SCR_UpdateScreen();
    memset(cg_items, 0, sizeof(cg_items));
    memset(cg_weapons, 0, sizeof(cg_weapons));
    SCR_UpdateScreen();
    CG_RegisterItems();
    SCR_UpdateScreen();

    IVPointerRaw result = {};
    for (unsigned int index = 34; index < 0x400; ++index) {
        const char* configString = CL_GetConfigString(index);
        if (configString == nullptr || *configString == '\0')
            break;
        IVPointerRaw* model = static_cast<IVPointerRaw*>(RE_RegisterModel(
            &result, configString, static_cast<int>(CurPakId()), 7));
        if (model != nullptr)
            cgsGlobal.gameModels[index - 33] = *model;
    }
    SCR_UpdateScreen();
}

// ea: 0x0068F640
int CG_RegisterItems()
{
    char items[260];
    strcpy(items, CL_GetConfigString(8));
    unsigned char* v1 = &((unsigned char*)cg_items)[8];
    gitem_s* p_item = &bg_itemlist[1];
    for (int v0 = 1; v0 < 137; ++v0)
    {
        int v3 = items[v0 / 4];
        int v4 = v3 > 57 ? v3 - 87 : v3 - 48;
        if (((1 << (v0 & 3)) & v4) != 0 && v1[0] == 0)
        {
            const char* v5 = p_item->icon;
            v1[0] = 0;
            if (v5 != nullptr)
                *(void**)(v1 + 4) = GetTextureData(v5, 5, "mp_frontEnd");
            if (p_item->giType == 1 /* IT_WEAPON */)
                CG_RegisterWeapon(p_item->giTag);
            v1[0] = 1;
        }
        v1 += 8;
        ++p_item;
    }
    return 137;
}

// ea: 0x0068A910
void CG_ItemPickup(int itemNum)
{
    if (bg_itemlist[itemNum].giType == 1 /* IT_WEAPON */
        && cg_aWeaponSelect[currCl] == 0)
    {
        cg_aWeaponSelectTime[currCl] = cgGlobal.time;
        cg_aWeaponSelect[currCl] = bg_itemlist[itemNum].giTag;
    }
}

// ea: 0x00694850
int CG_ConsoleCommand()
{
    Cmd_ArgvBuffer(0, buffer_0, 256);
    if (!sCommandsList[0].cmd)
        return 0;
    int v0 = 0;
    const consoleCommand_t* v1 = sCommandsList;
    while (Q_stricmp(buffer_0, v1->cmd))
    {
        const char* cmd = v1[1].cmd;
        ++v1;
        ++v0;
        if (!cmd)
            return 0;
    }
    if (sCommandsList[v0].function)
        sCommandsList[v0].function();
    return 1;
}

// ea: 0x00689910
void CG_General(Entity* entity)
{
    if (entity->s.eFlags >= 0)
    {
        void* mDObj = entity->mDObj;
        if (mDObj != nullptr)
        {
            refEntity_t* RefEntity = (refEntity_t*)&entity->GetRefEntity();
            RefEntity->origin[0] = entity->s.lerpOrigin.v.m128_f32[0];
            RefEntity->origin[1] = entity->s.lerpOrigin.v.m128_f32[1];
            RefEntity->origin[2] = entity->s.lerpOrigin.v.m128_f32[2];
            RefEntity->oldorigin[0] = entity->s.lerpOrigin.v.m128_f32[0];
            RefEntity->oldorigin[1] = entity->s.lerpOrigin.v.m128_f32[1];
            RefEntity->oldorigin[2] = entity->s.lerpOrigin.v.m128_f32[2];
            AnglesToAxis((const float*)&entity->s.lerpAngles,
                         RefEntity->axis);
            RefEntity->obj = mDObj;
            RefEntity->entity = entity;
            RefEntity->reType = 1;  // RT_XMODEL
            CG_LockLightingOrigin(entity, RefEntity);
            RE_AddRefEntityToScene(RefEntity, -1);
        }
    }
}

// ea: 0x0068A870
void CG_SaveEntity()
{
    CG_ASSERT("0", "c:\\cod\\code\\game\\cg_ent.cpp", 1415);
}

// ea: 0x0068A8C0
void CG_LoadEntity()
{
    CG_ASSERT("0", "c:\\cod\\code\\game\\cg_ent.cpp", 1436);
}

// ea: 0x00697DE0
void CG_Shutdown()
{
    re.TrackStatistics(nullptr);
    SoundDevice_DampenAllSounds(SoundDevice::sInst, 0.0f);
    CG_FreeWeapons();
    Com_FreeWeaponInfoMemory(2, 0);
    memset(cg, 0, sizeof(cg_t));
    void* v0 = RumbleManager::Inst(currCl);
    RumbleManager_Reset(v0);
}

// ea: 0x006A3700
void CG_MapInit(int restart)
{
    CG_InitServerCommandHashVals();
    int v1 = 1580 * currCl;
    memset(&cg[currCl], 0, sizeof(cg_t));
    dword_F6295C[v1] = restart;
    dword_F64184[v1] = 0;
    dword_F64188[v1] = 0;
    CG_InitLocalEntities();
    CG_ParseFog();
    for (int i = 16; i < 32; ++i)
        CG_ParseObjectiveChange(i);
    const char* ConfigString = CL_GetConfigString(9);
    float restarta = (float)atof(ConfigString);
    re.SetCullDist(restarta);
    const char* v4 = CL_GetConfigString(11);
    float v5 = (float)atof(v4);
    dword_F64140[1580 * currCl] = *(int*)&v5;
    SoundDevice_StopAllSounds(SoundDevice::sInst);
    SoundDevice::sInst->FrameAdvance( 0.0f);
    const char* v7 = CL_GetConfigString(3);
    Info_ValueForKey(v7, "n");
    const char* v8 = Info_ValueForKey(v7, "t");
    atoi(v8);
    Cvar_VMSet(&cg_thirdPerson, "0");
    Cvar_Set("ui_scriptMenuAllowResponse", "0");
    CG_CloseScriptMenu();
    Cvar_Set("ui_scriptMenuAllowResponse", "1");
    int v9 = 1580 * currCl;
    dword_F63BB4[v9] = -1;
    dword_F63BAC[v9] = -1;
    dword_F63CF4[v9] = 1;
    dword_F641E4[0] = 0;
    dword_F641E8[0] = 0;
    dword_F641E0[0] = -1;
    dword_F641EC[0] = 0;
}

// ea: 0x006B0EB0
int cg_vmMain(int command, int arg0, void* arg1, int* arg2, int arg3,
              int arg4)
{
    int v6 = 0;
    int result;
    switch (command)
    {
    case 0:
        CG_Init();
        result = 0;
        break;
    case 1:
        CG_Shutdown();
        result = 0;
        break;
    case 2:
        result = CG_ConsoleCommand();
        break;
    case 3:
        CG_DrawActiveFrame(arg0, (int)arg1, arg2 != nullptr ? *arg2 : 0,
                           arg3, arg4);
        result = 0;
        break;
    case 4:
    case 5:
    case 7:
    case 13:
    case 14:
    case 15:
    case 16:
    case 17:
    case 18:
        goto LABEL_707;
    case 8:
        result = 0;
        break;
    case 9:
        CG_DObjCalcPose((Entity*)arg0, arg1, arg2);
        result = 0;
        break;
    case 11:
        CG_StartShakeCamera(*(float*)&arg0, (int)arg1, (const float*)arg2,
                            *(float*)&arg3, currCl);
        result = 0;
        break;
    case 12:
        result = 1;
        break;
    case 19:
        if (dword_F6A290[0] == 2)
        {
            currCl = NS_CLIENT;
            CG_MapInit(1);
        }
        currCl = NS_CLIENT;
        result = 0;
        break;
    case 20:
        CG_ConfigStringModifiedInternal(arg0);
        result = 0;
        break;
    case 21:
        cgGlobal.time = arg0;
        cgGlobal.oldTime = arg0;
        result = 0;
        break;
    case 22:
        CG_SaveEntity();
        result = 0;
        break;
    case 23:
        CG_LoadEntity();
        result = 0;
        break;
    default:
        CG_Error("cg_vmMain: unknown command %i", command);
        v6 = -1;
    LABEL_707:
        result = v6;
        break;
    }
    return result;
}
