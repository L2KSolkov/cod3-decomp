// ============================================================================
// cl_input.cpp - client input button state (cl.o cl_input.cpp)
// 70+ functions, verified against IDA (release map offsets + 0x40C000 = VA).
// ============================================================================

#include "cl_input.h"

#include <string.h>

// ============================================================================
// Externs (cross-object; core.o / game.o / cg.o)
// ============================================================================
extern void Com_Printf(const char* fmt, ...);
extern int com_frameTime;
extern int frame_msec;
extern int currCl;
extern int dword_106000;

namespace AeAssert {
extern bool gAssertsEnabled;
enum ECoderId { COD3 = 0 };
extern ECoderId gCurrentAuthor;
extern const char* gCurrentFile;
extern int gCurrentLine;
extern const char* gCurrentExpr;
bool IsIgnored();
bool Warning(const char* fmt, ...);
bool Assert(const char* fmt, ...);
}

#define ASSERT(expr, file, line)                                          \
    do {                                                                  \
        AeAssert::gCurrentAuthor = AeAssert::COD3;                        \
        AeAssert::gCurrentFile = (file);                                  \
        AeAssert::gCurrentLine = (line);                                  \
        AeAssert::gCurrentExpr = (expr);                                  \
        if (!AeAssert::IsIgnored()                                        \
            && AeAssert::Assert("old cod assert"))                        \
            __debugbreak();                                               \
    } while (0)

extern void CG_WeaponSlot_f(int iSlot);
extern void CG_NextWeapon_f();
extern void CG_PrevWeapon_f();
extern bool CL_IsADS(int client);
extern void GamePause_SetGamePaused(int client, bool paused);
extern bool GamePause_IsGamePaused(int client);
extern bool gGrenadeCanBePickedUp;
extern bool gCookingLiveGrenade;
extern int cl_analogStickLean_integer;
extern int cl_freelook_integer;
extern int cl_binocButtonDown_integer;

struct EntityView {
    void* client;
};
extern EntityView* EntityManager_GetPlayer(void* inst, int idx);
extern void* EntityManager_sInst;

// ============================================================================
// Globals (cl.o data)
// ============================================================================
kbutton_t kb[KB_COUNT];
kbutton_t kbss[2][KB_COUNT];
int cl_stance_ss[2];
int cl_altFireButtonDown_ss[2];
int cl_grenadeButtonDown_ss[2];
int cl_aADS[2];

// ============================================================================
// IN_KeyDown / IN_KeyUp / CL_KeyState (core button machinery)
// ============================================================================

// ea: 0x52A5D0
void IN_KeyDown(kbutton_t* b, int key, unsigned int time)
{
    if (key != b->down[0])
    {
        int v3 = b->down[1];
        if (key != v3)
        {
            if (b->down[0] != 0)
            {
                if (v3 != 0)
                {
                    Com_Printf("Three keys down for a button!\n");
                    return;
                }
                b->down[1] = key;
            }
            else
            {
                b->down[0] = key;
            }
            if (b->active == 0)
            {
                b->downtime = time;
                b->active = 1;
                b->wasPressed = 1;
            }
        }
    }
}

// ea: 0x52A630
void IN_KeyUp(kbutton_t* b, unsigned int key, int time)
{
    (void)time;
    if (key == (unsigned int)-1)
    {
        b->down[1] = 0;
        b->down[0] = 0;
        b->active = 0;
        return;
    }
    if (key >= 0x100)
    {
        ASSERT("key >= 0 && key < 256", "c:\\cod\\code\\game\\cl_input.cpp", 165);
    }
    if ((unsigned int)b->down[0] >= 0x100)
    {
        ASSERT("down[0] < 256", "c:\\cod\\code\\game\\cl_input.cpp", 172);
    }
    if (b->down[0] == (int)key)
    {
        b->down[0] = 0;
    }
    else if (b->down[1] == (int)key)
    {
        b->down[1] = 0;
    }
    else
    {
        return;
    }
    if (b->down[0] == 0 && b->down[1] == 0)
    {
        b->active = 0;
        b->wasPressed = 0;
    }
}

