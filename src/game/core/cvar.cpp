// ============================================================================
// cvar.cpp - console variables (core.o cvar.cpp)
// Reconstructed from IDA release decompiles (ea comments below).
// ============================================================================

#include "game/core/core_types.h"
#include "game/core/core_systems.h"
#include "game/core/core_globals.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ============================================================================
// Externs
// ============================================================================
extern void Com_Error(int code, const char* fmt, ...);
extern void Com_Printf(const char* fmt, ...);
extern void Com_DPrintf(const char* fmt, ...);
extern void Com_sprintf(char* dest, int size, const char* fmt, ...);
extern int Com_Filter(char* filter, char* name, int casesensitive);
extern void Com_CvarDump(int type);
extern void Info_SetValueForKey(char* s, const char* key, const char* value);
extern void Info_SetValueForKey_Big(char* s, const char* key,
                                    const char* value);
extern int Cmd_Argc();
extern const char* Cmd_Argv(int arg);
extern void Cmd_AddCommand(const char* cmd_name, void (*function)());
extern int Q_stricmp(const char* s1, const char* s2);
extern char* CopyStringInternal(const char* in);
extern void mem_heap_free(void* ptr);
extern char* va(const char* fmt, ...);
extern void Q_strncpyz(char* dest, const char* src, int destsize);
extern int FS_Printf(int h, const char* fmt, ...);

// Forward declarations
cvar_t* Cvar_Set2(const char* var_name, const char* value, int force);

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
// cvar.cpp data
// ============================================================================
cvar_t* cvar_cheats;
cvar_t cvar_indexes[630];
cvar_t* hashTable[256];
char sCvarBuff1[1024];
char sCvarBuff2[4096];

// ea: 0x004C37A0
int generateHashValue(const char* fname)
{
    const char* v1 = fname;
    if (fname == nullptr)
        Com_Error(1, "null name in generateHashValue");
    char v2 = *v1;
    unsigned char v3 = 0;
    int i = 0;
    if (*v1 != 0)
    {
        do
        {
            v3 = (unsigned char)(v3 + (i + 119) * tolower((unsigned char)v2));
            v2 = *++v1;
            ++i;
        }
        while (v2 != 0);
    }
    return v3;
}

// ============================================================================
// Cvar system
// ============================================================================

// ea: 0x004BC920
void Cvar_Shutdown()
{
    for (unsigned int i = 0; i < 256; ++i)
    {
        for (cvar_t* j = hashTable[i]; j != nullptr; j = j->hashNext)
        {
            if (j->latchedString != nullptr)
            {
                mem_heap_free(j->latchedString);
                j->latchedString = nullptr;
            }
            if (j->string != nullptr)
            {
                mem_heap_free(j->string);
                j->string = nullptr;
            }
            if (j->resetString != nullptr)
            {
                mem_heap_free(j->resetString);
                j->resetString = nullptr;
            }
            ASSERT("var->name", "c:\\cod\\code\\game\\cvar.cpp", 138);
            mem_heap_free(j->name);
            j->name = nullptr;
        }
        hashTable[i] = nullptr;
    }
    cvar_numIndexes = 0;
    cvar_vars = nullptr;
    cvar_cheats = nullptr;
    cvar_modifiedFlags = 0;
}

// ea: 0x004BCA10
void Cvar_CommandCompletion(void (*callback)(const char*))
{
    for (cvar_t* i = cvar_vars; i != nullptr; i = i->next)
        callback(i->name);
}

