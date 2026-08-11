// ============================================================================
// cl_usercmd.cpp - client usercmd construction (cl.o cl_input.cpp / cl_main.cpp)
// 22 functions, verified against IDA (release map offsets + 0x40C000 = VA).
// ============================================================================

#include "cl_input.h"
#include "cl_console.h"

#include <string.h>

// ============================================================================
// Externs (core.o / cl.o)
// ============================================================================
extern void Com_Printf(const char* fmt, ...);
extern void Com_Error(int code, const char* fmt, ...);
extern int com_frameTime;
extern int frame_msec;
extern int currCl;
int dword_106000 = 0;  // cl.o BSS (EF_* flags mask)
extern int anykeydown;
extern bool CL_IsADS(int client);
extern int CL_StanceButtonUpdate();
extern char ClampChar(int i);
extern int dword_F12110;     // mlook active
int dword_F170F0 = 0;  // cl.o BSS @ 0xF170F0 (base turn speed)
float Deltas[4][5];  // ?Deltas@@3PAY04MA (cl.o BSS @ 0xF128C0)
int dword_F133C4[20];  // cl.o BSS (delta accumulation)
int dword_F133C8[20];  // cl.o BSS
int dword_F133CC[20];  // cl.o BSS
int dword_F133D0[20];  // cl.o BSS
extern struct cvar_t* cl_anglespeedkey;
extern struct cvar_t* cl_yawspeed;
extern struct cvar_t* cl_pitchspeed;
extern struct cvar_t* cl_stanceHoldTime;
extern usercmd_s CL_CreateCmd();

// Client entity view for view-angle setters
struct ClientEntityView {
    struct ClientPSView {
        float delta_angles[3];
        float viewangles[3];
    } ps;
};
struct EntityView2 {
    ClientEntityView* client;
};
extern EntityView2* EntityManager_GetPlayer2(void* inst, int idx);
extern void* EntityManager_sInst2;