// ea: 0x52A790
float CL_KeyState(kbutton_t* key)
{
    int val = (int)key->msec;
    int active = key->active;
    key->msec = 0;
    if (active != 0)
    {
        unsigned int downtime = key->downtime;
        if (downtime != 0)
            val += com_frameTime - (int)downtime;
        else
            val = com_frameTime;
        key->downtime = (unsigned int)com_frameTime;
    }
    float v4 = (float)val / (float)frame_msec;
    if (v4 < 0.0f)
        return 0.0f;
    if (v4 > 1.0f)
        return 1.0f;
    return v4;
}

// ============================================================================
// Button toggles
// ============================================================================

// ea: 0x52A590
int CL_InitButtons()
{
    memset(kbss, 0, sizeof(kbss));
    memset(kb, 0, sizeof(kb));
    cl_stance_ss[0] = 0;
    cl_altFireButtonDown_ss[0] = 0;
    cl_grenadeButtonDown_ss[0] = 0;
    return 0;
}

// ea: 0x52A530
void CL_BackUpKeys()
{
    memcpy(kbss[currCl], kb, sizeof(kbss[currCl]));
}

// ea: 0x52A560
void CL_RecallKeys()
{
    memcpy(kb, kbss[currCl], sizeof(kbss[currCl]));
}

// ea: 0x52A5C0
void IN_MLookDown()
{
    kb[KB_MLOOK].active = 1;
}

// ea: 0x530510
int IN_MLookUp()
{
    kb[KB_MLOOK].active = 0;
    if (cl_freelook_integer == 0)
    {
        cl[currCl].viewangles[0] = 0.0f - (cl[currCl].snap.ps.delta_angles[0] * 0.0054931641f);
    }
    return 0;
}

// ea: 0x52A860
void IN_UpDown(int key, int time)
{
    IN_KeyDown(&kb[KB_UP], key, (unsigned int)time);
    if (kb[KB_WBUTTON6].active == 0 && kb[KB_CROUCH].active == 0)
    {
        int v2 = cl_stance_ss[currCl];
        if (v2 <= 1)
        {
            if (v2 <= 0)
                IN_KeyDown(&kb[KB_STAND], key, (unsigned int)time);
            else
                cl_stance_ss[currCl] = 0;
        }
        else
        {
            cl_stance_ss[currCl] = 1;
        }
    }
}

// ea: 0x52A8E0
void IN_UpUp(int key, int time)
{
    IN_KeyUp(&kb[KB_UP], (unsigned int)key, time);
    IN_KeyUp(&kb[KB_STAND], (unsigned int)key, time);
}

// ea: 0x52A910
void IN_DownDown(int key, int time)
{
    IN_KeyDown(&kb[KB_CROUCH], key, (unsigned int)time);
    cl_stance_ss[currCl] = kb[KB_WBUTTON6].active != 0 || kb[KB_CROUCH].active != 0;
}

// ea: 0x52A960
void IN_DownUp(int key, int time)
{
    IN_KeyUp(&kb[KB_CROUCH], (unsigned int)key, time);
    cl_stance_ss[currCl] = kb[KB_WBUTTON6].active != 0 || kb[KB_CROUCH].active != 0;
}

// ea: 0x52A9B0
void IN_LeftDown(int key, int time)
{
    // inlined IN_KeyDown(kb, ...)
    if (key != kb[KB_LEFT].down[0] && key != kb[KB_LEFT].down[1])
    {
        if (kb[KB_LEFT].down[0] != 0)
        {
            if (kb[KB_LEFT].down[1] != 0)
            {
                Com_Printf("Three keys down for a button!\n");
                return;
            }
            kb[KB_LEFT].down[1] = key;
        }
        else
        {
            kb[KB_LEFT].down[0] = key;
        }
        if (kb[KB_LEFT].active == 0)
        {
            kb[KB_LEFT].downtime = (unsigned int)time;
            kb[KB_LEFT].active = 1;
            kb[KB_LEFT].wasPressed = 1;
        }
    }
}

// ea: 0x52AA10
void IN_LeftUp(int key, int time)
{
    IN_KeyUp(&kb[KB_LEFT], (unsigned int)key, time);
}