// ea: 0x004BCA40
void Cvar_List_f()
{
    char* v0;
    if (Cmd_Argc() <= 1)
        v0 = nullptr;
    else
        v0 = (char*)Cmd_Argv(1);
    cvar_t* v1 = cvar_vars;
    int i = 0;
    for (; v1 != nullptr; ++i)
    {
        if (v0 == nullptr || Com_Filter(v0, v1->name, 0) != 0)
        {
            if ((v1->flags & 4) != 0)
                Com_Printf("S");
            else
                Com_Printf(" ");
            Com_Printf(" ");
            if ((v1->flags & 0x40) != 0)
                Com_Printf("R");
            else
                Com_Printf(" ");
            if ((v1->flags & 0x10) != 0)
                Com_Printf("I");
            else
                Com_Printf(" ");
            if ((v1->flags & 1) != 0)
                Com_Printf("A");
            else
                Com_Printf(" ");
            if ((v1->flags & 0x20) != 0)
                Com_Printf("L");
            else
                Com_Printf(" ");
            if ((v1->flags & 0x200) != 0)
                Com_Printf("C");
            else
                Com_Printf(" ");
            Com_Printf(" %s \"%s\"\n", v1->name, v1->string);
        }
        v1 = v1->next;
    }
    Com_Printf("\n%i total cvars\n", i);
    Com_Printf("%i cvar indexes\n", cvar_numIndexes);
}

// ea: 0x004BCCA0
char* Cvar_InfoString(int bit)
{
    cvar_t* v1 = cvar_vars;
    sCvarBuff1[0] = 0;
    for (; v1 != nullptr; v1 = v1->next)
    {
        if ((bit & v1->flags) != 0)
            Info_SetValueForKey(sCvarBuff1, v1->name, v1->string);
    }
    return sCvarBuff1;
}

// ea: 0x004BCCF0
char* Cvar_InfoString_Big(int bit)
{
    cvar_t* v1 = cvar_vars;
    sCvarBuff2[0] = 0;
    for (; v1 != nullptr; v1 = v1->next)
    {
        if ((bit & v1->flags) != 0)
            Info_SetValueForKey_Big(sCvarBuff2, v1->name, v1->string);
    }
    return sCvarBuff2;
}

// ea: 0x004BCD40
void Cvar_InfoStringBuffer(int bit, char* buff, int buffsize)
{
    const char* v3 = Cvar_InfoString(bit);
    Q_strncpyz(buff, v3, buffsize);
}

// ea: 0x004C0770
void Cvar_WriteVariables(int f)
{
    char buffer[1024];
    for (cvar_t* i = cvar_vars; i != nullptr; i = i->next)
    {
        if (Q_stricmp(i->name, "cl_cdkey") != 0 && (i->flags & 1) != 0)
        {
            const char* latchedString = i->latchedString;
            if (latchedString == nullptr)
                latchedString = i->string;
            Com_sprintf(buffer, 1024, "seta %s \"%s\"\n", i->name, latchedString);
            FS_Printf(f, "%s", buffer);
        }
    }
}

// ea: 0x004C07F0
void Cvar_WriteDefaults(int f)
{
    char buffer[1024];
    for (cvar_t* i = cvar_vars; i != nullptr; i = i->next)
    {
        if (Q_stricmp(i->name, "cl_cdkey") != 0 && (i->flags & 0x12C0) == 0)
        {
            Com_sprintf(buffer, 1024, "set %s \"%s\"\n", i->name, i->resetString);
            FS_Printf(f, "%s", buffer);
        }
    }
}

// ea: 0x004C0870
void Cvar_Dump_f()
{
    Com_CvarDump(0);
}

// ea: 0x004C37F0
cvar_t* Cvar_FindVar(const char* var_name)
{
    cvar_t* v1 = hashTable[generateHashValue(var_name)];
    if (v1 == nullptr)
        return nullptr;
    while (Q_stricmp(var_name, v1->name) != 0)
    {
        v1 = v1->hashNext;
        if (v1 == nullptr)
            return nullptr;
    }
    return v1;
}

