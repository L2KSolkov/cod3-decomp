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
// Struct tag is clientActive_t to match the binary's ?cl@@3PAUclientActive_t@@A.
// LAYOUT-DIVERGES: this is a partial view; the real 0x18B0-byte type has
// clSnapshot_t snap at +0 and other fields at binary offsets.
// ============================================================================
struct clPlayerState {
    unsigned char _pad00[0x24];
    int pm_type;               // +0x24
    unsigned char _pad28[0x2C - 0x28];
    int pm_flags;             // +0x2C
    unsigned char _pad30[0x54 - 0x30];
    int delta_angles[3];      // +0x54
    unsigned char _pad60[0xA4 - 0x60];
    int weapon;               // +0xA4
    unsigned char _padA8[0xAC - 0xA8];
    float fWeaponPosFrac;     // +0xAC
    unsigned char _padB0[0xD0 - 0xB0];
    float viewangles[3];      // +0xD0
    unsigned char _padDC[0xF4 - 0xDC];
    int eFlags;               // +0xF4
    unsigned char _padF8[0x144 - 0xF8];
    int ammo[92];              // +0x144
    int ammoclip[92];          // +0x2B4
    unsigned char _pad42C[0x42C - 0x424];
    char weaponslots[10];      // +0x42C
    unsigned char _pad436[0x4A8 - 0x436];
    int serverCursorHint;      // +0x4A8
    unsigned char _pad4AC[0x524 - 0x4AC];
    int vehPos;                // +0x524
    int vehType;               // +0x528
    unsigned char _pad52C[0x5A0 - 0x52C];
    int prevTargetPointValid;  // +0x5A0
    unsigned char _pad5A4[0x5C8 - 0x5A4];
    unsigned int mFlags_mask;  // +0x5C8
    unsigned char _pad5CC[0x5D0 - 0x5CC];
};
static_assert(sizeof(clPlayerState) == 0x5D0, "clPlayerState size mismatch");
struct clSnapshot {
    int valid;                 // +0x00
    int snapFlags;             // +0x04
    int serverTime;            // +0x08
    int messageNum;            // +0x0C
    int cmdNum;                // +0x10
    unsigned char _pad14[0x20 - 0x14];
    clPlayerState ps;          // +0x20
    int numEntities;           // +0x5F0
    int parseEntitiesNum;      // +0x5F4
    int serverCommandNum;      // +0x5F8
    unsigned char _tail5FC[4];
};
static_assert(sizeof(clSnapshot) == 0x600, "clSnapshot size mismatch");
struct outPacket_t {
    int p_cmdNumber;   // +0x00
    int p_serverTime;  // +0x04
    int p_realtime;    // +0x08
};
static_assert(sizeof(outPacket_t) == 0x0C, "outPacket_t size mismatch");
struct clientActive_t {
    clSnapshot snap;
    int serverTime;            // +0x600
    int oldServerTime;         // +0x604
    int parseEntitiesNum;      // +0x608
    int mouseDx[2];            // +0x60C
    int mouseDy[2];            // +0x614
    int mouseIndex;            // +0x61C
    int joystickAxis[6];       // +0x620
    bool stanceHeld;           // +0x638
    unsigned char _pad639[3];
    int stancePosition;        // +0x63C
    int stanceTime;            // +0x640
    int cgameUserCmdValue;     // +0x644
    int cgameUserHoldableValue;// +0x648
    int cgameInShellshock;     // +0x64C
    float cgameSensitivity;    // +0x650
    float cgameMaxPitchSpeed;  // +0x654
    float cgameMaxYawSpeed;    // +0x658
    float cgameCurrentAimAccel;// +0x65C
    float cgameGunPitch;       // +0x660
    float cgameGunYaw;         // +0x664
    float cgameGunXOfs;        // +0x668
    float cgameGunYOfs;        // +0x66C
    float cgameGunZOfs;        // +0x670
    float viewangles[3];       // +0x674
    int serverId;              // +0x680
    int cameraMode;            // +0x684
    usercmd_s cmds[64];        // +0x688
    int cmdNumber;             // +0x1288
    int bCmdForceValues;       // +0x128C
    int iForceButtons;         // +0x1290
    int iForceWeapon;          // +0x1294
    outPacket_t outPackets[1]; // +0x1298
    unsigned char _pad12A4[0x12B0 - 0x12A4];
    clSnapshot snapshots[1];  // +0x12B0
};
static_assert(sizeof(clientActive_t) == 0x18B0, "clientActive_t size mismatch");
extern clientActive_t cl[2];

// cl[] snapshot/entity externs used by input + usercmd code
extern int anykeydown;
extern int dword_106000;
struct cvar_t;
extern cvar_t* com_cl_running;
extern const signed char ClampChar(int i);
extern bool CL_IsADS(int client);
extern int CL_StanceButtonUpdate();
extern int BG_AmmoForWeapon(int iWeapon);
extern int BG_ClipForWeapon(int iWeapon);

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
    KB_RIGHT = 1,
    KB_FORWARD = 2,
    KB_BACK = 3,
    KB_LOOKUP = 4,
    KB_LOOKDOWN = 5,
    KB_MOVELEFT = 6,
    KB_MOVERIGHT = 7,
    KB_STRAFE = 8,
    KB_SPEED = 9,
    KB_STAND = 10,
    KB_CROUCH = 11,
    KB_UP = 12,
    KB_BUTTON0 = 13,
    KB_BUTTON1 = 14,
    KB_SPRINT = 15,
    KB_BUTTON4 = 16,
    KB_BUTTON5 = 17,
    KB_ACTIVATE = 18,
    KB_ACTIVATE_RELOAD = 19,
    KB_CLASS = 20,
    KB_UNUSED21 = 21,
    KB_WBUTTON2 = 22,
    KB_RELOAD = 23,
    KB_LEAN_LEFT = 24,
    KB_LEAN_RIGHT = 25,
    KB_WBUTTON6 = 26,
    KB_WBUTTON7 = 27,
    KB_MLOOK = 28,
    KB_COUNT = 29,
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
extern unsigned int frame_msec;
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
void IN_MLookUp();
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
void IN_HoldBreathDown(int key, int time);
void IN_HoldBreathUp(int key, int time);
void IN_BinocularsDown(int key, int time);
void IN_BinocularsUp(int key, int time);
void IN_ActivateMoveUpDown(int key, int time);
void IN_ActivateMoveUpUp(int key, int time);
void IN_ActivateMeleeDown(int key, int time);
void IN_ActivateMeleeUp(int key, int time);
void IN_SetTempStanceStatus();