// ea: 0x52AA30
void IN_RightDown(int key, int time)
{
    IN_KeyDown(&kb[KB_LEFT + 1], key, (unsigned int)time);
}

// ea: 0x52AA50
void IN_RightUp(int key, int time)
{
    IN_KeyUp(&kb[KB_LEFT + 1], (unsigned int)key, time);
}

// ea: 0x52AA70
void IN_ForwardDown(int key, int time)
{
    IN_KeyDown(&kb[KB_FORWARD], key, (unsigned int)time);
}

// ea: 0x52AA90
void IN_ForwardUp(int key, int time)
{
    IN_KeyUp(&kb[KB_FORWARD], (unsigned int)key, time);
}

// ea: 0x52AAB0
void IN_BackDown(int key, int time)
{
    IN_KeyDown(&kb[KB_BACK], key, (unsigned int)time);
}

// ea: 0x52AAD0
void IN_BackUp(int key, int time)
{
    IN_KeyUp(&kb[KB_BACK], (unsigned int)key, time);
}

// ea: 0x52AAF0
void IN_LookupDown(int key, int time)
{
    IN_KeyDown(&kb[KB_LOOKUP], key, (unsigned int)time);
}

// ea: 0x52AB10
void IN_LookupUp(int key, int time)
{
    IN_KeyUp(&kb[KB_LOOKUP], (unsigned int)key, time);
}

// ea: 0x52AB30
void IN_LookdownDown(int key, int time)
{
    IN_KeyDown(&kb[KB_LOOKDOWN], key, (unsigned int)time);
}

// ea: 0x52AB50
void IN_LookdownUp(int key, int time)
{
    IN_KeyUp(&kb[KB_LOOKDOWN], (unsigned int)key, time);
}

// ea: 0x52AB70
void IN_MoveleftDown(int key, int time)
{
    IN_KeyDown(&kb[KB_MOVELEFT], key, (unsigned int)time);
}

// ea: 0x52AB90
void IN_MoveleftUp(int key, int time)
{
    IN_KeyUp(&kb[KB_MOVELEFT], (unsigned int)key, time);
}

// ea: 0x52ABB0
void IN_MoverightDown(int key, int time)
{
    IN_KeyDown(&kb[KB_MOVERIGHT], key, (unsigned int)time);
}

// ea: 0x52ABD0
void IN_MoverightUp(int key, int time)
{
    IN_KeyUp(&kb[KB_MOVERIGHT], (unsigned int)key, time);
}

// ea: 0x52ABF0
void IN_SpeedDown(int key, int time)
{
    IN_KeyDown(&kb[KB_SPEED], key, (unsigned int)time);
}

// ea: 0x52AC10
void IN_SpeedUp(int key, int time)
{
    IN_KeyUp(&kb[KB_SPEED], (unsigned int)key, time);
}

// ea: 0x52AC30
void IN_StrafeDown(int key, int time)
{
    IN_KeyDown(&kb[KB_STRAFE], key, (unsigned int)time);
}

// ea: 0x52AC50
void IN_StrafeUp(int key, int time)
{
    IN_KeyUp(&kb[KB_STRAFE], (unsigned int)key, time);
}

// ea: 0x534100
void IN_Button0Down(int key, int time)
{
    if (cl_binocButtonDown_integer != 0)
        IN_BinocularsDown(key, time);
    IN_KeyDown(&kb[KB_BUTTON0], key, (unsigned int)time);
}

// ea: 0x52AC70
void IN_Button0Up(int key, int time)
{
    IN_KeyUp(&kb[KB_BUTTON0], (unsigned int)key, time);
}

// ea: 0x52AC90
void IN_Button1Down(int key, int time)
{
    IN_KeyDown(&kb[KB_BUTTON1], key, (unsigned int)time);
}

// ea: 0x52ACB0
void IN_Button1Up(int key, int time)
{
    IN_KeyUp(&kb[KB_BUTTON1], (unsigned int)key, time);
}