// ea: 0x004C3840
cvar_t* Cvar_Get(const char* var_name, const char* var_value, int flags)
{
    if (!var_name || !var_value)
        Com_Error(0, "Cvar_Get: NULL parameter");
    if (!var_name || strchr(var_name, 92) || strchr(var_name, 34)
        || strchr(var_name, 59))
    {
        const char* v3 = va("invalid cvar name string: %s", var_name);
        Com_Error(0, v3);
    }
    cvar_t* Var = Cvar_FindVar(var_name);
    cvar_t* v5 = Var;
    if (Var)
    {
        int v6 = Var->flags;
        if ((v6 & 0x1080) != 0 && (flags & 0x1080) == 0
            && (*var_value || (flags & 0x200) != 0))
        {
            v5->flags = v6 & 0xFFFFEF7F;
            mem_heap_free(v5->resetString);
            v5->resetString = CopyStringInternal(var_value);
            cvar_modifiedFlags |= flags;
        }
        char* resetString = v5->resetString;
        v5->flags |= flags;
        if (*resetString)
        {
            if (*var_value && strcmp(resetString, var_value))
            {
                Com_DPrintf(
                    "Warning: cvar \"%s\" given initial values: \"%s\" and "
                    "\"%s\"\n",
                    var_name, v5->resetString, var_value);
            }
        }
        else
        {
            mem_heap_free(resetString);
            v5->resetString = CopyStringInternal(var_value);
        }
        char* latchedString = v5->latchedString;
        if (latchedString)
        {
            v5->latchedString = nullptr;
            Cvar_Set2(var_name, latchedString, 1);
            mem_heap_free(latchedString);
        }
        if ((v5->flags & 0x200) != 0 && cvar_cheats && !cvar_cheats->integer)
        {
            Cvar_Set2(var_name, var_value, 1);
            return v5;
        }
    }
    else
    {
        if (cvar_numIndexes >= 630)
            Com_Error(0, "MAX_CVARS");
        v5 = &cvar_indexes[cvar_numIndexes++];
        v5->name = CopyStringInternal(var_name);
        char* v10 = CopyStringInternal(var_value);
        v5->string = v10;
        v5->modified = 1;
        v5->modificationCount = 1;
        v5->value = atof(v10);
        v5->integer = atoi(v5->string);
        v5->resetString = CopyStringInternal(var_value);
        cvar_t** v11 = &cvar_vars;
        if (cvar_vars)
        {
            do
            {
                if (_stricmp(v5->name, (*v11)->name) < 0)
                    break;
                v11 = &(*v11)->next;
            }
            while (*v11);
        }
        v5->next = *v11;
        *v11 = v5;
        v5->flags = flags;
        int HashValue = generateHashValue(var_name);
        v5->hashNext = hashTable[HashValue];
        hashTable[HashValue] = v5;
    }
    return v5;
}

