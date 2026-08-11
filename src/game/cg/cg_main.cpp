// ============================================================================
// cg_main.cpp - client game main entry + register/console plumbing (cg.o)
// ============================================================================

#include "game/cg/cg_local.h"
#include "game/game_types.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

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
extern int dword_F641E4;
extern int dword_F641E8;
extern int dword_F641E0[2];
extern int dword_F641EC;
enum netsrc_t {
    NS_CLIENT = 0,
    NS_SERVER = 1,
};

extern void CL_AddCgameCommand(const char* cmdName, void (*function)());
extern const char* Info_ValueForKey(const char* s, const char* key);
extern void Cmd_ArgvBuffer(int arg, char* buffer, int bufferLength);
extern int Q_stricmp(const char* s1, const char* s2);
extern void Com_Printf(const char* fmt, ...);
struct vmCvar_t;
extern void Cvar_Register(vmCvar_t* vmCvar, const char* varName,
                          const char* defaultValue, int flags);
extern void Cvar_Update(vmCvar_t* vmCvar);
extern void Cvar_Set(const char* var_name, const char* value);
extern void Cvar_VMSet(vmCvar_t* vmCvar, const char* value);
extern void Com_Error(int code, const char* fmt, ...);
extern void CG_Init();
extern void CG_InitServerCommandHashVals();
extern void CG_InitLocalEntities();
extern void CG_ParseFog();
extern void CG_ParseObjectiveChange(int iNum);
extern void CG_CloseScriptMenu();
extern void CG_RegisterServerShader(int num);
extern void CG_RegisterWeapon(int weaponNum);
extern void CG_FreeWeapons();
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
extern void* Entity_GetRefEntity(Entity* ent);
extern void AnglesToAxis(const float* angles, float (*axis)[3]);
extern void SoundDevice_DampenAllSounds(void* sInst, float level);
extern void SoundDevice_StopAllSounds(void* sInst);
extern void SoundDevice_FrameAdvance(void* sInst, float delta);
extern void* SoundDevice_sInst;
extern void Com_FreeWeaponInfoMemory(int iSource, int bRestart);
extern void RumbleManager_Reset(void* mgr);
extern void* RumbleManager_Inst(int instance);
extern void* GetTextureData(const char* name, int image_type,
                            const char* fromPak);
extern void* bg_itemlist;      // gitem_s[]
extern void* cg_items;         // itemInfo_t[]
extern void* cgCvarTable;      // cvarTable_t[170]
extern void* cvarTable;        // cvarTable_t[170]

struct cg_t {
    unsigned char data[0x18B0];
};
extern cg_t* cg;  // 0x00F62940

struct vmCvar_t {
    int integer;  // +0x00
};
extern vmCvar_t cg_thirdPerson;

struct cgGlobal_t {
    int time;
    int oldTime;
    int teamGame;
};
extern cgGlobal_t cgGlobal;  // 0x00F5FE30


struct consoleCommand_t {
    const char* cmd;
    void (*function)();
};

// console command table (commandsList / off_D0CD44)
static const consoleCommand_t sCommandsList[] = {
    {"viewpos", nullptr},
    {nullptr, nullptr},
};

static char buffer_0[256];
static int (*syscall_)(int, ...) = nullptr;

// ea: 0x0068C860
void cg_dllEntry(int (*syscallptr)(int, ...))
{
    syscall_ = syscallptr;
}

// ea: 0x0068C8F0
int CG_UI_Popup()
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
    Com_Error(2 /* ERR_DROP */, text);
}

// ea: 0x0068B580
char* CG_Argv(int arg)
{
    Cmd_ArgvBuffer(arg, buffer_0, 256);
    return buffer_0;
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

// ea: 0x0068F5E0
void CG_RegisterItemVisuals(int itemNum)
{
    unsigned char* item = &((unsigned char*)cg_items)[8 * itemNum];
    if (item[0] == 0)
    {
        unsigned int* gitem = &((unsigned int*)bg_itemlist)[13 * itemNum];
        const char* icon = *(const char**)gitem;
        item[0] = 0;
        if (icon != nullptr)
            *(void**)(item + 4) = GetTextureData(icon, 5, "mp_frontEnd");
        if (gitem[4] == 1 /* IT_WEAPON */)
            CG_RegisterWeapon(gitem[5]);
        item[0] = 1;
    }
}

// ea: 0x0068F640
int CG_RegisterItems()
{
    char items[260];
    strcpy(items, CL_GetConfigString(8));
    unsigned char* v1 = &((unsigned char*)cg_items)[8];
    unsigned int* p_icon = &((unsigned int*)bg_itemlist)[13];
    for (int v0 = 1; v0 < 137; ++v0)
    {
        int v3 = items[v0 / 4];
        int v4 = v3 > 57 ? v3 - 87 : v3 - 48;
        if (((1 << (v0 & 3)) & v4) != 0 && v1[0] == 0)
        {
            const char* v5 = *(const char**)p_icon;
            v1[0] = 0;
            if (v5 != nullptr)
                *(void**)(v1 + 4) = GetTextureData(v5, 5, "mp_frontEnd");
            if (p_icon[4] == 1)
                CG_RegisterWeapon(p_icon[5]);
            v1[0] = 1;
        }
        v1 += 8;
        p_icon += 13;
    }
    return 137;
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
            refEntity_t* RefEntity = (refEntity_t*)Entity_GetRefEntity(entity);
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
    SoundDevice_DampenAllSounds(SoundDevice_sInst, 0.0f);
    CG_FreeWeapons();
    Com_FreeWeaponInfoMemory(2, 0);
    memset(cg, 0, sizeof(cg_t));
    void* v0 = RumbleManager_Inst(currCl);
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
    SoundDevice_StopAllSounds(SoundDevice_sInst);
    SoundDevice_FrameAdvance(SoundDevice_sInst, 0.0f);
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
    dword_F641E4 = 0;
    dword_F641E8 = 0;
    dword_F641E0[0] = -1;
    dword_F641EC = 0;
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