// ea: 0x52ACD0
void IN_Button4Down(int key, int time)
{
    IN_KeyDown(&kb[KB_BUTTON4], key, (unsigned int)time);
}

// ea: 0x52ACF0
void IN_Button4Up(int key, int time)
{
    IN_KeyUp(&kb[KB_BUTTON4], (unsigned int)key, time);
}

// ea: 0x52AD10
void IN_Button5Down(int key, int time)
{
    IN_KeyDown(&kb[KB_BUTTON5], key, (unsigned int)time);
}

// ea: 0x52AD30
void IN_Button5Up(int key, int time)
{
    IN_KeyUp(&kb[KB_BUTTON5], (unsigned int)key, time);
}

// ea: 0x52AD50
void IN_ActivateDown(int key, int time)
{
    IN_KeyDown(&kb[KB_ACTIVATE], key, (unsigned int)time);
}

// ea: 0x52AD70
void IN_ActivateUp(int key, int time)
{
    IN_KeyUp(&kb[KB_ACTIVATE], (unsigned int)key, time);
}

// ea: 0x52AD90
void IN_ActivateReloadDown(int key, int time)
{
    IN_KeyDown(&kb[KB_ACTIVATE_RELOAD], key, (unsigned int)time);
}

// ea: 0x52ADB0
void IN_ActivateReloadUp(int key, int time)
{
    IN_KeyUp(&kb[KB_ACTIVATE_RELOAD], (unsigned int)key, time);
}

// ea: 0x52ADD0
void IN_Wbutton0Down(int key, int time)
{
    IN_KeyDown(&kb[KB_CLASS], key, (unsigned int)time);
}

// ea: 0x52ADF0
void IN_Wbutton0Up(int key, int time)
{
    IN_KeyUp(&kb[KB_CLASS], (unsigned int)key, time);
}

// ea: 0x52AE10
void IN_Wbutton2Down(int key, int time)
{
    IN_KeyDown(&kb[KB_WBUTTON2], key, (unsigned int)time);
}

// ea: 0x52AE30
void IN_Wbutton2Up(int key, int time)
{
    IN_KeyUp(&kb[KB_WBUTTON2], (unsigned int)key, time);
}

// ea: 0x52AE50
void IN_ReloadDown(int key, int time)
{
    IN_KeyDown(&kb[KB_RELOAD], key, (unsigned int)time);
}

// ea: 0x52AE70
void IN_ReloadUp(int key, int time)
{
    IN_KeyUp(&kb[KB_RELOAD], (unsigned int)key, time);
}

// ea: 0x52AE90
void IN_LeanLeftDown(int key, int time)
{
    IN_KeyDown(&kb[KB_LEAN_LEFT], key, (unsigned int)time);
}

// ea: 0x52AEB0
void IN_LeanLeftUp(int key, int time)
{
    IN_KeyUp(&kb[KB_LEAN_LEFT], (unsigned int)key, time);
}

// ea: 0x52AED0
void IN_LeanRightDown(int key, int time)
{
    IN_KeyDown(&kb[KB_LEAN_RIGHT], key, (unsigned int)time);
}

// ea: 0x52AEF0
void IN_LeanRightUp(int key, int time)
{
    IN_KeyUp(&kb[KB_LEAN_RIGHT], (unsigned int)key, time);
}

// ea: 0x52AF10
void IN_Wbutton6Down(int key, int time)
{
    IN_KeyDown(&kb[KB_WBUTTON6], key, (unsigned int)time);
    cl_stance_ss[currCl] = kb[KB_WBUTTON6].active != 0 || kb[KB_CROUCH].active != 0;
}

// ea: 0x52AF60
void IN_Wbutton6Up(int key, int time)
{
    IN_KeyUp(&kb[KB_WBUTTON6], (unsigned int)key, time);
    cl_stance_ss[currCl] = kb[KB_WBUTTON6].active != 0 || kb[KB_CROUCH].active != 0;
}

// ea: 0x52AFB0
void IN_Wbutton7Down(int key, int time)
{
    IN_KeyDown(&kb[KB_WBUTTON7], key, (unsigned int)time);
}

