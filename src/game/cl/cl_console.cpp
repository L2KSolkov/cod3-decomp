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
extern char* Cmd_Argv(int arg);
extern int FS_FOpenFileWrite(const char* filename);
extern unsigned int FS_Write(char* buffer, unsigned int len, int h);
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
extern int CL_ConsolePrint_AddLine(int type, const char* txt, int duration,
                                   int linewidth, int color, int flags);
extern void CL_AddConsoleInfoColor(int iFirstInfo, const float* vColor);
extern void CL_AddDeathMessageText(const char* pszString, int iForceColor);
extern void* FEManager_GetFont(void* self, int f, float scale);
struct FEManager; extern FEManager g_femanager;
struct nglFont;
extern void nglGetStringDimensions(nglFont* font, unsigned int* width,
                                   unsigned int* height, float scaleX,
                                   float scaleY, const char* fmt, ...);
extern bool View_IsSplitScreen();
extern void SCR_FillRect(float x, float y, float width, float height,
                         const float* color);
extern void SCR_DrawSmallChar(int x, int y, int ch);
extern void CL_LookupColor(unsigned char c, float* color);
extern void Field_Draw(field_t* edit, int x, int y, int showCursor);
int dword_F13324;  // ?dword_F13324@@3HA (cl.o)
extern int dword_CE8814[64];
extern int dword_CE8818[64];
extern int dword_CE881C[64];
extern char* va(const char* fmt, ...);
extern const char* SEH_StringEd_GetString(const char* pszReference);

// re renderer externs (console text painting)
struct re_console_api {
    void SetColor(const float* color);
    void DrawStretchPic(float a1, float a2, float a3, float a4, float a5,
                        float a6, float a7, float a8, int a9);
    int Text_ConsoleWidth(const short* a1, int a2, float a3, float a4, int a5);
    void Text_ConsolePaint(float a1, float a2, int a3, float a4,
                           const float* a5, const short* a6, float a7,
                           int a8, int a9);
    void Text_Paint(float a1, float a2, int a3, float a4, const float* a5,
                    const char* a6, float a7, int a8, int a9);
};
extern re_console_api re;