// ea: 0x004C3AA0
cvar_t* Cvar_Set2(const char* var_name, const char* value, int force)
{
    const char* resetString = value;
    va("      cvar set %s %s\n", var_name, value);
    char* mutableValue = (char*)value;
    if (mutableValue != nullptr)
    {
        for (unsigned int i = 0; i < strlen(mutableValue); ++i)
        {
            if (mutableValue[i] == 92)
                mutableValue[i] = 47;
        }
    }
    if (var_name == nullptr || strchr(var_name, 92) != nullptr
        || strchr(var_name, 34) != nullptr || strchr(var_name, 59) != nullptr)
    {
        const char* v5 = va("invalid cvar name string: %s", var_name);
        Com_Error(0, v5);
    }
    cvar_t* v6 = hashTable[generateHashValue(var_name)];
    if (v6 != nullptr)
    {
        while (Q_stricmp(var_name, v6->name) != 0)
        {
            v6 = v6->hashNext;
            if (v6 == nullptr)
                goto notFound;
        }
        if (value == nullptr)
        {
            resetString = v6->resetString;
            value = resetString;
        }
        if (strcmp(resetString, v6->string) == 0)
        {
            if ((v6->flags & 0x20) != 0 && v6->latchedString != nullptr)
            {
                mem_heap_free(v6->latchedString);
                v6->latchedString = nullptr;
                return v6;
            }
            return v6;
        }
        cvar_modifiedFlags |= v6->flags;
        if (force != 0)
        {
            if (v6->latchedString != nullptr)
            {
                mem_heap_free(v6->latchedString);
                v6->latchedString = nullptr;
            }
        }
        else
        {
            int flags = v6->flags;
            if ((flags & 0x40) != 0)
            {
                Com_Printf("%s is read only.\n", var_name);
                return v6;
            }
            if ((flags & 0x10) != 0)
            {
                Com_Printf("%s is write protected.\n", var_name);
                return v6;
            }
            if ((flags & 0x200) != 0 && cvar_cheats->integer == 0)
            {
                Com_Printf("%s is cheat protected.\n", var_name);
                return v6;
            }
            if ((flags & 0x20) != 0)
            {
                if (v6->latchedString != nullptr)
                {
                    if (strcmp(resetString, v6->latchedString) != 0)
                    {
                        mem_heap_free(v6->latchedString);
                        resetString = value;
                        Com_Printf("%s will be changed upon restarting.\n",
                                   var_name);
                        v6->latchedString = CopyStringInternal(resetString);
                        v6->modified = 1;
                        return v6;
                    }
                }
                else if (strcmp(resetString, v6->string) != 0)
                {
                    Com_Printf("%s will be changed upon restarting.\n",
                               var_name);
                    v6->latchedString = CopyStringInternal(resetString);
                    v6->modified = 1;
                    return v6;
                }
                return v6;
            }
        }
        if (strcmp(resetString, v6->string) != 0)
        {
            int v9 = v6->modificationCount + 1;
            char* string = v6->string;
            v6->modified = 1;
            v6->modificationCount = v9;
            mem_heap_free(string);
            char* v10 = CopyStringInternal(value);
            v6->string = v10;
            v6->value = atof(v10);
            v6->integer = atoi(v6->string);
        }
        return v6;
    }
notFound:
    if (value == nullptr)
        return nullptr;
    if (force != 0)
        return Cvar_Get(var_name, value, 0);
    return Cvar_Get(var_name, value, 128);
}

// ea: 0x004C3DD0
void Cvar_Set(const char* var_name, const char* value)
{
    Cvar_Set2(var_name, value, 1);
}

// ea: 0x004C3DF0
void Cvar_SetLatched(const char* var_name, const char* value)
{
    Cvar_Set2(var_name, value, 0);
}

// ea: 0x004C3E10
void Cvar_SetValue(const char* var_name, float value)
{
    char val[32];
    if (value == value)
        Com_sprintf(val, 32, "%i", (int)value);
    else
        Com_sprintf(val, 32, "%f", value);
    Cvar_Set2(var_name, val, 1);
}

// ea: 0x004C3E80
void Cvar_Reset(const char* var_name)
{
    Cvar_Set2(var_name, nullptr, 0);
}

// ea: 0x004C3EA0
void Cvar_Update(vmCvar_t* vmCvar)
{
    ASSERT("vmCvar", "c:\\cod\\code\\game\\cvar.cpp", 473);
    if (vmCvar->handle >= cvar_numIndexes)
        Com_Error(1, "cvar index out of range");
    cvar_t* v1 = &cvar_indexes[vmCvar->handle];
    int modificationCount = v1->modificationCount;
    if (modificationCount != vmCvar->modificationCount && v1->string != nullptr)
    {
        vmCvar->modificationCount = modificationCount;
        if (strlen(v1->string) + 1 > 0x80)
            Com_Error(1, "cvar string too long: %s (%i > %i)", v1->string,
                      (int)strlen(v1->string), 128);
        Q_strncpyz(vmCvar->string, v1->string, 128);
        vmCvar->value = v1->value;
        vmCvar->integer = v1->integer;
    }
}

// ea: 0x004C3F90
float Cvar_VariableValue(const char* var_name)
{
    cvar_t* Var = Cvar_FindVar(var_name);
    if (Var != nullptr)
        return Var->value;
    return 0.0f;
}

// ea: 0x004C3FB0
int Cvar_VariableIntegerValue(const char* var_name)
{
    cvar_t* result = Cvar_FindVar(var_name);
    if (result != nullptr)
        return result->integer;
    return 0;
}

// ea: 0x004C3FD0
char* Cvar_VariableString(const char* var_name)
{
    cvar_t* Var = Cvar_FindVar(var_name);
    if (Var != nullptr)
        return Var->string;
    return (char*)"";
}