// ea: 0x52AFD0
void IN_Wbutton7Up(int key, int time)
{
    IN_KeyUp(&kb[KB_WBUTTON7], (unsigned int)key, time);
}

// ea: 0x52AFF0
void IN_ToggleADS()
{
    cl_aADS[currCl] = cl_aADS[currCl] == 0;
}

// ea: 0x52B9F0
void IN_AnalogStickLeanDown()
{
    cl_analogStickLean_integer = 1;
}

// ea: 0x52BA00
void IN_AnalogStickLeanUp()
{
    cl_analogStickLean_integer = 0;
}

// ea: 0x52C2B0
void IN_EnableAsserts()
{
    AeAssert::gAssertsEnabled = true;
}

// ea: 0x52B510
void IN_TogglePaused()
{
    bool IsGamePaused = GamePause_IsGamePaused(currCl);
    GamePause_SetGamePaused(currCl, !IsGamePaused);
}

// ea: 0x52B090
int IN_CenterView()
{
    cl[currCl].viewangles[0] = 0.0f - (cl[currCl].snap.ps.delta_angles[0] * 0.0054931641f);
    return 6320 * currCl;
}

// ea: 0x52B2D0
void IN_GoStandDown(int key, int time)
{
    if ((dword_106000 & cl[currCl].snap.ps.eFlags) == 0
        || cl[currCl].snap.ps.vehType == 2 && cl[currCl].snap.ps.vehPos == 1)
    {
        IN_KeyDown(&kb[KB_UP], key, (unsigned int)time);
        if (cl_stance_ss[currCl] != 0)
        {
            if (kb[KB_WBUTTON6].active == 0 && kb[KB_CROUCH].active == 0)
                cl_stance_ss[currCl] = 0;
        }
        else
        {
            IN_KeyDown(&kb[KB_STAND], key, (unsigned int)time);
        }
    }
}

// ea: 0x52B360
void IN_GoStandUp(int key, int time)
{
    IN_KeyUp(&kb[KB_UP], (unsigned int)key, time);
    IN_KeyUp(&kb[KB_STAND], (unsigned int)key, time);
}

// ea: 0x52B280
int IN_GoCrouch()
{
    if ((dword_106000 & cl[currCl].snap.ps.eFlags) == 0
        || cl[currCl].snap.ps.vehType == 2 && cl[currCl].snap.ps.vehPos == 1)
    {
        if (kb[KB_WBUTTON6].active == 0)
        {
            if (kb[KB_CROUCH].active == 0)
                cl_stance_ss[currCl] = 1;
        }
    }
    return 6320 * currCl;
}

// ea: 0x52B230
int IN_GoProne()
{
    if ((dword_106000 & cl[currCl].snap.ps.eFlags) == 0
        || cl[currCl].snap.ps.vehType == 2 && cl[currCl].snap.ps.vehPos == 1)
    {
        if (kb[KB_WBUTTON6].active == 0)
        {
            if (kb[KB_CROUCH].active == 0)
                cl_stance_ss[currCl] = 2;
        }
    }
    return 6320 * currCl;
}

// ea: 0x52B170
int IN_ToggleCrouch()
{
    if ((dword_106000 & cl[currCl].snap.ps.eFlags) == 0
        || cl[currCl].snap.ps.vehType == 2 && cl[currCl].snap.ps.vehPos == 1)
    {
        if (kb[KB_WBUTTON6].active == 0)
        {
            if (kb[KB_CROUCH].active == 0)
            {
                cl_stance_ss[currCl] = cl_stance_ss[currCl] != 1;
            }
        }
    }
    return 6320 * currCl;
}

// ea: 0x52B1D0
int IN_ToggleProne()
{
    if ((dword_106000 & cl[currCl].snap.ps.eFlags) == 0
        || cl[currCl].snap.ps.vehType == 2 && cl[currCl].snap.ps.vehPos == 1)
    {
        if (kb[KB_WBUTTON6].active == 0)
        {
            if (kb[KB_CROUCH].active == 0)
            {
                cl_stance_ss[currCl] = cl_stance_ss[currCl] == 2 ? 0 : 2;
            }
        }
    }
    return 6320 * currCl;
}