namespace AeAssert {
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
                    FS_Write((char*)buffer, (unsigned int)strlen(buffer), f);
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

// ============================================================================
// Console / HUD drawing (cl.o cl_console.cpp)
// ============================================================================

// ea: 0x5295A0
void Con_DrawMiniConsole(int iXPos, int iYPos, float fAlpha)
{
}

// ea: 0x5295B0
void Con_DrawSubtitles(int iXPos, int iYPos, float fAlpha,
                       msgwnd_mode_t eMode)
{
}

// ea: 0x52A270
void Con_DrawStringOnHUD(int iXPos, int iYPos, const short* psString,
                         int iLength, float fAlpha, int bCentered)
{
    float vColor[4];
    vColor[0] = 1.0f;
    vColor[1] = 1.0f;
    vColor[2] = 1.0f;
    vColor[3] = fAlpha;
    int v6 = 0;
    if (bCentered != 0)
    {
        v6 = 4;
        iXPos += re.Text_ConsoleWidth(psString, 4, 0.34999999f, 0.0f,
                                      iLength) / -2;
    }
    re.Text_ConsolePaint((float)iXPos, (float)(iYPos + 18), v6, 0.34999999f,
                         vColor, psString, 0.0f, iLength, 3);
}

// ea: 0x5304E0
void Con_DrawBoldMessages(int iXPos, int iYPos, float fAlpha,
                          msgwnd_mode_t eMode)
{
    Con_DrawMessageWindow(&msgwnd, iXPos, iYPos, fAlpha, eMode);
}

// ea: 0x533450
void Con_DrawNotify(int iXPos, int iYPos, float fAlpha, msgwnd_mode_t eMode)
{
    Con_DrawMessageWindow((msgwnd_t*)&con.gamemsg_starttimes, iXPos, iYPos,
                          fAlpha, eMode);
}

// ea: 0x5333E0
void Con_DrawInput()
{
    if (cls.state == 0 || (cls.keyCatchers & 1) != 0)  // CA_DISCONNECTED
    {
        int v0 = (int)(con.vislines - 24.0f);
        re.SetColor((const float*)&dword_F13324);
        SCR_DrawSmallChar((int)con.xadjust, v0, 93);
        Field_Draw(&g_consoleField, (int)(con.xadjust + 8.0f), v0, 1);
    }
}

// ea: 0x533480
void Con_DrawSolidConsole(float frac)
{
    int v1 = (int)((float)dword_F171A0 * frac);
    if (v1 > 0)
    {
        float v2 = (float)dword_F1719C * 0.0015625f;
        float v3 = (frac * 480.0f) - 2.0f;
        if (v1 > dword_F171A0)
            v1 = dword_F171A0;
        int v4 = (int)v3;
        con.xadjust = v2 * 8.0f;
        if (v3 >= 1.0f)
        {
            re.DrawStretchPic(v2 * 0.0f,
                              ((float)dword_F171A0 * 0.0020833334f) * 0.0f,
                              v2 * 640.0f,
                              (float)v4 * ((float)dword_F171A0 * 0.0020833334f),
                              0.0f, 0.0f, 1.0f, 1.0f, dword_F171BC);
        }
        else
        {
            v4 = 0;
        }
        float color[4];
        color[0] = 0.0f;
        color[1] = 0.0f;
        color[2] = 0.0f;
        color[3] = 0.0f;
        SCR_FillRect(0.0f, (float)v4, 640.0f, 2.0f, color);
        con.vislines = v1;
        int rows = (int)((v1 - 8.0f) * 0.125f);
        int v5 = v1 - 36;
        if (con.display != con.current)
        {
            CL_LookupColor(0x37, color);
            re.SetColor(color);
            for (int i = 0; i < con.linewidth; i += 4)
            {
                SCR_DrawSmallChar((int)(((i + 1) * 8.0f) + con.xadjust), v5,
                                  94);
            }
            v5 = v5 - 12;
            --rows;
        }
        int display = con.display;
        if (con.x == 0)
            display = con.display - 1;
        unsigned char v8 = ColorIndex(0x37);
        int v10;
        if (v8 >= 8u)
        {
            color[0] = 1.0f;
            color[1] = 1.0f;
            color[2] = 1.0f;
            color[3] = 1.0f;
            v10 = 1065353216;
        }
        else
        {
            unsigned int v9 = 4 * v8;
            color[0] = (float)g_color_table[v9 / 4][0];
            color[1] = (float)dword_CE8814[v9];
            color[2] = (float)dword_CE8818[v9];
            v10 = dword_CE881C[v9];
        }
        int v11 = 0;
        for (int j = v10; v11 < rows; v5 = v5 - 12)
        {
            if (display < 0)
                break;
            if (con.current - display < con.totallines)
            {
                re.Text_ConsolePaint(
                    con.xadjust * (640.0f / (float)dword_F1719C),
                    (float)(v5 + 12) * (480.0f / (float)dword_F171A0), 5,
                    (480.0f / (float)dword_F171A0) * 0.5f, color,
                    &con.text[con.linewidth * (display % con.totallines)],
                    (640.0f / (float)dword_F1719C) * 8.0f, con.linewidth, 0);
            }
            ++v11;
            --display;
        }
        if (cls.state == 0 || (cls.keyCatchers & 1) != 0)  // CA_DISCONNECTED
        {
            int v12 = (int)(con.vislines - 24.0f);
            re.SetColor((const float*)&dword_F13324);
            float v16[4];
            v16[0] = 1.0f;
            v16[1] = 1.0f;
            v16[2] = 1.0f;
            v16[3] = 1.0f;
            float v13 = 640.0f / (float)dword_F1719C;
            float v21 = con.xadjust * v13;
            float v14 = 480.0f / (float)dword_F171A0;
            float v22 = (v12 + 12.0f) * v14;
            float v23 = v14 * 0.5f;
            const char* v15 = va("%c", 93);
            re.Text_Paint(v21, v22, 5, v23, v16, v15, v13 * 8.0f, 0, 0);
            Field_Draw(&g_consoleField, (int)(con.xadjust + 8.0f), v12, 1);
        }
        re.SetColor(nullptr);
    }
}

// ea: 0x5338E0
void Con_DrawConsole()
{
    Con_CheckResize();
    if (cls.state == 0)  // CA_DISCONNECTED
    {
        if ((cls.keyCatchers & 2) != 0)
        {
            if (con.displayFrac != 0.0f)
                Con_DrawSolidConsole(con.displayFrac);
            return;
        }
        Con_DrawSolidConsole(1.0f);
        return;
    }
    if (cls.state != 1)  // CA_LOADING
    {
        if (cls.state == 2)  // CA_ACTIVE
        {
            if (con.displayFrac == 0.0f)
                return;
            if (con_debug->integer == 2)
            {
                Con_DrawSolidConsole(con.displayFrac * 2.0f);
                return;
            }
        }
        if (con.displayFrac != 0.0f)
            Con_DrawSolidConsole(con.displayFrac);
        return;
    }
    if (con_debug->integer != 0 && (cls.keyCatchers & 2) == 0)
        Con_DrawSolidConsole(1.0f);
}

// ============================================================================
// Console print queue (cl.o cl_console.cpp)
// ============================================================================

// ea: 0x52F790
void CL_ConsolePrint(print_msg_type_t type, const char* txt, int duration,
                     int linewidth, int flags)
{
    const char* v5 = txt;
    if (txt == nullptr)
    {
        ASSERT("txt != 0", "c:\\cod\\code\\game\\cl_console.cpp", 1259);
    }
    if ((cl_noprint == nullptr || cl_noprint->integer == 0)
        && type != PMSG_LOGFILE)
    {
        if (con.initialized == 0)
        {
            Con_OneTimeInit();
            if (con.initialized == 0)
            {
                ASSERT("con.initialized", "c:\\cod\\code\\game\\cl_console.cpp",
                       1270);
            }
        }
        bool v6 = duration < 0;
        if (duration == 0)
        {
            switch (type)
            {
            case PMSG_CONSOLE:
            case PMSG_SUBTITLE:
                goto addLine;
            case PMSG_GAME:
                duration = (int)((con_gamemessagetime->value * 1000.0f)
                                 + 0.5f);
                v6 = duration < 0;
                break;
            case PMSG_BOLDGAME:
                duration = (int)((con_boldgamemessagetime->value * 1000.0f)
                                 + 0.5f);
                v6 = duration < 0;
                break;
            default:
                AeAssert::gCurrentAuthor = AeAssert::COD3;
                AeAssert::gCurrentFile = "c:\\cod\\code\\game\\cl_console.cpp";
                AeAssert::gCurrentLine = 1295;
                AeAssert::gCurrentExpr = nullptr;
                if (!AeAssert::IsIgnored() && AeAssert::Warning("unhandled case"))
                    __debugbreak();
                goto addLine;
            }
        }
        if (v6)
            duration = 0;
    addLine:
        unsigned char v7 = ColorIndex(0x37);
        int v8 = v7;
        if (type == PMSG_GAME || type == PMSG_BOLDGAME)
        {
            char szString[4096];
            if (strstr(txt, "\n") != nullptr)
            {
                const char* v9 = strstr(txt, "\n");
                while (1)
                {
                    if (*v5 == 10)
                    {
                        Con_Linefeed(type, duration, flags);
                        ++v5;
                    }
                    else
                    {
                        unsigned int v10 = (unsigned int)(v9 - v5 + 1);
                        if (v10 >= 0x1000)
                        {
                            Com_Printf("Text line too long. Clipping to fit\n");
                            v10 = 4096;
                        }
                        Q_strncpyz(szString, v5, v10);
                        v8 = CL_ConsolePrint_AddLine(type, szString, duration,
                                                     linewidth, v8, flags);
                        v5 = v9;
                        if (*v9 == 10)
                            v5 = v9 + 1;
                    }
                    if (strstr(v5, "\n") == nullptr)
                        break;
                    if (v5 != nullptr && *v5 != 0)
                    {
                        v9 = strstr(v5, "\n");
                        if (v9 != nullptr)
                            continue;
                    }
                    return;
                }
                CL_ConsolePrint_AddLine(type, v5, duration, linewidth, v8,
                                        flags);
            }
            else
            {
                CL_ConsolePrint_AddLine(type, txt, duration, linewidth, v8,
                                        flags);
            }
        }
        else
        {
            CL_ConsolePrint_AddLine(type, txt, duration, linewidth, v7,
                                    flags);
        }
    }
}

// ea: 0x5331E0
void CL_SubtitlePrint(const char* pszText, int iDuration, int iLineWidth)
{
    const char* String = SEH_StringEd_GetString(pszText);
    if (String == nullptr)
    {
        if (cl_languagewarnings->integer != 0)
        {
            if (cl_languagewarningsaserrors->integer != 0)
                Com_Error(4, "Could not translate subtitle text: \"%s\"",
                          pszText);
            else
                Com_Printf("^3WARNING: Could not translate subtitle text: "
                           "\"%s\"\n", pszText);
            String = va("^1UNLOCALIZED(^7%s^1)^7", pszText);
        }
        else
        {
            String = pszText;
        }
    }
    int v4 = iDuration;
    if (String == nullptr)
    {
        ASSERT("txt != 0", "c:\\cod\\code\\game\\cl_console.cpp", 1259);
    }
    if (cl_noprint == nullptr || cl_noprint->integer == 0)
    {
        if (con.initialized == 0)
        {
            Con_OneTimeInit();
            if (con.initialized == 0)
            {
                ASSERT("con.initialized", "c:\\cod\\code\\game\\cl_console.cpp",
                       1270);
            }
        }
        if (iDuration < 0)
            v4 = 0;
        unsigned char v5 = ColorIndex(0x37);
        CL_ConsolePrint_AddLine(PMSG_SUBTITLE, String, v4, iLineWidth, v5, 0);
    }
}

// ea: 0x52FA70
void CL_DeathMessagePrint(print_msg_type_t type, const char* pszAttackerName,
                          float* vAttackerColor, const char* pszVictimName,
                          float* vVictimColor, const char* pszIconShader,
                          float fIconWidth, float fIconHeight,
                          float* vIconColor, int iDuration)
{
    if (pszAttackerName == nullptr)
    {
        ASSERT("pszAttackerName != 0", "c:\\cod\\code\\game\\cl_console.cpp",
               1450);
    }
    if (pszVictimName == nullptr)
    {
        ASSERT("pszVictimName != 0", "c:\\cod\\code\\game\\cl_console.cpp",
               1451);
    }
    if (type != PMSG_LOGFILE)
    {
        if (con.initialized == 0)
        {
            Con_OneTimeInit();
            if (con.initialized == 0)
            {
                ASSERT("con.initialized", "c:\\cod\\code\\game\\cl_console.cpp",
                       1463);
            }
        }
        int v10 = iDuration;
        bool v11 = iDuration < 0;
        if (iDuration == 0)
        {
            float value;
            switch (type)
            {
            case PMSG_CONSOLE:
                value = con_minicontime->value;
                goto durationCalc;
            case PMSG_GAME:
                value = con_gamemessagetime->value;
                goto durationCalc;
            case PMSG_BOLDGAME:
                value = con_boldgamemessagetime->value;
            durationCalc:
                v10 = (int)((value * 1000.0f) + 0.5f);
                v11 = v10 < 0;
                break;
            case PMSG_SUBTITLE:
                v10 = 5000;
                goto durationDone;
            default:
                AeAssert::gCurrentAuthor = AeAssert::COD3;
                AeAssert::gCurrentFile = "c:\\cod\\code\\game\\cl_console.cpp";
                AeAssert::gCurrentLine = 1484;
                AeAssert::gCurrentExpr = nullptr;
                if (!AeAssert::IsIgnored() && AeAssert::Warning("unhandled case"))
                    __debugbreak();
                goto durationDone;
            }
        }
        if (v11)
            v10 = 0;
    durationDone:
        if (con.x > 0)
            Con_Linefeed(con.prevType, v10, 0);
        int v13 = ColorIndex(0x37);
        if (*pszAttackerName != 0)
        {
            CL_AddConsoleInfoColor(10, vAttackerColor);
            CL_AddDeathMessageText(pszAttackerName, v13);
            CL_AddDeathMessageText(" ", v13);
        }
        CL_AddConsoleInfoColor(13, vIconColor);
        con.text[con.x + con.linewidth * (con.current % con.totallines)] =
            (short)(((int)(fIconWidth * 32.0f)) | 0x1000);
        con.text[++con.x + con.linewidth * (con.current % con.totallines)] =
            (short)(((int)(fIconHeight * 32.0f)) | 0x1100);
        ++con.x;
        CL_AddDeathMessageText(pszIconShader, 18);
        unsigned int attacker_name_width;
        unsigned int victim_name_width;
        unsigned int dontcare;
        void* Font = FEManager_GetFont(&g_femanager, 0, 1.0f);  // FONT_GARAMOND
        nglGetStringDimensions((nglFont*)Font, &attacker_name_width,
                               &dontcare, 0.34999999f, 0.34999999f,
                               pszAttackerName);
        void* v15 = FEManager_GetFont(&g_femanager, 0, 1.0f);
        nglGetStringDimensions((nglFont*)v15, &victim_name_width,
                               &dontcare, 0.34999999f, 0.34999999f,
                               pszVictimName);
        print_msg_type_t v16;
        int v17;
        if (attacker_name_width + victim_name_width > 0xFA
            || View_IsSplitScreen())
        {
            Con_Linefeed(type, v10, 0);
            CL_AddConsoleInfoColor(10, vVictimColor);
            CL_AddDeathMessageText("    ", v13);
            CL_AddDeathMessageText(pszVictimName, v13);
            v16 = type;
            v17 = v10;
            if (type != PMSG_CONSOLE)
                goto linefeedDone;
        }
        else
        {
            CL_AddDeathMessageText(" ", v13);
            CL_AddConsoleInfoColor(10, vVictimColor);
            CL_AddDeathMessageText(pszVictimName, v13);
            v16 = type;
            v17 = v10;
            if (type != PMSG_CONSOLE)
            {
            linefeedDone:
                Con_Linefeed(type, v17, 0);
                con.prevType = v16;
                return;
            }
        }
        Con_UpdateNotifyLine(PMSG_CONSOLE, 0, v17, 0);
        con.prevType = v16;
    }
}
