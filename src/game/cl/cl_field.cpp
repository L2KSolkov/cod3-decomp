// ============================================================================
// cl_field.cpp - text-field editing + key bindings + swirl (cl.o)
// 18 functions, verified against IDA (release map offsets + 0x40C000 = VA).
// ============================================================================

#include "cl_input.h"
#include "cl_console.h"
#include "game/game_types.h"

#include "core/math_types.h"

#include <ctype.h>
#include <math.h>
#include <string.h>

// Minimal view of GamePause (full class in game/sv/sv_stubs.h).
struct GamePause { static bool IsGamePaused(int client); };


// ============================================================================
// Externs
// ============================================================================
extern void Com_Printf(const char* fmt, ...);
extern int Cmd_Argc();
extern char* Cmd_Argv(int arg);
extern int Q_stricmp(const char* s1, const char* s2);
extern void Q_strncpyz(char* dest, const char* src, int destsize);
extern "C" void _Z_FreeInternal(void* ptr);
extern char* CopyStringInternal(const char* in);
extern int cvar_modifiedFlags;
extern int dword_F1719C;
extern int dword_F171A0;
extern void Field_Clear(field_t* edit);
extern void Field_AdjustScroll(field_t* edit);
extern void Field_Draw(field_t* edit, int x, int y);
extern int Key_StringToKeynum(char* str);
extern char* Key_KeynumToString(int keynum, int bTranslate);
extern void FS_Printf(int h, const char* fmt, ...);

// ============================================================================
// KeyInfo (key binding table)
// ============================================================================
struct KeyInfoEntry2 {
    int   mState;          // +0x00 (low 2 bits = down, high 30 = repeats)
    char* mBoundCmdName;
};
template <typename T, int N>
struct ae_array_fixed {
    T m_elements[N];
};
struct KeyInfo {
    static ae_array_fixed<ae_array_fixed<KeyInfoEntry2, 256>, 1> mKeys;
    static void SetBinding(int keyIndex, int clnt, const char* boundCmdName);
    static void SetDown(int keyIndex, int clnt, int down);  // ?SetDown@KeyInfo@@SAXHHH@Z (cl.o 0x5398F0)
    static void IncRepeats(int keyIndex, int clnt);         // ?IncRepeats@KeyInfo@@SAXHH@Z (cl.o 0x539930)
    static void ClearRepeats(int keyIndex, int clnt);       // ?ClearRepeats@KeyInfo@@SAXHH@Z (cl.o 0x539970)
    static int  IsDown(int keyIndex, int clnt);             // ?IsDown@KeyInfo@@SAHHH@Z (cl.o 0x5399A0)
    static int  GetRepeats(int keyIndex, int clnt);         // ?GetRepeats@KeyInfo@@SAHHH@Z (cl.o 0x5399E0)
    static int GetKey(const char* boundCmdName, int clnt);
    static void ClearAllBindings();
};
ae_array_fixed<ae_array_fixed<KeyInfoEntry2, 256>, 1> KeyInfo::mKeys;

// ea: 0x5398F0
void KeyInfo::SetDown(int keyIndex, int clnt, int down)
{
    KeyInfoEntry2* e = &mKeys.m_elements[clnt].m_elements[keyIndex];
    e->mState ^= (down ^ e->mState) & 3;
}

// ea: 0x539930
void KeyInfo::IncRepeats(int keyIndex, int clnt)
{
    KeyInfoEntry2* e = &mKeys.m_elements[clnt].m_elements[keyIndex];
    e->mState = (e->mState & 3) ^ ((e->mState & 0xFFFFFFFC) + 4);
}

// ea: 0x539970
void KeyInfo::ClearRepeats(int keyIndex, int clnt)
{
    KeyInfoEntry2* e = &mKeys.m_elements[clnt].m_elements[keyIndex];
    e->mState &= 3;
}

// ea: 0x5399A0
int KeyInfo::IsDown(int keyIndex, int clnt)
{
    KeyInfoEntry2* e = &mKeys.m_elements[clnt].m_elements[keyIndex];
    return (e->mState & 3) != 0;
}

// ea: 0x5399E0
int KeyInfo::GetRepeats(int keyIndex, int clnt)
{
    KeyInfoEntry2* e = &mKeys.m_elements[clnt].m_elements[keyIndex];
    return e->mState >> 2;
}

