// ============================================================================
// g_info.cpp - game.o infostring helpers (q_shared.cpp)
// Verified against IDA (release map offsets + 0x40C000 = VA).
// ============================================================================

#include "game/logic/g_local.h"

#include <stdio.h>
#include <string.h>

static char value1[2][8192];
static int valueindex;

extern int AeStringSupport::ae_stricmpn(const char* s1, const char* s2, int n);

// ============================================================================
// Info_ValueForKey - ea: 0x610DD0
// ============================================================================
// ea: 0x00610DD0
const char* Info_ValueForKey(const char* s, const char* key)
{
    const char* v2 = s;
    char v5;
    char* v6;
    char pkey[8192];
    if (s != nullptr && key != nullptr)
    {
        if (strlen(s) >= 0x400)
            Com_Error(ERR_DROP, "Info_ValueForKey: oversize infostring");
        int v3 = valueindex ^ 1;
        bool v4 = *s == 92;
        valueindex ^= 1u;
        if (!v4)
            goto not_backslash;
        for (;;)
        {
            ++v2;
        not_backslash:
            v5 = *v2;
            v6 = pkey;
            if (*v2 != 92)
                break;
        copy_pair:
            char v7 = *++v2;
            char* v8 = value1[v3];
            *v6 = 0;
            for (; v7 != 92; ++v2)
            {
                if (v7 == 0)
                    break;
                *v8 = v7;
                v7 = v2[1];
                ++v8;
            }
            *v8 = 0;
            if (AeStringSupport::ae_stricmpn(key, pkey, 0x7FFFFFFF) == 0)
                return value1[valueindex];
            if (*v2 == 0)
                return "";
            v3 = valueindex;
        }
        while (v5 != 0)
        {
            *v6 = v5;
            v5 = v2[1];
            ++v6;
            ++v2;
            if (v5 == 92)
                goto copy_pair;
        }
    }
    return "";
}

// ============================================================================
// Info_NextPair - ea: 0x610EC0
// ============================================================================
// ea: 0x00610EC0
void Info_NextPair(const char** head, char* key, char* value)
{
    const char* v3 = *head;
    if (**head == 92)
        ++v3;
    char* i = key;
    *key = 0;
    *value = 0;
    char v5 = *v3;
    if (*v3 == 92)
    {
    copy_pair:
        ++v3;
        *i = 0;
        char v6 = *v3;
        for (i = value; v6 != 92; ++v3)
        {
            if (v6 == 0)
                break;
            *i = v6;
            v6 = v3[1];
            ++i;
        }
    }
    else
    {
        while (v5 != 0)
        {
            *i = v5;
            v5 = v3[1];
            ++i;
            ++v3;
            if (v5 == 92)
                goto copy_pair;
        }
    }
    *i = 0;
    *head = v3;
}

// ============================================================================
// Info_RemoveKey - ea: 0x610F20
// ============================================================================
void Info_RemoveKey(char* s, const char* key)
{
    char* v2 = s;
    char v4;
    char* v5;
    char* i;
    char* v8;
    int v9;
    char v10;
    char value[1024];
    char pkey[1024];
    if (strlen(s) >= 0x400)
        Com_Error(ERR_DROP, "Info_RemoveKey: oversize infostring");
    if (strchr(key, 92) != nullptr)
        return;
    for (;;)
    {
        bool v3 = *v2 == 92;
        char* start = v2;
        if (v3)
            ++v2;
        v4 = *v2;
        v5 = pkey;
        if (*v2 != 92)
            break;
    copy_pair:
        char v6 = *++v2;
        *v5 = 0;
        i = value;
        for (; v6 != 92; ++v2)
        {
            if (v6 == 0)
                break;
            *i = v6;
            v6 = v2[1];
            ++i;
        }
        *i = 0;
        if (strcmp(key, pkey) == 0)
        {
            v8 = v2;
            v9 = (int)(start - v2);
            do
            {
                v10 = *v8;
                v8[v9] = *v8;
                ++v8;
            } while (v10 != 0);
            return;
        }
        if (*v2 == 0)
            return;
    }
    while (v4 != 0)
    {
        *v5 = v4;
        v4 = v2[1];
        ++v5;
        ++v2;
        if (v4 == 92)
            goto copy_pair;
    }
}

// ============================================================================
// Info_RemoveKey_Big - ea: 0x611030
// ============================================================================
void Info_RemoveKey_Big(char* s, const char* key)
{
    char* v2 = s;
    char v4;
    char* v5;
    char* i;
    char* v8;
    int v9;
    char v10;
    char pkey[8192];
    char value[1024];
    if (strlen(s) >= 0x400)
        Com_Error(ERR_DROP, "Info_RemoveKey_Big: oversize infostring");
    if (strchr(key, 92) != nullptr)
        return;
    for (;;)
    {
        bool v3 = *v2 == 92;
        char* start = v2;
        if (v3)
            ++v2;
        v4 = *v2;
        v5 = pkey;
        if (*v2 != 92)
            break;
    copy_pair:
        char v6 = *++v2;
        *v5 = 0;
        i = value;
        for (; v6 != 92; ++v2)
        {
            if (v6 == 0)
                break;
            *i = v6;
            v6 = v2[1];
            ++i;
        }
        *i = 0;
        if (strcmp(key, pkey) == 0)
        {
            v8 = v2;
            v9 = (int)(start - v2);
            do
            {
                v10 = *v8;
                v8[v9] = *v8;
                ++v8;
            } while (v10 != 0);
            return;
        }
        if (*v2 == 0)
            return;
    }
    while (v4 != 0)
    {
        *v5 = v4;
        v4 = v2[1];
        ++v5;
        ++v2;
        if (v4 == 92)
            goto copy_pair;
    }
}