// ea: 0x004C3FF0
void Cvar_VariableStringBuffer(const char* var_name, char* buffer, int bufsize)
{
    cvar_t* Var = Cvar_FindVar(var_name);
    if (Var != nullptr)
        Q_strncpyz(buffer, Var->string, bufsize);
    else
        *buffer = 0;
}

// ea: 0x004C4020
void Cvar_SetCheatState()
{
    for (cvar_t* i = cvar_vars; i != nullptr; i = i->next)
    {
        if ((i->flags & 0x200) != 0
            && strcmp(i->resetString, i->string) != 0)
        {
            Cvar_Set2(i->name, i->resetString, 1);
        }
    }
}

// ea: 0x004C4090
int Cvar_Command()
{
    const char* v0 = Cmd_Argv(0);
    cvar_t* result = Cvar_FindVar(v0);
    if (result != 0)
    {
        if (Cmd_Argc() == 1)
        {
            Com_Printf("\"%s\" is:\"%s^7\" default:\"%s^7\"\n", result->name,
                       result->string, result->resetString);
            char* v3 = result->latchedString;
            if (v3 != nullptr)
            {
                Com_Printf("latched: \"%s\"\n", v3);
                return 1;
            }
        }
        else
        {
            const char* v4 = Cmd_Argv(1);
            Cvar_Set2(result->name, v4, 0);
        }
        return 1;
    }
    return 0;
}

// ea: 0x004C4110
void Cvar_Toggle_f()
{
    if (Cmd_Argc() >= 2)
    {
        if (Cmd_Argc() == 2)
        {
            const char* v0 = Cmd_Argv(1);
            cvar_t* Var = Cvar_FindVar(v0);
            float value = 0.0f;
            if (Var != nullptr)
                value = Var->value;
            const char* v12 = va("%i", value == 0);
            const char* v3 = Cmd_Argv(1);
            Cvar_Set2(v3, v12, 0);
        }
        else
        {
            const char* v4 = Cmd_Argv(1);
            cvar_t* v5 = Cvar_FindVar(v4);
            const char* string = "";
            if (v5 != nullptr)
                string = v5->string;
            int v7 = 2;
            const char* v9;
            if (Cmd_Argc() - 1 <= 2)
            {
                v9 = Cmd_Argv(2);
            }
            else
            {
                while (1)
                {
                    int v8 = strcmp(string, Cmd_Argv(v7++));
                    if (v8 == 0)
                        break;
                    if (v7 >= Cmd_Argc() - 1)
                    {
                        v9 = Cmd_Argv(2);
                        break;
                    }
                }
                v9 = Cmd_Argv(v7);
            }
            const char* v11 = v9;
            const char* v10 = Cmd_Argv(1);
            Cvar_Set2(v10, v11, 0);
        }
    }
    else
    {
        Com_Printf("usage: toggle <variable> <optional value sequence>\n");
    }
}

// ea: 0x004C4230
void Cvar_Set_f()
{
    int c = Cmd_Argc();
    if (c >= 3)
    {
        char combined[4096];
        int v0 = 2;
        int v1 = 0;
        combined[0] = 0;
        int v6 = c;
        do
        {
            int v13 = v1 + (int)strlen(Cmd_Argv(v0)) + 1;
            if (v13 >= 4094)
                break;
            strcat(combined, Cmd_Argv(v0));
            if (v0 != c - 1)
                strcat(combined, " ");
            v1 = v13;
            ++v0;
        }
        while (v0 < v6);
        const char* v9 = Cmd_Argv(1);
        Cvar_Set2(v9, combined, 0);
    }
    else
    {
        Com_Printf("usage: set <variable> <value>\n");
    }
}

// ea: 0x004C4330
void Cvar_SetS_f()
{
    if (Cmd_Argc() == 3)
    {
        Cvar_Set_f();
        const char* v0 = Cmd_Argv(1);
        cvar_t* Var = Cvar_FindVar(v0);
        if (Var != nullptr)
            Var->flags |= 4u;
    }
    else
    {
        Com_Printf("usage: sets <variable> <value>\n");
    }
}