extern void ButtonMgr_UpdateBinding(int keyInfoIndex, int clnt);

// ea: 0x531DF0
void KeyInfo::SetBinding(int keyIndex, int clnt, const char* boundCmdName)
{
    KeyInfoEntry2* v4 = &mKeys.m_elements[clnt].m_elements[keyIndex];
    if (v4->mBoundCmdName != nullptr)
    {
        _Z_FreeInternal(v4->mBoundCmdName);
        v4->mBoundCmdName = nullptr;
    }
    if (boundCmdName != nullptr)
    {
        v4->mBoundCmdName = CopyStringInternal(boundCmdName);
        cvar_modifiedFlags |= 1;
    }
    ButtonMgr_UpdateBinding(keyIndex, clnt);
}

// ea: 0x531E60
int KeyInfo::GetKey(const char* boundCmdName, int clnt)
{
    if (boundCmdName == nullptr)
        return -1;
    int v2 = 0;
    while (1)
    {
        KeyInfoEntry2* e = &mKeys.m_elements[clnt].m_elements[v2];
        if (e->mBoundCmdName != nullptr
            && Q_stricmp(boundCmdName, e->mBoundCmdName) == 0)
        {
            break;
        }
        if (++v2 >= 256)
            return -1;
    }
    return v2;
}

// ea: 0x531F20
void KeyInfo::ClearAllBindings()
{
    for (int i = 0; i < 256; ++i)
    {
        KeyInfoEntry2* e = &mKeys.m_elements[0].m_elements[i];
        if (e->mBoundCmdName != nullptr)
        {
            _Z_FreeInternal(e->mBoundCmdName);
            e->mBoundCmdName = nullptr;
        }
        ButtonMgr_UpdateBinding(i, 0);
    }
}

// ============================================================================
// Key bind commands
// ============================================================================

// ea: 0x531FB0
void Key_Unbind_f()
{
    if (Cmd_Argc() == 2)
    {
        int v1 = Key_StringToKeynum((char*)Cmd_Argv(1));
        if (v1 == -1)
        {
            Com_Printf("\"%s\" isn't a valid key\n", Cmd_Argv(1));
        }
        else
        {
            KeyInfoEntry2* v5 = &KeyInfo::mKeys.m_elements[currCl].m_elements[v1];
            if (v5->mBoundCmdName != nullptr)
            {
                _Z_FreeInternal(v5->mBoundCmdName);
                v5->mBoundCmdName = nullptr;
            }
            ButtonMgr_UpdateBinding(v1, currCl);
        }
    }
    else
    {
        Com_Printf("usage: unbind <key>\n");
    }
}

// ea: 0x532040
void Key_Unbindall_f()
{
    KeyInfo::ClearAllBindings();
}

// ea: 0x532150
void Key_Bind_f()
{
    int c = Cmd_Argc();
    if (c >= 2)
    {
        int v2 = Key_StringToKeynum((char*)Cmd_Argv(1));
        if (v2 == -1)
        {
            Com_Printf("\"%s\" isn't a valid key\n", Cmd_Argv(1));
        }
        else if (c == 2)
        {
            KeyInfoEntry2* e = &KeyInfo::mKeys.m_elements[currCl].m_elements[v2];
            if (e->mBoundCmdName != nullptr)
                Com_Printf("\"%s\" = \"%s\"\n", Key_KeynumToString(v2, 1),
                           e->mBoundCmdName);
            else
                Com_Printf("\"%s\" is not bound\n", Key_KeynumToString(v2, 1));
        }
        else
        {
            // Cmd_Args(2) joined with spaces
            extern char* Cmd_Args(int start);
            KeyInfo::SetBinding(v2, currCl, Cmd_Args(2));
        }
    }
    else
    {
        Com_Printf("usage: bind <key> [command]\n");
    }
}

// ea: 0x532320
void Key_WriteBindings(int f)
{
    FS_Printf(f, "unbindall\n");
    for (int i = 0; i < 256; ++i)
    {
        KeyInfoEntry2* e = &KeyInfo::mKeys.m_elements[currCl].m_elements[i];
        if (e->mBoundCmdName != nullptr)
        {
            FS_Printf(f, "bind \"%s\" \"%s\"\n", Key_KeynumToString(i, 1),
                      e->mBoundCmdName);
        }
    }
}