// ea: 0x52B130
int IN_RaiseStance()
{
    if (kb[KB_WBUTTON6].active == 0)
    {
        if (kb[KB_CROUCH].active == 0)
        {
            int result = cl_stance_ss[currCl];
            if (result <= 1)
            {
                if (result > 0)
                    cl_stance_ss[currCl] = 0;
            }
            else
            {
                cl_stance_ss[currCl] = 1;
            }
        }
    }
    return kb[KB_WBUTTON6].active;
}

// ea: 0x52B0C0
int IN_LowerStance()
{
    if ((dword_106000 & cl[currCl].snap.ps.eFlags) == 0
        || cl[currCl].snap.ps.vehType == 2 && cl[currCl].snap.ps.vehPos == 1)
    {
        if (kb[KB_WBUTTON6].active == 0)
        {
            if (kb[KB_CROUCH].active == 0)
            {
                int result = cl_stance_ss[currCl];
                if (result >= 1)
                {
                    if (result < 2)
                        cl_stance_ss[currCl] = 2;
                }
                else
                {
                    cl_stance_ss[currCl] = 1;
                }
            }
        }
    }
    return 6320 * currCl;
}

// ea: 0x52B390
int IN_Stance_Down()
{
    int v0 = currCl;
    if (((dword_106000 & cl[currCl].snap.ps.eFlags) == 0
         || cl[currCl].snap.ps.vehType == 2 && cl[currCl].snap.ps.vehPos == 1)
        && kb[KB_WBUTTON6].active == 0
        && kb[KB_CROUCH].active == 0)
    {
        int v2 = cl_stance_ss[currCl];
        cl[currCl].stanceHeld = true;
        cl[currCl].stancePosition = v2;
        cl[currCl].stanceTime = com_frameTime;
        if (cl[currCl].stancePosition != 1)
            cl_stance_ss[v0] = 1;
    }
    return currCl * 6320;
}

// ea: 0x52B410
int IN_Stance_Up()
{
    if (kb[KB_WBUTTON6].active == 0 && kb[KB_CROUCH].active == 0)
    {
        if (cl[currCl].stanceHeld && cl[currCl].stancePosition == 1)
            cl_stance_ss[currCl] = 0;
        cl[currCl].joystickAxis[6] = 0;
    }
    return kb[KB_WBUTTON6].active;
}

// ea: 0x52B4B0
void IN_LeanLeftSwitchNextDown(int key, int time)
{
    if ((cl[currCl].snap.ps.pm_flags & 0x20) != 0)
        IN_KeyDown(&kb[KB_LEAN_LEFT], key, (unsigned int)time);
    else
        CG_PrevWeapon_f();
}

// ea: 0x52B4F0
void IN_LeanLeftSwitchNextUp(int key, int time)
{
    IN_KeyUp(&kb[KB_LEAN_LEFT], (unsigned int)key, time);
}

// ea: 0x52B450
void IN_LeanRightSwitchNextDown(int key, int time)
{
    if ((cl[currCl].snap.ps.pm_flags & 0x20) != 0)
        IN_KeyDown(&kb[KB_LEAN_RIGHT], key, (unsigned int)time);
    else
        CG_NextWeapon_f();
}

// ea: 0x52B490
void IN_LeanRightSwitchNextUp(int key, int time)
{
    IN_KeyUp(&kb[KB_LEAN_RIGHT], (unsigned int)key, time);
}

// ea: 0x52B540
void IN_SmokeGrenadeAttackDown(int key, int time)
{
    IN_KeyDown(&kb[KB_BUTTON1], key, (unsigned int)time);
    if ((0x10000 & cl[currCl].snap.ps.pm_flags) == 0)
    {
        int weapon = cl[currCl].snap.ps.weapon;
        if (weapon == 0
            || BG_GetInfoForWeapon(weapon)->weapClass != 8
            || (cl[currCl].snap.ps.pm_flags & 0x20) == 0
                && cl[currCl].snap.ps.fWeaponPosFrac <= 0.000099999997f)
        {
            CG_WeaponSlot_f(5);
        }
    }
}

