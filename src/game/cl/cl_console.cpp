// ============================================================================
// cl_console.cpp - client console core (cl.o cl_console.cpp)
// 14+ functions, verified against IDA (release map offsets + 0x40C000 = VA).
// Render-heavy draw funcs + print-queue helpers are declared extern (ported
// with the renderer); the pure console state machine lives here.
// ============================================================================

#include "cl_console.h"

#include <string.h>

// ============================================================================
// Externs (core.o / cl.o render helpers)
// ============================================================================
extern void Com_Printf(const char* fmt, ...);
extern void Com_Error(int code, const char* fmt, ...);
extern void Cvar_Set(const char* var_name, const char* value);
extern struct cvar_t* Cvar_Get(const char* var_name, const char* var_value,
                               int flags);
extern void Cmd_AddCommand(const char* cmd_name, void (*function)());
extern int Cmd_Argc();
extern const char* Cmd_Argv(int arg);
extern int FS_FOpenFileWrite(const char* filename);
extern void FS_Write(const void* buffer, int len, int h);
extern void FS_FCloseFile(int f);
extern void Con_OneTimeInit();
extern void* mem_heap_malloc(unsigned int size);
extern void mem_heap_free(void* ptr);
extern char* va(const char* fmt, ...);
extern void Q_strncpyz(char* dest, const char* src, int destsize);
extern const char* SEH_StringEd_GetString(const char* pszReference);
extern void Con_UpdateMessageWindowLine(msgwnd_t* msgwnd, int linefeed,
                                        int duration, int flags);
extern void Con_DrawMessageWindow(msgwnd_t* msgwnd, int x, int y,
                                  float alpha, msgwnd_mode_t mode);

// ============================================================================
// Globals (cl.o data)
// ============================================================================
console_t con;
msgwnd_t msgwnd;
field_t g_consoleField;
field_t historyEditLines[32];
int g_console_char_width;
int g_console_field_width;
int g_console_char_height;
cls_t cls;
int dword_F170FC;
int dword_F1719C;
int dword_F171A0;
int dword_F171BC;
int g_color_table[64][4];
float g_color_table_flat[64 * 4];

cvar_t* con_conspeed;
cvar_t* con_debug;
cvar_t* con_gamemessagetime;
cvar_t* con_boldgamemessagetime;
cvar_t* con_minicontime;
cvar_t* cl_noprint;
cvar_t* cl_languagewarnings;
cvar_t* cl_languagewarningsaserrors;
cvar_t* com_cl_running;

// ============================================================================
// Console state machine
// ============================================================================

// ea: 0x529070
int Con_ToggleConsole_f()
{
    Field_Clear(&g_consoleField);
    g_consoleField.widthInPixels = g_console_field_width;
    g_consoleField.charWidth = (float)g_console_char_width;
    g_consoleField.charHeight = (float)g_console_char_height;
    g_consoleField.bFixedSize = 1;
    cls.keyCatchers ^= 1;
    return 1;
}

// ea: 0x5290C0
void Con_Dump_f()
{
    if (Cmd_Argc() == 2)
    {
        const char* v0 = Cmd_Argv(1);
        Com_Printf("Dumped console text to %s.\n", v0);
        const char* v1 = Cmd_Argv(1);
        int f = FS_FOpenFileWrite(v1);
        if (f != 0)
        {
            int linewidth = con.linewidth;
            int current = con.current;
            int totallines = con.totallines;
            int i;
            for (i = con.current - con.totallines + 1; i <= con.current; ++i)
            {
                int j;
                for (j = 0; j < con.linewidth; ++j)
                {
                    if ((unsigned char)con.text[con.linewidth * (i % con.totallines) + j] != 32)
                        break;
                }
                if (j != con.linewidth)
                    break;
            }
            char buffer[1024];
            buffer[con.linewidth] = 0;
            if (i <= current)
            {
                while (1)
                {
                    int k;
                    for (k = 0; k < linewidth; ++k)
                        buffer[k] = (char)con.text[linewidth * (i % totallines) + k];
                    int m;
                    for (m = linewidth - 1; m >= 0; buffer[m + 1] = 0)
                    {
                        if (buffer[m] != 32)
                            break;
                        --m;
                    }
                    strcpy(&buffer[strlen(buffer)], "\n");
                    FS_Write(buffer, (int)strlen(buffer), f);
                    if (++i > con.current)
                        break;
                    linewidth = con.linewidth;
                    totallines = con.totallines;
                }
            }
            FS_FCloseFile(f);
        }
        else
        {
            Com_Printf("ERROR: couldn't open.\n");
        }
    }
    else
    {
        Com_Printf("usage: condump <filename>\n");
    }
}