// ea: 0x532520
void Key_Bindlist_f()
{
    for (int i = 0; i < 256; ++i)
    {
        KeyInfoEntry2* e = &KeyInfo::mKeys.m_elements[currCl].m_elements[i];
        if (e->mBoundCmdName != nullptr)
        {
            Com_Printf("%s \"%s\"\n", Key_KeynumToString(i, 1),
                       e->mBoundCmdName);
        }
    }
}

// ea: 0x532EC0
void Key_GetBindingBuf(int keynum, char* buf, int buflen)
{
    char* mBoundCmdName =
        KeyInfo::mKeys.m_elements[currCl].m_elements[keynum].mBoundCmdName;
    if (mBoundCmdName != nullptr)
        Q_strncpyz(buf, mBoundCmdName, buflen);
    else
        *buf = 0;
}

// ============================================================================
// Field editing
// ============================================================================
extern int key_overstrikeMode;
// Pointer table matching core/common.cpp's `re` layout (cl.o re_export).
struct refexport_t {
    void (*Shutdown)(int);
    void (*BeginRegistration)(void*);
    void* (*RegisterModel)(void* result, const char*, int, int);
    int (*RegisterShader)(const char*, int);
    int (*RegisterShaderNoMip)(const char*, int);
    void (*LoadWorld)(const char*, int*);
    void (*SetFXImageMemory)(int);
    int (*GetFXImageMemory)();
    int (*GetImageMemory)();
    float (*GetFarPlaneDist)();
    void (*EndRegistration)();
    void (*ClearScene)();
    void (*AddPolyToScene)(void*, int, const void*);
    void (*AddLightToScene)(const float*, float, float, float, float);
    void (*SetCullDist)(float);
    void (*SetFog)(int, int, int, float, float, float, float);
    void (*RenderScene)(const void*);
    void (*ClearFlares)();
    void (*SetColor)(const float*);
    void (*DrawStretchPic)(float, float, float, float, float, float, float,
                           float, void*);
    void (*DrawStretchPicGradient)(float, float, float, float, float, float,
                                   float, float, void*, const float*, int);
    void (*DrawStretchPicRotate)(float, float, float, float, float, float,
                                 float, float, float, void*);
    void (*DrawQuadPic)(const float (*)[2], const float (*)[2], void*);
    void (*DrawStretchRaw)(int, int, int, int, int, int,
                           const unsigned char*, int, int);
    void (*UploadCinematic)(int, int, int, int, const unsigned char*, int, int);
    void (*BeginFrame)();
    void (*EndFrame)(int*, int*);
    void (*SaveScreen)();
    void (*TrackStatistics)(void*);
    int (*PickShader)(const float*, const float*, char*, char*, char*, int);
    void (*ResetImageAllocations)();
    void (*FreeImageAllocations)();
    void (*CubemapShot)(const char*, int, int, float, float);
    void (*CubemapWaterShot)(const char*, int, int, float*, float*);
    void (*LocateDebugStrings)(void*, int);
    void (*LocateDebugLines)(void*, int);
    int (*Text_Width)(const char*, int, float, float, int);
    int (*Text_Height)(int, float);
    void (*Text_Paint)(float, float, int, float, const float*, const char*,
                       float, int, int);
    int (*Text_ConsoleWidth)(const short*, int, float, float, int);
    void (*Text_ConsolePaint)(float, float, int, float, const float*,
                              const short*, float, int, int);
    void (*Text_PaintWithCursor)(float, float, int, float, const float*,
                                 const char*, int, char, float, int, int);
};
extern refexport_t re;
extern int SEH_PrintStrlen(const char* string);  // shell.o
// ea: 0x5315B0
void Field_Draw(field_t* edit, int x, int y, int showCursor)
{
    (void)showCursor;
    int scroll = edit->scroll;
    float vColor[4];
    vColor[0] = 1.0f;
    vColor[1] = 1.0f;
    vColor[2] = 1.0f;
    vColor[3] = 1.0f;
    char str[256];
    Q_strncpyz(str, &edit->buffer[scroll], 256 - scroll);
    int bFixedSize = edit->bFixedSize;
    float v6 = (float)y + edit->charHeight;
    float v7 = edit->charHeight * 0.041666668f;
    float charWidth = edit->charWidth;
    int v9 = edit->cursor - edit->scroll;
    float fX = (float)x;
    float fY = v6;
    float fFontScale = v7;
    float fCharWidth = charWidth;
    int v12;
    int v13;
    int v10;
    if (bFixedSize != 0)
    {
        fX = (640.0f / (float)dword_F1719C) * x;
        v10 = 0;
        float v11 = 480.0f / (float)dword_F171A0;
        fY = v11 * v6;
        fFontScale = v11 * v7;
        fCharWidth = (640.0f / (float)dword_F1719C) * charWidth;
        v12 = 5;
        v13 = (key_overstrikeMode != 0) + 10;
    }
    else
    {
        v12 = 0;
        v10 = 3;
        v13 = key_overstrikeMode == 0 ? 124 : 95;
    }
    if (edit->drawWidth == 0)
        edit->drawWidth = 256;
    re.Text_PaintWithCursor(fX, fY, v12, fFontScale, vColor, str, v9, v13,
                            fCharWidth, edit->drawWidth, v10);
}

