// ============================================================================
// cl_input.h - client input button state (cl.o cl_input.cpp)
// Reconstructed from IDA local types. kbutton_t 0x18 verified.
// ============================================================================

#pragma once

#include <stdint.h>

// ============================================================================
// kbutton_t - single input button (0x18, verified against IDA)
// ============================================================================
struct kbutton_t {
    int down[2];        // +0x00 (two keys can map to one button)
    unsigned int downtime;  // +0x08
    unsigned int msec;      // +0x0C
    int active;             // +0x10
    int wasPressed;         // +0x14
};
static_assert(sizeof(kbutton_t) == 0x18, "kbutton_t size mismatch");

// ============================================================================
// usercmd_s - 0x30 (verified against IDA)
// ============================================================================
struct usercmd_s {
    int serverTime;      // +0x00
    int buttons;         // +0x04
    int weapon;          // +0x08
    int angles[3];       // +0x0C
    char forwardmove;    // +0x18
    char rightmove;      // +0x19
    char upmove;         // +0x1A
    float gunPitch;      // +0x1C
    float gunYaw;        // +0x20
    float gunXOfs;       // +0x24
    float gunYOfs;       // +0x28
    float gunZOfs;       // +0x2C
};
static_assert(sizeof(usercmd_s) == 0x30, "usercmd_s size mismatch");

// ============================================================================
// cl[] client snapshot + usercmd state (cl.o data; fields used by input/cmd)
// ============================================================================
struct clPlayerState {
    int eFlags;
    int pm_flags;
    int pm_type;
    int weapon;
    int weaponslots[16];
    int serverCursorHint;
    float delta_angles[3];
    float fWeaponPosFrac;
    int vehType;
    int vehPos;
    unsigned int mFlags_mask;
};
struct clSnapshot {
    clPlayerState ps;
};
struct clSnap_t {
    clSnapshot snap;
    float viewangles[3];
    bool stanceHeld;
    int stancePosition;
    int stanceTime;
    int joystickAxis[8];
    int cmdNumber;
    usercmd_s cmds[64];
    int serverTime;
    int cgameUserCmdValue;
    int cgameUserHoldableValue;
    float cgameSensitivity;
    float cgameGunPitch;
    float cgameGunYaw;
    float cgameGunXOfs;
    float cgameGunYOfs;
    float cgameGunZOfs;
    float cgameMaxPitchSpeed;
    float cgameMaxYawSpeed;
    int mouseIndex;
    int mouseDx[4];
    int mouseDy[4];
    int bCmdForceValues;
    int iForceButtons;
    int iForceWeapon;
    int cgameInShellshock;
};
extern clSnap_t cl[2];

// cl[] snapshot/entity externs used by input + usercmd code
extern int anykeydown;
extern int dword_106000;
struct cvar_t;
extern cvar_t* com_cl_running;
extern char ClampChar(int i);
extern bool CL_IsADS(int client);
extern int CL_StanceButtonUpdate();

struct weaponFileInfo_t {
    int weapClass;
    int type;
    int slot;
    int iMeleeDamage;
    int bADSOnly;
};
extern weaponFileInfo_t* BG_GetInfoForWeapon(int iWeapon);

// ============================================================================
// Button indices into the kb[] array (offsets derived from IDA addresses:
// kb base 0xF11E60; each button is 0x18 bytes)
// ============================================================================
enum {
    KB_LEFT = 0,
    KB_FORWARD = 2,
    KB_BACK = 4,
    KB_LOOKUP = 6,
    KB_LOOKDOWN = 7,
    KB_MOVELEFT = 8,
    KB_MOVERIGHT = 9,
    KB_STRAFE = 10,
    KB_SPEED = 11,
    KB_STAND = 12,
    KB_CROUCH = 13,
    KB_UP = 14,
    KB_BUTTON0 = 15,
    KB_BUTTON1 = 16,
    KB_SPRINT = 17,
    KB_BUTTON4 = 18,
    KB_BUTTON5 = 19,
    KB_ACTIVATE = 20,
    KB_ACTIVATE_RELOAD = 21,
    KB_CLASS = 22,
    KB_WBUTTON2 = 24,
    KB_RELOAD = 25,
    KB_LEAN_LEFT = 26,
    KB_LEAN_RIGHT = 27,
    KB_WBUTTON6 = 28,
    KB_WBUTTON7 = 29,
    KB_MLOOK = 33,
    KB_COUNT = 46,
};

// ============================================================================
// Globals (cl.o data)
// ============================================================================
extern kbutton_t kb[KB_COUNT];
extern kbutton_t kbss[2][KB_COUNT];
extern int cl_stance_ss[2];
extern int cl_altFireButtonDown_ss[2];
extern int cl_grenadeButtonDown_ss[2];
extern int cl_aADS[2];
extern int currCl;
extern int com_frameTime;
extern int frame_msec;
extern int dword_106000;  // EF_* flags mask used by stance checks

