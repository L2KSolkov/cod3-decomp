// ============================================================================
// info.cpp - info string helpers (core.o)
// ============================================================================

#include "game/core/core_types.h"
#include "game/core/core_systems.h"

#include <string.h>

extern void Com_Printf(const char* fmt, ...);

// ea: 0x004BB7B0
void Info_Print(const char* s)
{
    const char* v1 = s;
    if (*s == 92)
        v1 = s + 1;
    char v2 = *v1;
    if (*v1 != 0)
    {
        char value[512];
        char key[512];
        while (1)
        {
            char* i = key;
            for (; v2 != 0; ++v1)
            {
                if (v2 == 92)
                    break;
                *i = v2;
                v2 = v1[1];
                ++i;
            }
            if (i - key >= 20)
            {
                *i = 0;
            }
            else
            {
                memset(i, 0x20u, 20 - (int)(i - key));
                key[20] = 0;
            }
            Com_Printf("%s", key);
            if (*v1 == 0)
                break;
            char v4 = *++v1;
            char* j = value;
            for (; v4 != 0; ++v1)
            {
                if (v4 == 92)
                    break;
                *j = v4;
                v4 = v1[1];
                ++j;
            }
            bool v6 = *v1 == 0;
            *j = 0;
            if (!v6)
                ++v1;
            Com_Printf("%s\n", value);
            v2 = *v1;
            if (*v1 == 0)
                return;
        }
        Com_Printf("MISSING VALUE\n");
    }
}