// ea: 0x52B5C0
void IN_SmokeGrenadeAttackUp(int key, int time)
{
    IN_KeyUp(&kb[KB_BUTTON1], (unsigned int)key, time);
}

// ea: 0x52B5E0
void IN_ClassButtonDown(int key, int time)
{
    IN_KeyDown(&kb[KB_CLASS], key, (unsigned int)time);
    if ((0x10000 & cl[currCl].snap.ps.pm_flags) == 0)
    {
        if (cl[currCl].snap.ps.weapon == 0
            || (BG_GetInfoForWeapon(cl[currCl].snap.ps.weapon)->weapClass != 2)
            || (cl[currCl].snap.ps.pm_flags & 0x20) == 0
                && cl[currCl].snap.ps.fWeaponPosFrac <= 0.000099999997f)
        {
            int v4 = cl[currCl].snap.ps.weaponslots[9];
            if (v4 != 0)
            {
                weaponFileInfo_t* InfoForWeapon = BG_GetInfoForWeapon(v4);
                if (InfoForWeapon->type == 1 && InfoForWeapon->weapClass == 1)
                    CG_WeaponSlot_f(9);
            }
        }
    }
}

// ea: 0x52B700
void IN_ClassButtonUp(int key, int time)
{
    IN_KeyUp(&kb[KB_CLASS], (unsigned int)key, time);
}

// ea: 0x52B840
void IN_GrenadeAttackUp(int key, int time)
{
    IN_KeyUp(&kb[KB_BUTTON1], (unsigned int)key, time);
    gCookingLiveGrenade = false;
}

// ea: 0x52B720
void IN_GrenadeAttackDown(int key, int time)
{
    if (gGrenadeCanBePickedUp)
    {
        kbutton_t* b = &kb[KB_ACTIVATE_RELOAD];
        if (b->down[0] != 211 && b->down[1] != 211)
        {
            if (b->down[0] != 0)
            {
                if (b->down[1] != 0)
                {
                    Com_Printf("Three keys down for a button!\n");
                    return;
                }
                b->down[1] = 211;
            }
            else
            {
                b->down[0] = 211;
            }
            if (b->active == 0)
            {
                b->downtime = (unsigned int)time;
                b->active = 1;
                b->wasPressed = 1;
            }
        }
    }
    else
    {
        IN_KeyDown(&kb[KB_BUTTON1], key, (unsigned int)time);
        if ((0x10000 & cl[currCl].snap.ps.pm_flags) == 0)
        {
            int weapon = cl[currCl].snap.ps.weapon;
            if (weapon == 0)
            {
                CG_WeaponSlot_f(4);
            }
            else
            {
                int weapClass = BG_GetInfoForWeapon(weapon)->weapClass;
                if ((weapClass != 2
                     || (cl[currCl].snap.ps.pm_flags & 0x20) == 0
                         && cl[currCl].snap.ps.fWeaponPosFrac <= 0.000099999997f)
                    && (weapClass != 12
                        || (cl[currCl].snap.ps.pm_flags & 0x20) == 0
                            && cl[currCl].snap.ps.fWeaponPosFrac <= 0.000099999997f))
                {
                    CG_WeaponSlot_f(4);
                }
            }
        }
    }
}

// ea: 0x52B050
void IN_ButtonDown(int key, int time)
{
    IN_KeyDown(&kb[KB_BUTTON1], key, (unsigned int)time);
}

// ea: 0x52B070
void IN_ButtonUp(int key, int time)
{
    IN_KeyUp(&kb[KB_BUTTON1], (unsigned int)key, time);
}

// ea: 0x52B010
void IN_SprintDown(int key, int time)
{
    IN_KeyDown(&kb[KB_SPRINT], key, (unsigned int)time);
}

// ea: 0x52B030
void IN_SprintUp(int key, int time)
{
    IN_KeyUp(&kb[KB_SPRINT], (unsigned int)key, time);
}