// ea: 0x5295C0
void Con_RunConsole()
{
    float v0 = 0.5f;
    if ((cls.keyCatchers & 1) == 0)
        v0 = 0.0f;
    con.finalFrac = v0;
    if (con.displayFrac <= v0)
    {
        if (v0 <= con.displayFrac)
            return;
        con.displayFrac = ((float)(dword_F170FC * con_conspeed->integer) * 0.001f) + con.displayFrac;
        if (con.displayFrac <= v0)
            return;
    }
    else
    {
        con.displayFrac = con.displayFrac - ((float)(dword_F170FC * con_conspeed->integer) * 0.001f);
        if (v0 <= con.displayFrac)
            return;
    }
    con.displayFrac = v0;
}

// ea: 0x529650
int Con_PageUp()
{
    int v1 = con.current - (con.display - 2);
    con.display -= 2;
    if (v1 >= con.totallines)
    {
        con.display = con.current - con.totallines + 1;
    }
    return con.current;
}

// ea: 0x529680
int Con_PageDown()
{
    con.display += 2;
    if (con.display > con.current)
        con.display = con.current;
    return con.display;
}

// ea: 0x5296A0
int Con_Top()
{
    con.display = con.totallines;
    if (con.current - con.totallines >= con.totallines)
        con.display = con.current - con.totallines + 1;
    return con.current - con.totallines;
}

// ea: 0x5296C0
int Con_Bottom()
{
    con.display = con.current;
    return con.current;
}

// ea: 0x529720
int Con_ClearNotify()
{
    memset(con.gamemsg_starttimes, 0, 4 * 8);
    memset(con.gamemsg_endtimes, 0, 4 * 8);
    memset(con.gamemsg_lines, 0, 4 * 8);
    memset(msgwnd.starttimes, 0, 4 * msgwnd.count);
    memset(msgwnd.endtimes, 0, 4 * msgwnd.count);
    msgwnd.current_line = 0;
    msgwnd.typingLineIndex = -1;
    return 0;
}

// ea: 0x529780
void Con_ClearMiniConsole()
{
}

// ea: 0x529790
void Con_ClearSubtitles()
{
}

// ea: 0x5297A0
int Con_CheckResize()
{
    int result = dword_F1719C;
    if (dword_F1719C < 640)
        result = 640;
    int linewidth = con.linewidth;
    int v3 = (int)((float)result * 0.125f - 2.0f);
    if (v3 != con.linewidth)
    {
        if (v3 >= 1)
        {
            int totallines = con.totallines;
            int oldwidth = con.linewidth;
            con.linewidth = v3;
            int numlines = con.totallines;
            bool v8 = 2048 / v3 < con.totallines;
            con.totallines = 2048 / v3;
            if (v8)
                numlines = 2048 / v3;
            int numchars = linewidth;
            if (v3 < linewidth)
                numchars = v3;
            short* v9 = (short*)mem_heap_malloc(0x1000);
            memcpy(v9, con.text, 4096);
            short* tbuf = v9;
            short* text = con.text;
            do
            {
                *text++ = (short)((ColorIndex(0x37) << 8) | 0x20);
            }
            while (text < &con.text[con.current]);
            for (int i = 0; i < numlines; ++i)
            {
                for (int j = 0; j < numchars; ++j)
                {
                    con.text[j + con.linewidth * (con.totallines - i - 1)] =
                        tbuf[j + oldwidth * ((totallines + con.current - i) % totallines)];
                }
            }
            mem_heap_free(tbuf);
            con.gamemsg_starttimes[3] = 0;
            memset(msgwnd.starttimes, 0, 4 * msgwnd.count);
            memset(msgwnd.endtimes, 0, 4 * msgwnd.count);
            msgwnd.current_line = 0;
            msgwnd.typingLineIndex = -1;
        }
        else
        {
            con.linewidth = 78;
            con.totallines = 26;
            short* v4 = con.text;
            do
            {
                *v4++ = (short)((ColorIndex(0x37) << 8) | 0x20);
            }
            while (v4 < &con.text[con.current]);
        }
        con.current = con.totallines - 1;
        con.display = con.totallines - 1;
        return con.totallines - 1;
    }
    return result;
}