// ea: 0x004C4370
void Cvar_SetA_f()
{
    if (Cmd_Argc() == 3)
    {
        Cvar_Set_f();
        const char* v0 = Cmd_Argv(1);
        cvar_t* Var = Cvar_FindVar(v0);
        if (Var != nullptr)
            Var->flags |= 1u;
    }
    else
    {
        Com_Printf("usage: seta <variable> <value>\n");
    }
}

// ea: 0x004C43B0
void Cvar_SetFromCvar_f()
{
    if (Cmd_Argc() == 3)
    {
        const char* v0 = Cmd_Argv(2);
        cvar_t* Var = Cvar_FindVar(v0);
        const char* string;
        if (Var != nullptr)
            string = Var->string;
        else
            string = "";
        const char* v4 = string;
        const char* v3 = Cmd_Argv(1);
        Cvar_Set2(v3, v4, 0);
    }
    else
    {
        Com_Printf("usage: setfromcvar <variable> <variablein>\n");
    }
}

// ea: 0x004C4400
void Cvar_Reset_f()
{
    if (Cmd_Argc() == 2)
    {
        const char* v0 = Cmd_Argv(1);
        Cvar_Set2(v0, nullptr, 0);
    }
    else
    {
        Com_Printf("usage: reset <variable>\n");
    }
}

// ea: 0x004C4430
void Cvar_Restart_f()
{
    cvar_t* v0 = cvar_vars;
    cvar_t** p_next = &cvar_vars;
    if (cvar_vars != nullptr)
    {
        do
        {
            int flags = v0->flags;
            if ((flags & 0x450) == 0)
            {
                if ((flags & 0x80u) != 0)
                {
                    *p_next = v0->next;
                    if (v0->name != nullptr)
                        mem_heap_free(v0->name);
                    if (v0->string != nullptr)
                        mem_heap_free(v0->string);
                    if (v0->latchedString != nullptr)
                        mem_heap_free(v0->latchedString);
                    if (v0->resetString != nullptr)
                        mem_heap_free(v0->resetString);
                    v0->name = nullptr;
                    goto nextCvar;
                }
                Cvar_Set2(v0->name, v0->resetString, 1);
            }
            p_next = &v0->next;
        nextCvar:
            v0 = *p_next;
        }
        while (*p_next != nullptr);
    }
}

// ea: 0x004C44C0
void Cvar_AddCommands()
{
    Cmd_AddCommand("toggle", Cvar_Toggle_f);
    Cmd_AddCommand("set", Cvar_Set_f);
    Cmd_AddCommand("sets", Cvar_SetS_f);
    Cmd_AddCommand("seta", Cvar_SetA_f);
    Cmd_AddCommand("setfromcvar", Cvar_SetFromCvar_f);
    Cmd_AddCommand("reset", Cvar_Reset_f);
    Cmd_AddCommand("cvar_restart", Cvar_Restart_f);
    Cmd_AddCommand("cvarlist", Cvar_List_f);
    Cmd_AddCommand("cvardump", Cvar_Dump_f);
}

// ea: 0x004CA590
void Cvar_VMSet(vmCvar_t* vmCvar, const char* value)
{
    Cvar_Set2(cvar_indexes[vmCvar->handle].name, value, 1);
    Cvar_Update(vmCvar);
}

// ea: 0x004CA5C0
void Cvar_Register(vmCvar_t* vmCvar, const char* varName,
                   const char* defaultValue, int flags)
{
    cvar_t* v4 = Cvar_Get(varName, defaultValue, flags);
    if (vmCvar != nullptr)
    {
        vmCvar->handle = (int)(v4 - cvar_indexes);
        vmCvar->modificationCount = -1;
        Cvar_Update(vmCvar);
    }
}

// ea: 0x004CA610
void Cvar_Init()
{
    cvar_cheats = Cvar_Get("sv_cheats", "0", 72);
    Cvar_AddCommands();
}