// ea: 0x530560
void IN_SprintBreathDown(int key, int time)
{
    if (EntityManager_GetPlayer(EntityManager_sInst, currCl) != nullptr
        && (cl[currCl].snap.ps.pm_flags & 0x20) != 0
        && (BG_GetInfoForWeapon(cl[currCl].snap.ps.weapon)->weapClass == 11))
    {
        cl[currCl].snap.ps.mFlags_mask |= 1u;
    }
    else
    {
        IN_KeyDown(&kb[KB_SPRINT], key, (unsigned int)time);
    }
}

// ea: 0x530610
void IN_SprintBreathUp(int key, int time)
{
    if (EntityManager_GetPlayer(EntityManager_sInst, currCl) != nullptr
        && (cl[currCl].snap.ps.pm_flags & 0x20) != 0
        && (BG_GetInfoForWeapon(cl[currCl].snap.ps.weapon)->weapClass == 11))
    {
        cl[currCl].snap.ps.mFlags_mask &= ~1u;
    }
    else
    {
        IN_KeyUp(&kb[KB_SPRINT], (unsigned int)key, time);
    }
}

// ea: 0x5306C0
void IN_HoldBreathDown()
{
    if (EntityManager_GetPlayer(EntityManager_sInst, currCl) != nullptr)
        cl[currCl].snap.ps.mFlags_mask |= 1u;
}

// ea: 0x530720
void IN_HoldBreathUp()
{
    if (EntityManager_GetPlayer(EntityManager_sInst, currCl) != nullptr)
        cl[currCl].snap.ps.mFlags_mask &= ~1u;
}

// ea: 0x533970
void IN_BinocularsDown(int key, int time)
{
    if (EntityManager_GetPlayer(EntityManager_sInst, currCl) != nullptr
        && cl[currCl].snap.ps.weapon != 0)
    {
        weaponFileInfo_t* InfoForWeapon = BG_GetInfoForWeapon(cl[currCl].snap.ps.weapon);
        if (InfoForWeapon != nullptr)
        {
            if (InfoForWeapon->slot == 7)
            {
                CG_WeaponSlot_f(7);
                cl_binocButtonDown_integer = 0;
            }
            else if (CL_IsADS(currCl))
            {
                IN_HoldBreathDown();
            }
            else
            {
                IN_KeyDown(&kb[KB_BUTTON0], key, (unsigned int)time);
            }
        }
    }
}

// ea: 0x533A80
void IN_BinocularsUp(int key, int time)
{
    IN_HoldBreathUp();
    IN_KeyUp(&kb[KB_BUTTON0], (unsigned int)key, time);
}

// ea: 0x52B870
void IN_ActivateMoveUpDown(int key, int time)
{
    if (EntityManager_GetPlayer(EntityManager_sInst, currCl) != nullptr)
        IN_KeyDown(&kb[KB_ACTIVATE], key, (unsigned int)time);
    if (cl[currCl].snap.ps.serverCursorHint != 0 || (dword_106000 & cl[currCl].snap.ps.eFlags) != 0)
        IN_KeyDown(&kb[KB_ACTIVATE], key, (unsigned int)time);
    else
        IN_UpDown(key, time);
}

// ea: 0x52B920
void IN_ActivateMoveUpUp(int key, int time)
{
    IN_KeyUp(&kb[KB_UP], (unsigned int)key, time);
    IN_KeyUp(&kb[KB_STAND], (unsigned int)key, time);
    IN_KeyUp(&kb[KB_ACTIVATE], (unsigned int)key, time);
}

// ea: 0x52B960
void IN_ActivateMeleeDown(int key, int time)
{
    if (cl[currCl].snap.ps.serverCursorHint != 0 || (dword_106000 & cl[currCl].snap.ps.eFlags) != 0)
        IN_KeyDown(&kb[KB_ACTIVATE], key, (unsigned int)time);
    else
        IN_KeyDown(&kb[KB_BUTTON5], key, (unsigned int)time);
}

// ea: 0x52B9C0
void IN_ActivateMeleeUp(int key, int time)
{
    IN_KeyUp(&kb[KB_ACTIVATE], (unsigned int)key, time);
    IN_KeyUp(&kb[KB_BUTTON5], (unsigned int)key, time);
}

// ea: 0x52A820
void IN_SetTempStanceStatus()
{
}