// ============================================================================
// Info_Validate - ea: 0x611140
// ============================================================================
// ea: 0x00611140
int Info_Validate(const char* s)
{
    return strchr(s, 34) == nullptr && strchr(s, 59) == nullptr;
}

// ============================================================================
// Info_SetValueForKey - ea: 0x611170
// ============================================================================
void Info_SetValueForKey(char* s, const char* key, const char* value)
{
    int v3 = 0;
    if (value == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\q_shared.cpp";
        AeAssert::gCurrentLine = 909;
        AeAssert::gCurrentExpr = "value";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (strlen(s) >= 0x400)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\q_shared.cpp";
        AeAssert::gCurrentLine = 913;
        AeAssert::gCurrentExpr = "0";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("oversize infostring - Tell MikeA"))
            __debugbreak();
    }
    char newi[1024];
    char cleanValue[1024];
    int v4 = 0;
    do
    {
        char v5 = value[v3];
        if (v5 == 0)
            break;
        if (v5 != 92 && v5 != 59 && v5 != 34)
        {
            if (v4 >= 1024)
            {
                AeAssert::gCurrentAuthor = AeAssert::COD3;
                AeAssert::gCurrentFile = "c:\\cod\\code\\game\\q_shared.cpp";
                AeAssert::gCurrentLine = 923;
                AeAssert::gCurrentExpr = "j < 1024";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert("old cod assert"))
                    __debugbreak();
            }
            cleanValue[v4++] = v5;
        }
        ++v3;
    } while (v3 < 1023);
    if (v4 >= 1024)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\q_shared.cpp";
        AeAssert::gCurrentLine = 928;
        AeAssert::gCurrentExpr = "j < 1024";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    cleanValue[v4] = 0;
    if (strchr(key, 92) != nullptr)
        Com_Error(ERR_DROP, "Can't use keys with a \\nkey: '%s'\nvalue: '%s'\n", key, value);
    if (strchr(key, 59) != nullptr)
        Com_Error(ERR_DROP, "Can't use keys with a semicolon\nkey: '%s'\nvalue: '%s'\n", key, value);
    if (strchr(key, 34) != nullptr)
        Com_Error(ERR_DROP, "Can't use keys with a \"\nkey: '%s'\nvalue: '%s'\n", key, value);
    Info_RemoveKey(s, key);
    if (cleanValue[0] != 0)
    {
        Com_sprintf(newi, 1024, "\\%s\\%s", key, cleanValue);
        if (strlen(newi) + strlen(s) > 0x400)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\q_shared.cpp";
            AeAssert::gCurrentLine = 948;
            AeAssert::gCurrentExpr = "0";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("String length exceeded key : Tell MikeA"))
                __debugbreak();
        }
        strcat(s, newi);
    }
}

// ============================================================================
// Info_SetValueForKey_Big - ea: 0x611430
// ============================================================================
void Info_SetValueForKey_Big(char* s, const char* key, const char* value)
{
    int v3 = 0;
    if (value == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\q_shared.cpp";
        AeAssert::gCurrentLine = 968;
        AeAssert::gCurrentExpr = "value";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (strlen(s) >= 0x400)
        Com_Error(ERR_DROP, "Info_SetValueForKey: oversize infostring");
    char newi[1024];
    char cleanValue[1024];
    int v4 = 0;
    do
    {
        char v5 = value[v3];
        if (v5 == 0)
            break;
        if (v5 != 92 && v5 != 59 && v5 != 34)
        {
            if (v4 >= 1024)
            {
                AeAssert::gCurrentAuthor = AeAssert::COD3;
                AeAssert::gCurrentFile = "c:\\cod\\code\\game\\q_shared.cpp";
                AeAssert::gCurrentLine = 980;
                AeAssert::gCurrentExpr = "j < 1024";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert("old cod assert"))
                    __debugbreak();
            }
            cleanValue[v4++] = v5;
        }
        ++v3;
    } while (v3 < 1023);
    if (v4 >= 1024)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\q_shared.cpp";
        AeAssert::gCurrentLine = 985;
        AeAssert::gCurrentExpr = "j < 1024";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    cleanValue[v4] = 0;
    if (strchr(key, 92) != nullptr)
        Com_Error(ERR_DROP, "Can't use keys with a \\nkey: '%s'\nvalue: '%s'\n", key, value);
    if (strchr(key, 59) != nullptr)
        Com_Error(ERR_DROP, "Can't use keys with a semicolon\nkey: '%s'\nvalue: '%s'\n", key, value);
    if (strchr(key, 34) != nullptr)
        Com_Error(ERR_DROP, "Can't use keys with a \"\nkey: '%s'\nvalue: '%s'\n", key, value);
    Info_RemoveKey_Big(s, key);
    if (cleanValue[0] != 0)
    {
        Com_sprintf(newi, 1024, "\\%s\\%s", key, cleanValue);
        if (strlen(newi) + strlen(s) > 0x400)
            Com_Error(ERR_DROP, "BIG Info string length exceeded\nkey: '%s'\nvalue: '%s'\nInfo string: %s\n", key, value, s);
        strcat(s, newi);
    }
}