// ea: 0x529AB0
void Con_UpdateNotifyLine(print_msg_type_t type, int bLineFeed,
                          int duration, int flags)
{
    if (con.current >= 0)
    {
        if (type >= PMSG_CONSOLE && type <= PMSG_GAME)
        {
            // console + game share the gamemsg window (msgwnd at +0x32?)
            Con_UpdateMessageWindowLine(&msgwnd, bLineFeed, duration, flags);
            return;
        }
        if (type == PMSG_BOLDGAME)
        {
            Con_UpdateMessageWindowLine(&msgwnd, bLineFeed, duration, flags);
            return;
        }
    }
}

// ea: 0x529DD0
void Con_Linefeed(print_msg_type_t type, int duration, int flags)
{
    Con_UpdateNotifyLine(type, 1, duration, flags);
    con.x = 0;
    if (con.display == con.current)
        ++con.display;
    ++con.current;
    if (con.linewidth > 0)
    {
        int v3 = 0;
        do
        {
            unsigned char v4 = ColorIndex(0x37);
            con.text[v3 + con.linewidth * (con.current % con.totallines)] =
                (short)((v4 << 8) | 0x20);
            ++v3;
        }
        while (v3 < con.linewidth);
    }
}

// ea: 0x52F550
unsigned char Con_Clear_f()
{
    short* text = con.text;
    do
    {
        unsigned char result = ColorIndex(0x37);
        *text++ = (short)((result << 8) | 0x20);
    }
    while (text < &con.text[con.current]);
    con.display = con.current;
    return ColorIndex(0x37);
}

// ea: 0x52F750
void Con_Close()
{
    if (com_cl_running->integer != 0)
    {
        Field_Clear(&g_consoleField);
        Con_ClearNotify();
        cls.keyCatchers &= ~1;
        con.finalFrac = 0.0f;
        con.displayFrac = 0.0f;
    }
}

// ea: 0x52F590
void Con_Init()
{
    con_conspeed = Cvar_Get("scr_conspeed", "3", 0);
    con_debug = Cvar_Get("con_debug", "0", 1);
    Field_Clear(&g_consoleField);
    g_consoleField.charWidth = (float)g_console_char_width;
    g_consoleField.widthInPixels = g_console_field_width;
    g_consoleField.charHeight = (float)g_console_char_height;
    g_consoleField.bFixedSize = 1;
    for (int i = 0; i < 32; ++i)
    {
        Field_Clear(&historyEditLines[i]);
        historyEditLines[i].charWidth = (float)g_console_char_width;
        historyEditLines[i].widthInPixels = g_console_field_width;
        historyEditLines[i].charHeight = (float)g_console_char_height;
        historyEditLines[i].bFixedSize = 1;
    }
    Cmd_AddCommand("clear", (void(__cdecl*)())Con_Clear_f);
    Cmd_AddCommand("condump", (void(__cdecl*)())Con_Dump_f);
}