namespace AeAssert {
enum ECoderId { COD3 = 0 };
extern ECoderId gCurrentAuthor;
extern const char* gCurrentFile;
extern int gCurrentLine;
extern const char* gCurrentExpr;
bool IsIgnored();
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

// ============================================================================
// Usercmd construction
// ============================================================================

// ea: 0x528460
int CL_GetUserCmd(int cmdNumber, usercmd_s* ucmd)
{
    if (cmdNumber > cl[currCl].cmdNumber)
    {
        Com_Error(1, "CL_GetUserCmd: old command %i > current %i",
                  cmdNumber, cl[currCl].cmdNumber);
    }
    if (cmdNumber <= cl[currCl].cmdNumber - 64 || cmdNumber <= 0)
        return 0;
    *ucmd = cl[currCl].cmds[cmdNumber & 0x3F];
    return 1;
}

// ea: 0x5284E0
int CL_GetCurrentCmdNumber()
{
    return cl[currCl].cmdNumber;
}

// ea: 0x52BC10
void CL_KeyMove(usercmd_s* cmd)
{
    unsigned int v3;
    if (kb[KB_WBUTTON6].active != 0)
    {
        cmd->buttons = cmd->buttons & 0xFFFF9FFF | 0x2000;
    }
    else
    {
        if (kb[KB_CROUCH].active == 0)
        {
            CL_StanceButtonUpdate();
            unsigned int v3;
            if (cl_stance_ss[currCl] == 1)
            {
                unsigned int v5 = cmd->buttons & 0xFFFF9FFF | 0x4000;
                cmd->buttons = (int)v5;
                v3 = v5 & 0xFFFFFEFF;
            }
            else if (cl_stance_ss[currCl] == 2)
            {
                unsigned int v4 = cmd->buttons & 0xFFFF9FFF | 0x2000;
                cmd->buttons = (int)v4;
                v3 = v4 & 0xFFFFFEFF;
            }
            else
            {
                unsigned int v2 = cmd->buttons & 0xFFFF9FFF;
                cmd->buttons = (int)v2;
                v3 = v2 & 0xFFFFFEFF;
            }
            goto LABEL_11;
        }
        cmd->buttons = cmd->buttons & 0xFFFF9FFF | 0x4000;
    }
    v3 = (unsigned int)cmd->buttons | 0x100;
LABEL_11:
    cmd->buttons = (int)v3;
    bool v6 = !CL_IsADS(currCl);
    int buttons = cmd->buttons;
    unsigned int v8;
    if (v6)
        v8 = (unsigned int)buttons & 0xFFFFFFF7;
    else
        v8 = (unsigned int)buttons | 8;
    cmd->buttons = (int)v8;
    int v9 = 0;
    int v10 = 0;
    if (kb[KB_STRAFE].active != 0)
    {
        int v11 = (int)(CL_KeyState(&kb[KB_LEFT + 1]) * 127.0f);
        v9 = (int)(CL_KeyState(&kb[KB_LEFT]) * -127.0f) + v11;
    }
    int v12 = (int)(CL_KeyState(&kb[KB_MOVERIGHT]) * 127.0f) + v9;
    int side = (int)(CL_KeyState(&kb[KB_MOVELEFT]) * -127.0f) + v12;
    int pm_type = cl[currCl].snap.ps.pm_type;
    if (pm_type == 2
        || pm_type == 3
        || (0x100000 & cl[currCl].snap.ps.eFlags) != 0
            && (0x400000 & cl[currCl].snap.ps.eFlags) == 0)
    {
        v10 = (int)(CL_KeyState(&kb[KB_UP]) * 127.0f);
    }
    int v16 = (int)(CL_KeyState(&kb[KB_STAND]) * 127.0f) + v10;
    int v17 = cmd->buttons;
    if ((v17 & 0x4000) != 0)
    {
        if (kb[KB_CROUCH].active != 0)
        {
            v16 += (int)(CL_KeyState(&kb[KB_CROUCH]) * -127.0f);
            goto LABEL_26;
        }
    }
    else if ((v17 & 0x2000) == 0)
    {
        goto LABEL_26;
    }
    v16 -= 127;
LABEL_26:
    int forward = (int)(CL_KeyState(&kb[KB_FORWARD]) * 127.0f);
    int v18 = (int)(CL_KeyState(&kb[KB_BACK]) * -127.0f) + forward;
    if ((cl[currCl].snap.ps.eFlags & 0x6000) == 0)
    {
        cmd->forwardmove = ClampChar(v18);
        cmd->rightmove = ClampChar(side);
        cmd->upmove = ClampChar(v16);
    }
}

// ea: 0x52C080
void CL_CmdButtons(usercmd_s* cmd)
{
    char v1 = 0;
    int* p_wasPressed = &kb[KB_BUTTON0].wasPressed;
    do
    {
        if (*(p_wasPressed - 1) != 0 || *p_wasPressed != 0)
            cmd->buttons |= 1 << v1;
        *p_wasPressed = 0;
        p_wasPressed += 6;
        ++v1;
    }
    while (p_wasPressed < &kb[KB_MLOOK].wasPressed);
    if (anykeydown != 0 && cls.keyCatchers == 0)
        cmd->buttons |= 0x8000;
    if (cl[currCl].cgameInShellshock != 0)
        cmd->buttons |= 0x200;
}

// ea: 0x52C100
void CL_SetUsercmdButtonsWeapons(int buttons, int weapon)
{
    if (com_cl_running->integer != 0)
    {
        cl[currCl].bCmdForceValues = 1;
        cl[currCl].iForceButtons = buttons;
        cl[currCl].iForceWeapon = weapon;
    }
}

// ea: 0x52C140
void CL_FinishMove(usercmd_s* cmd)
{
    cmd->weapon = cl[currCl].cgameUserCmdValue;
    cmd->serverTime = cl[currCl].serverTime;
    for (int v1 = 0; v1 < 3; ++v1)
        cmd->angles[v1] = (int)((cl[currCl].viewangles[v1] * 182.04445f) + 0.5f);
    cmd->gunPitch = cl[currCl].cgameGunPitch;
    cmd->gunYaw = cl[currCl].cgameGunYaw;
    cmd->gunXOfs = cl[currCl].cgameGunXOfs;
    cmd->gunYOfs = cl[currCl].cgameGunYOfs;
    cmd->gunZOfs = cl[currCl].cgameGunZOfs;
    if (cl[currCl].bCmdForceValues != 0)
    {
        cmd->buttons = cl[currCl].iForceButtons;
        cmd->weapon = cl[currCl].iForceWeapon;
        cl[currCl].bCmdForceValues = 0;
    }
}

// ea: 0x52C280
const usercmd_s& CL_GetCurUserCmd(int cmdNum)
{
    return cl[currCl].cmds[cmdNum & 0x3F];
}

// ea: 0x52C2C0
void CL_ShutdownInput()
{
    memset(kb, 0, sizeof(kb));
}

// ea: 0x52C540
void CL_ClearKeys()
{
    memset(kb, 0, sizeof(kb));
}

// ea: 0x528500
void CL_CapTurnRate(float maxPitchSpeed, float maxYawSpeed)
{
    cl[currCl].cgameMaxPitchSpeed = maxPitchSpeed;
    cl[currCl].cgameMaxYawSpeed = maxYawSpeed;
}

// ea: 0x528530
void CL_SetViewAngles(float* angles)
{
    ClientEntityView* client =
        EntityManager_GetPlayer2(EntityManager_sInst2, currCl)->client;
    float v2 = angles[0] - (client->ps.delta_angles[0] * 0.0054931641f);
    float v3 = angles[1] - (client->ps.delta_angles[1] * 0.0054931641f);
    float v4 = client->ps.delta_angles[2];
    cl[currCl].viewangles[0] = v2;
    cl[currCl].viewangles[1] = v3;
    cl[currCl].viewangles[2] = angles[2] - (v4 * 0.0054931641f);
}

// ea: 0x5285B0
void CL_SetViewAnglesAxis(int axis, float angle)
{
    ClientEntityView* client =
        EntityManager_GetPlayer2(EntityManager_sInst2, currCl)->client;
    cl[currCl].viewangles[axis] = angle - (client->ps.delta_angles[axis] * 0.0054931641f);
    client->ps.viewangles[axis] = angle;
    cl[currCl].cmds[cl[currCl].cmdNumber & 0x3F] = CL_CreateCmd();
}

// ea: 0x5287F0
void CL_SetUserCmdValue(int userCmdValue, int holdableValue, float sensitivityScale)
{
    cl[currCl].cgameUserCmdValue = userCmdValue;
    cl[currCl].cgameUserHoldableValue = holdableValue;
    cl[currCl].cgameSensitivity = sensitivityScale;
}

// ea: 0x528820
void CL_SetUserCmdAimValues(float gunPitch, float gunYaw, float gunXOfs,
                            float gunYOfs, float gunZOfs)
{
    cl[currCl].cgameGunPitch = gunPitch;
    cl[currCl].cgameGunYaw = gunYaw;
    cl[currCl].cgameGunXOfs = gunXOfs;
    cl[currCl].cgameGunYOfs = gunYOfs;
    cl[currCl].cgameGunZOfs = gunZOfs;
}

// ea: 0x528880
void CL_SetUserCmdInShellshock(int shocked)
{
    cl[currCl].cgameInShellshock = shocked;
}

// ea: 0x52BA10
void CL_AdjustAngles()
{
    float v0 = (float)dword_F170F0;
    if (kb[KB_SPEED].active != 0)
        v0 = v0 * cl_anglespeedkey->value;
    float speed = v0 * 0.001f;
    if (kb[KB_STRAFE].active == 0)
    {
        float* v3 = &cl[currCl].viewangles[1];
        *v3 = *v3 - CL_KeyState(&kb[KB_LEFT + 1]) * cl_yawspeed->value * speed;
        *v3 = CL_KeyState(&kb[KB_LEFT]) * cl_yawspeed->value * speed + *v3;
    }
    float* viewangles = cl[currCl].viewangles;
    *viewangles = *viewangles - CL_KeyState(&kb[KB_LOOKUP]) * cl_pitchspeed->value * speed;
    *viewangles = CL_KeyState(&kb[KB_LOOKDOWN]) * cl_pitchspeed->value * speed + *viewangles;
}

// ea: 0x52BB50
int CL_StanceButtonUpdate()
{
    if (kb[KB_WBUTTON6].active != 0 || kb[KB_CROUCH].active != 0)
    {
        ASSERT("!((kb[KB_PRONE].active) || (kb[KB_DOWN].active))",
               "c:\\cod\\code\\game\\cl_input.cpp", 1094);
    }
    int result = 6320 * currCl;
    if (cl[currCl].stanceHeld
        && (0x100000 & cl[currCl].snap.ps.eFlags) == 0
        && com_frameTime - cl[currCl].stanceTime >= cl_stanceHoldTime->integer)
    {
        int stancePosition = cl[currCl].stancePosition;
        cl[currCl].stanceHeld = false;
        cl_stance_ss[currCl] = stancePosition == 2 ? 0 : 2;
    }
    return result;
}

// ea: 0x52BE60
void CL_MouseEvent(int dx, int dy)
{
    int mouseIndex = cl[currCl].mouseIndex;
    cl[currCl].mouseDx[mouseIndex] += dx;
    cl[currCl].mouseDy[mouseIndex] += dy;
}

// ea: 0x52BEB0
void CL_JoystickEvent(unsigned int axis, int value)
{
    if (axis >= 6)
        Com_Error(1, "CL_JoystickEvent: bad axis %i", axis);
    cl[currCl].joystickAxis[axis] = value;
}

// ea: 0x52BEF0
void CL_JoystickMove(usercmd_s* cmd)
{
    if (CL_IsADS(currCl))
        cmd->buttons |= 8;
    float v1 = (float)dword_F170F0;
    if (kb[KB_SPEED].active != 0)
        v1 = v1 * cl_anglespeedkey->value;
    cmd->rightmove = ClampChar(cl[currCl].joystickAxis[0] + cmd->rightmove);
    if (dword_F12110 != 0)
    {
        cl[currCl].viewangles[0] =
            (cl[currCl].joystickAxis[1] * cl_pitchspeed->value) * (v1 * 0.001f)
            + cl[currCl].viewangles[0];
    }
    else
    {
        cmd->forwardmove = ClampChar(cl[currCl].joystickAxis[1] + cmd->forwardmove);
    }
    cmd->upmove = ClampChar(cl[currCl].joystickAxis[2] + cmd->upmove);
}

// ea: 0x52BFF0
void GetAverageDelta(float* deltaAngle, int* index, int iStickIndex)
{
    if (*index >= 5)
        *index = 0;
    Deltas[iStickIndex][*index] = *deltaAngle;
    *deltaAngle = 0.0f;
    *deltaAngle = Deltas[iStickIndex][0];
    *deltaAngle = dword_F133C4[5 * iStickIndex] + *deltaAngle;
    *deltaAngle = dword_F133C8[5 * iStickIndex] + *deltaAngle;
    *deltaAngle = dword_F133CC[5 * iStickIndex] + *deltaAngle;
    *deltaAngle = (dword_F133D0[5 * iStickIndex] + *deltaAngle) * 0.2f;
}

// ea: 0x52C560
int CL_ClearKeysForAll()
{
    memset(kb, 0, sizeof(kb));
    memset(kbss[currCl], 0, sizeof(kbss[currCl]));
    return 0;
}