// ============================================================================
// Functions (cl.o cl_input.cpp)
// ============================================================================
void IN_KeyDown(kbutton_t* b, int key, unsigned int time);
void IN_KeyUp(kbutton_t* b, unsigned int key, int time);
float CL_KeyState(kbutton_t* key);
int CL_InitButtons();
void CL_BackUpKeys();
void CL_RecallKeys();
void IN_MLookDown();
int IN_MLookUp();
void IN_UpDown(int key, int time);
void IN_UpUp(int key, int time);
void IN_DownDown(int key, int time);
void IN_DownUp(int key, int time);
void IN_LeftDown(int key, int time);
void IN_LeftUp(int key, int time);
void IN_RightDown(int key, int time);
void IN_RightUp(int key, int time);
void IN_ForwardDown(int key, int time);
void IN_ForwardUp(int key, int time);
void IN_BackDown(int key, int time);
void IN_BackUp(int key, int time);
void IN_LookupDown(int key, int time);
void IN_LookupUp(int key, int time);
void IN_LookdownDown(int key, int time);
void IN_LookdownUp(int key, int time);
void IN_MoveleftDown(int key, int time);
void IN_MoveleftUp(int key, int time);
void IN_MoverightDown(int key, int time);
void IN_MoverightUp(int key, int time);
void IN_SpeedDown(int key, int time);
void IN_SpeedUp(int key, int time);
void IN_StrafeDown(int key, int time);
void IN_StrafeUp(int key, int time);
void IN_Button0Down(int key, int time);
void IN_Button0Up(int key, int time);
void IN_Button1Down(int key, int time);
void IN_Button1Up(int key, int time);
void IN_Button4Down(int key, int time);
void IN_Button4Up(int key, int time);
void IN_Button5Down(int key, int time);
void IN_Button5Up(int key, int time);
void IN_ActivateDown(int key, int time);
void IN_ActivateUp(int key, int time);
void IN_ActivateReloadDown(int key, int time);
void IN_ActivateReloadUp(int key, int time);
void IN_Wbutton0Down(int key, int time);
void IN_Wbutton0Up(int key, int time);
void IN_Wbutton2Down(int key, int time);
void IN_Wbutton2Up(int key, int time);
void IN_ReloadDown(int key, int time);
void IN_ReloadUp(int key, int time);
void IN_LeanLeftDown(int key, int time);
void IN_LeanLeftUp(int key, int time);
void IN_LeanRightDown(int key, int time);
void IN_LeanRightUp(int key, int time);
void IN_Wbutton6Down(int key, int time);
void IN_Wbutton6Up(int key, int time);
void IN_Wbutton7Down(int key, int time);
void IN_Wbutton7Up(int key, int time);
void IN_ToggleADS();
void IN_AnalogStickLeanDown();
void IN_AnalogStickLeanUp();
void IN_EnableAsserts();
void IN_TogglePaused();
int IN_CenterView();
void IN_GoStandDown(int key, int time);
void IN_GoStandUp(int key, int time);
int IN_GoCrouch();
int IN_GoProne();
int IN_ToggleCrouch();
int IN_ToggleProne();
int IN_RaiseStance();
int IN_LowerStance();
int IN_Stance_Down();
int IN_Stance_Up();
void IN_ClassButtonDown(int key, int time);
void IN_ClassButtonUp(int key, int time);
void IN_LeanLeftSwitchNextDown(int key, int time);
void IN_LeanLeftSwitchNextUp(int key, int time);
void IN_LeanRightSwitchNextDown(int key, int time);
void IN_LeanRightSwitchNextUp(int key, int time);
void IN_SmokeGrenadeAttackDown(int key, int time);
void IN_SmokeGrenadeAttackUp(int key, int time);
void IN_GrenadeAttackDown(int key, int time);
void IN_GrenadeAttackUp(int key, int time);
void IN_ButtonDown(int key, int time);
void IN_ButtonUp(int key, int time);
void IN_SprintDown(int key, int time);
void IN_SprintUp(int key, int time);
void IN_SprintBreathDown(int key, int time);
void IN_SprintBreathUp(int key, int time);
void IN_HoldBreathDown();
void IN_HoldBreathUp();
void IN_BinocularsDown(int key, int time);
void IN_BinocularsUp(int key, int time);
void IN_ActivateMoveUpDown(int key, int time);
void IN_ActivateMoveUpUp(int key, int time);
void IN_ActivateMeleeDown(int key, int time);
void IN_ActivateMeleeUp(int key, int time);
void IN_SetTempStanceStatus();