// ea: 0x531700
void Field_AdjustScroll(field_t* edit)
{
    int bFixedSize = edit->bFixedSize;
    float v3 = edit->charHeight * 0.041666668f;
    float charWidth = edit->charWidth;
    float widthInPixels = (float)edit->widthInPixels;
    float fFontScale = v3;
    float fCharWidth = charWidth;
    float fLineWidth = widthInPixels;
    int v8;
    if (bFixedSize != 0)
    {
        float v6 = (480.0f / (float)dword_F171A0) * v3;
        float v7 = 640.0f / (float)dword_F1719C;
        fFontScale = v6;
        fCharWidth = v7 * charWidth;
        fLineWidth = v7 * widthInPixels;
        v8 = 5;
    }
    else
    {
        v8 = 0;
    }
    if (fLineWidth > (float)re.Text_Width(edit->buffer, v8, fFontScale,
                                          fCharWidth, 0))
    {
        edit->scroll = 0;
        edit->drawWidth = SEH_PrintStrlen(edit->buffer);
        return;
    }
    if (fLineWidth > 0.0f)
    {
        while (1)
        {
            if (edit->scroll <= 0
                || (float)re.Text_Width(&edit->buffer[edit->scroll], v8,
                                        fFontScale, fCharWidth, 0)
                       >= fLineWidth)
            {
                break;
            }
            --edit->scroll;
        }
    }
    float v9 = fCharWidth;
    while (1)
    {
        int v11 = re.Text_Width(&edit->buffer[edit->scroll], v8, fFontScale,
                                v9, 0);
        int v12 = v11 - re.Text_Width(&edit->buffer[edit->cursor], v8,
                                      fFontScale, fCharWidth, 0);
        if (v12 < 0)
        {
            if (edit->scroll == 0)
            {
                v12 = 0;
                goto LABEL_17;
            }
            edit->scroll -= 1;
        }
        else
        {
            if (v12 < (int)fLineWidth)
                goto LABEL_16;
            edit->scroll += 1;
        }
LABEL_16:
        if (v12 >= 0)
        {
LABEL_17:
            if (v12 < (int)fLineWidth)
                break;
        }
        v9 = fCharWidth;
    }
    int v14 = edit->scroll;
    int v15 = (int)strlen(&edit->buffer[v14]);
    edit->drawWidth = edit->cursor - v14;
    if (fLineWidth > 0.0f)
    {
        while (1)
        {
            int drawWidth = edit->drawWidth;
            if (drawWidth >= v15
                || fLineWidth <= (float)re.Text_Width(
                                     &edit->buffer[edit->scroll], v8,
                                     fFontScale, fCharWidth, drawWidth + 1))
            {
                break;
            }
            ++edit->drawWidth;
        }
    }
}

// ea: 0x531900
void Field_KeyDownEvent(field_t* edit, int key)
{
    int v2 = (int)strlen(edit->buffer);
    switch (key)
    {
    case 162:  // delete
        if (edit->cursor < v2)
        {
            memmove(&edit->buffer[edit->cursor],
                    &edit->buffer[edit->cursor + 1],
                    v2 - edit->cursor);
            Field_AdjustScroll(edit);
        }
        break;
    case 157:  // right arrow
        if (edit->cursor < v2)
            ++edit->cursor;
        break;
    case 156:  // left arrow
        if (edit->cursor > 0)
            --edit->cursor;
        break;
    case 161:  // home
        edit->cursor = 0;
        break;
    case 160:  // end
        edit->cursor = v2;
        break;
    default:
        break;
    }
    Field_AdjustScroll(edit);
}

// ea: 0x531B40
void Field_CharEvent(field_t* edit, int ch)
{
    int v2 = (int)strlen(edit->buffer);
    switch (ch)
    {
    case 3:  // ctrl-c: clear
        Field_Clear(edit);
        Field_AdjustScroll(edit);
        return;
    case 8:  // backspace
        if (edit->cursor > 0)
        {
            memmove(&edit->buffer[edit->cursor - 1],
                    &edit->buffer[edit->cursor],
                    v2 - edit->cursor + 1);
            --edit->cursor;
        }
        Field_AdjustScroll(edit);
        return;
    case 1:  // ctrl-a
        edit->cursor = 0;
        edit->scroll = 0;
        return;
    case 22:  // ctrl-v: paste
        Field_AdjustScroll(edit);
        return;
    default:
        if (ch >= 32 && ch < 127)
        {
            if (edit->cursor < 255)
            {
                memmove(&edit->buffer[edit->cursor + 1],
                        &edit->buffer[edit->cursor],
                        v2 - edit->cursor + 1);
                edit->buffer[edit->cursor++] = (char)ch;
            }
        }
        Field_AdjustScroll(edit);
        return;
    }
}

// ============================================================================
// Swirl
// ============================================================================
int gaGlobs_axes[12];  // cl.o BSS
float prevDir[2][3];  // ?prevDir@@3PAY02MA (cl.o BSS)
float dword_F11E4C[6];  // cl.o BSS; IDA uses these as float history vectors
float dword_F11E50[6];  // cl.o BSS; IDA uses these as float history vectors
float gSwirlPitchFactor;
extern unsigned int frame_msec;
int totalTime[2];  // cl.o BSS
float totalAngle[2];
float sTotalTimeMax;
extern double VectorNormalize(float* const v);
extern void CrossProduct(const float* v1, const float* v2, float* cross);

// ea: 0x530780
void GetSwirlSpeedDirect(float* fCosDeltaAngle, int* iRotationDir,
                         int iStickIndex)
{
    float currDir[3];
    float v7 = 0.0f - (float)gaGlobs_axes[2 * (iStickIndex == 0) + 1];
    currDir[0] = (float)gaGlobs_axes[iStickIndex != 0 ? 0 : 2];
    currDir[1] = v7;
    currDir[2] = 0.0f;
    VectorNormalize(currDir);
    float* v3 = prevDir[iStickIndex];
    if (*v3 != 0.0f || dword_F11E4C[3 * iStickIndex] != 0.0f)
    {
        *fCosDeltaAngle = (dword_F11E50[3 * iStickIndex] * 0.0f)
                          + (*v3 * currDir[0])
                          + (dword_F11E4C[3 * iStickIndex] * v7);
        float crossPro[2];
        CrossProduct(prevDir[iStickIndex], currDir, crossPro);
        if (crossPro[0] <= 0.0f)
        {
            if (crossPro[0] < 0.0f)
                *iRotationDir = -1;
        }
        else
        {
            *iRotationDir = 1;
        }
    }
    v3[0] = currDir[0];
    dword_F11E4C[3 * iStickIndex] = v7;
    dword_F11E50[3 * iStickIndex] = 0;
}

// ea: 0x5308B0
void SwirlControl(int stickIndex)
{
    float fCosDelta = -2.0f;
    int rotateDir = 0;
    GetSwirlSpeedDirect(&fCosDelta, &rotateDir, stickIndex);
    float v2 = (1.0f - fCosDelta) * rotateDir;
    if (stickIndex == 1)
    {
        float v3 = gSwirlPitchFactor;
        if (gSwirlPitchFactor <= 0.001f)
            v3 = 1.5f;
        v2 = v3 * v2;
    }
    if (totalTime[stickIndex] <= (int)sTotalTimeMax)
    {
        totalTime[stickIndex] += frame_msec;
        totalAngle[stickIndex] += v2;
    }
}

// ea: 0x530AC0
void CLSwirlControl()
{
    SwirlControl(0);
    SwirlControl(1);
}

// ============================================================================
// CL_GamepadMove - gamepad -> usercmd + view angle
// ============================================================================
extern float CL_GamepadAxisValue(unsigned int virtualAxis);
extern float CL_GamepadPhysicalAxisValue(int physicalAxis);
extern unsigned int frame_msec;
extern int dword_F6A28C[4 * 802];
int gSaveGameData_mInvertAim[4];            // ?gSaveGameData_mInvertAim (game2.o)
int gSaveGameData_mHorizontalSensitivity[4]; // ?gSaveGameData_mHorizontalSensitivity (game2.o)
int gSaveGameData_mVerticalSensitivity[4];   // ?gSaveGameData_mVerticalSensitivity (game2.o)
float accelRate;
float accel_time;
float accel_time_0;
float accel_scale;
float gMaxTurnSpeed;
extern struct cvar_t* cl_mouseAccel;
extern struct cvar_t* cl_showMouseRate;
extern struct cvar_t* m_yaw;
extern struct cvar_t* m_pitch;
extern bool IsPlayerFullySeatedInVehicle(Entity* player);
class EntityManager {
public:
    static EntityManager* sInst;
    Entity* GetPlayer(int idx);
};
extern const signed char ClampChar(int i);

// ea: 0x530AE0
void CL_GamepadMove(usercmd_s* cmd)
{
    if ((cl[currCl].snap.ps.pm_flags & 0x4000) != 0)
        return;
    if (cl[currCl].snap.ps.vehType == 5)
    {
        SwirlControl(0);
        SwirlControl(1);
        return;
    }
    float pitch = CL_GamepadAxisValue(4) * -128.0f;
    int v1 = currCl != 0 ? 0 : dword_F6A28C[0];
    pitch = (float)(2 * gSaveGameData_mInvertAim[v1] - 1) * pitch;
    float yaw = CL_GamepadAxisValue(3) * 128.0f;
    float accelSensitivityY = -CL_GamepadAxisValue(1);
    float accelSensitivityX = CL_GamepadAxisValue(0);
    CL_GamepadAxisValue(5);
    float v2 = fabsf(accelSensitivityX);
    float moveScale = 127.0f;
    if (v2 > 0.0f || fabsf(accelSensitivityY) > 0.0f)
    {
        float v3;
        if (v2 <= fabsf(accelSensitivityY))
            v3 = accelSensitivityX / accelSensitivityY;
        else
            v3 = accelSensitivityY / accelSensitivityX;
        moveScale = sqrtf(v3 * v3 + 1.0f) * 127.0f;
    }
    cmd->rightmove = ClampChar((int)(cmd->rightmove + (moveScale * accelSensitivityX)));
    int v46 = (int)(cmd->forwardmove + (moveScale * accelSensitivityY));
    cmd->forwardmove = ClampChar(v46);
    if (CL_IsADS(currCl))
        cmd->buttons |= 8;
    float v5 = pitch * 0.5f;
    pitch = pitch * 0.5f;
    int v6 = currCl;
    accelSensitivityY = (float)frame_msec * 0.001f;
    if (fabsf(yaw) > 110.0f
        && cl[currCl].snap.ps.prevTargetPointValid == 0
        && cl[currCl].snap.ps.fWeaponPosFrac != 1.0f
        && cl[currCl].snap.ps.vehType != 2
        && cl[currCl].snap.ps.vehPos != 1)
    {
        float v7 = yaw;
        cl[currCl].cgameCurrentAimAccel += accelRate * accelSensitivityY;
        int v8;
        if (v7 <= 0.0f)
            v8 = v7 >= 0.0f ? 0 : -1;
        else
            v8 = 1;
        yaw = (float)v8 * cl[currCl].cgameCurrentAimAccel + v7;
        goto LABEL_40;
    }
    int v9 = currCl;
    float v11;
    float v12;
    if (cl[currCl].snap.ps.vehPos == 1)
    {
        float v10 = (yaw == 0.0f && v5 == 0.0f) ? -accelSensitivityY : accelSensitivityY;
        cl[currCl].cgameCurrentAimAccel += v10;
        float zero = 0.0f;
        v11 = ClampRange(cl[v9].cgameCurrentAimAccel, zero, accel_time_0);
        moveScale = v11;
        v12 = (moveScale / accel_time_0) * accel_scale;
    }
    else
    {
        if (cl[currCl].snap.ps.fWeaponPosFrac != 1.0f)
        {
            cl[currCl].cgameCurrentAimAccel = 0.0f;
            goto LABEL_40;
        }
        float v14 = (yaw == 0.0f && v5 == 0.0f) ? -accelSensitivityY : accelSensitivityY;
        cl[currCl].cgameCurrentAimAccel += v14;
        float zero = 0.0f;
        v11 = ClampRange(cl[v9].cgameCurrentAimAccel, zero, accel_time);
        moveScale = v11;
        v12 = moveScale / accel_time;
    }
    v6 = currCl;
    float v13 = v12 * yaw;
    cl[currCl].cgameCurrentAimAccel = v11;
    yaw = v13;
    pitch = v12 * pitch;
LABEL_40:
    float v17;
    float v18;
    if ((EntityManager::sInst->GetPlayer(v6) != nullptr
         && (IsPlayerFullySeatedInVehicle(EntityManager::sInst->GetPlayer(currCl))
             || EntityManager::sInst->GetPlayer(currCl)->IsCameraTweening()))
        || GamePause::IsGamePaused(currCl))
    {
        v17 = 0.0f;
        v18 = 0.0f;
    }
    else
    {
        v17 = yaw;
        v18 = pitch;
    }
    int v19 = currCl;
    float v20 = accelSensitivityY * v17;
    yaw = accelSensitivityY * v17;
    pitch = accelSensitivityY * v18;
    int v21 = currCl != 0 ? 0 : dword_F6A28C[0];
    float v22 = (float)gSaveGameData_mHorizontalSensitivity[v21] * 0.039999999f;
    int v23 = currCl != 0 ? 0 : dword_F6A28C[0];
    float v24 = (float)gSaveGameData_mVerticalSensitivity[v23] * 0.039999999f;
    if (v22 == 0.0f)
        v22 = 0.1f;
    if (v24 == 0.0f)
        v24 = 0.1f;
    float v25 = (float)frame_msec;
    float value = cl_mouseAccel->value;
    moveScale = sqrtf(yaw * yaw + pitch * pitch) / (float)frame_msec;
    float v27 = value * moveScale;
    float v29 = (v27 + v22) * cl[currCl].cgameSensitivity;
    float v30 = (v27 + v24) * cl[currCl].cgameSensitivity;
    accelSensitivityX = v29;
    accelSensitivityY = v30;
    if (moveScale != 0.0f && cl_showMouseRate->integer != 0)
    {
        Com_Printf("%f : %f\n", moveScale, accelSensitivityX);
        Com_Printf("%f : %f\n", moveScale, accelSensitivityY);
        v19 = currCl;
        v25 = (float)frame_msec;
        v29 = accelSensitivityX;
        v30 = accelSensitivityY;
        v20 = yaw;
    }
    int v31 = v19;
    if ((cl[v31].snap.ps.pm_flags & 0x4000) == 0
        || (dword_106000 & cl[v31].snap.ps.eFlags) != 0)
    {
        float v32 = v30 * pitch;
        float v33 = v29 * v20;
        float v34 = v32;
        if (v33 != 0.0f || v32 != 0.0f)
        {
            float v35 = m_yaw->value * v33;
            float v38;
            if (cl[v31].cgameMaxYawSpeed == 0.0f)
                v38 = v25 * gMaxTurnSpeed;
            else
                v38 = cl[v31].cgameMaxYawSpeed * v25;
            float v39 = v38 * 0.001f;
            float v40 = v39;
            if (v35 <= v39)
                v40 = v35;
            if (-v39 <= v40)
            {
                if (v35 > v39)
                    v35 = v39;
            }
            else
            {
                v35 = -v39;
            }
            cl[v31].viewangles[1] -= v35;
            float cgameMaxPitchSpeed = cl[v31].cgameMaxPitchSpeed;
            float v43 = m_pitch->value * v34;
            if (cgameMaxPitchSpeed != 0.0f)
            {
                float v44 = (v25 * cgameMaxPitchSpeed) * 0.001f;
                float v45 = v44;
                if (v43 <= v44)
                    v45 = v43;
                if (-v44 <= v45)
                {
                    if (v43 > v44)
                        v43 = v44;
                }
                else
                {
                    v43 = -v44;
                }
            }
            cl[v31].viewangles[0] += v43;
        }
    }
}
