// ============================================================================
// cl_console.h - client console data (cl.o cl_console.cpp)
// Reconstructed from IDA local types. console_t 0x1144 / field_t 0x11C.
// ============================================================================

#pragma once

#include <stdint.h>
#include "engine/broc_types.h"

// ============================================================================
// clientStatic_t - client static state (cl.o; cls @ 0x13054D0)
// The ctor/dtor vector-construct/destroy the 1024-element configstrings
// array at +0x10 (Broc::string, 4 bytes each).
// ============================================================================
struct clientStatic_t {
    uint8_t     _pad[0x10];
    Broc::string configstrings[1024];  // +0x10

    clientStatic_t();  // ??0clientStatic_t@@QAE@XZ (cl.o 0x928F10)
    ~clientStatic_t(); // ??1clientStatic_t@@QAE@XZ (cl.o 0x928CD0)
};

// ============================================================================
// print_msg_type_t / msgwnd_mode_t
// ============================================================================
enum print_msg_type_t {
    PMSG_CONSOLE = 0,
    PMSG_GAME = 1,
    PMSG_BOLDGAME = 2,
    PMSG_SUBTITLE = 3,
    PMSG_LOGFILE = 4,
};
enum msgwnd_mode_t {
    MSGWND_MODE_NORMAL = 0,
    MSGWND_MODE_SCREENSHOT = 1,
};

// ============================================================================
// field_t - editable text field (0x11C, verified)
// ============================================================================
struct field_t {
    int cursor;           // +0x00
    int scroll;           // +0x04
    int drawWidth;        // +0x08
    int widthInPixels;    // +0x0C
    float charWidth;      // +0x10
    float charHeight;     // +0x14
    int bFixedSize;       // +0x18
    char buffer[256];     // +0x1C
};
static_assert(sizeof(field_t) == 0x11C, "field_t size mismatch");

// ============================================================================
// messagewindow_t - message window (cl.o; verified against IDA)
// ============================================================================
struct messagewindow_t {
    int* starttimes;
    int* endtimes;
    int* lines;
    int current_line;
    int count;
    int padding;
    int scrolltime;
    int fadein;
    int fadeout;
    int lineflags[11];
    int displayLength[11];
    int typingLineIndex;
};
static_assert(sizeof(messagewindow_t) == 0x80,
              "messagewindow_t size mismatch");
typedef messagewindow_t msgwnd_t;

// ============================================================================
// console_t - console state (0x1144, verified)
// ============================================================================
struct console_t {
    int initialized;          // +0x000
    short text[2048];         // +0x004 (char + color byte pair)
    int current;              // +0x1004
    int x;                    // +0x1008
    int display;              // +0x100C
    print_msg_type_t prevType; // +0x1010
    int linewidth;            // +0x1014
    int totallines;           // +0x1018
    float xadjust;            // +0x101C
    float displayFrac;        // +0x1020
    float finalFrac;          // +0x1024
    int vislines;             // +0x1028
    // gamemsg + msgwnd share the tail (offset 0x102C+)
    int gamemsg_starttimes[8];  // +0x102C
    int gamemsg_endtimes[8];    // +0x104C
    int gamemsg_lines[8];       // +0x106C
    union {
        struct messagewindow_t gamemsg;
        struct {
            unsigned char gap0[36];
            int boldgamemsg_starttimes[8];
            int boldgamemsg_endtimes[8];
            int boldgamemsg_lines[8];
        } boldgamemsg;
    } windows;
    unsigned char _tail[52];
};
static_assert(sizeof(console_t) == 0x1144, "console_t size mismatch");

// ============================================================================
// Globals (cl.o data)
// ============================================================================
extern console_t con;
extern msgwnd_t msgwnd;
extern field_t g_consoleField;
extern field_t historyEditLines[32];
extern int g_console_char_width;
extern int g_console_field_width;
extern int g_console_char_height;
extern struct cls_t {
    int keyCatchers;
    int state;
    int endgamemenu;
    int cddialog;
    int servername[1024 + 32];
} cls;
extern int dword_F170FC;   // frametime (ms)
extern int dword_F1719C;   // screen width
extern int dword_F171A0;   // screen height
extern int dword_F171BC;   // console background material
extern int dword_F13324;
extern int dword_F13328;
extern int dword_F1332C;
extern int dword_F13330;
extern int g_color_table[64][4];
extern float g_color_table_flat[64 * 4];

// cvars
struct cvar_t { int integer; float value; };
extern cvar_t* con_conspeed;
extern cvar_t* con_debug;
extern cvar_t* con_gamemessagetime;
extern cvar_t* con_boldgamemessagetime;
extern cvar_t* con_minicontime;
extern cvar_t* cl_noprint;
extern cvar_t* cl_languagewarnings;
extern cvar_t* cl_languagewarningsaserrors;
extern cvar_t* com_cl_running;
extern int endtimes[8];
extern int lines[8];
extern cvar_t* cl_shownet;
extern cvar_t* cl_yawspeed;
extern cvar_t* cl_pitchspeed;
extern cvar_t* cl_anglespeedkey;
extern cvar_t* cl_mouseAccel;
extern cvar_t* cl_freelook;
extern cvar_t* cl_showMouseRate;
extern cvar_t* cl_stanceHoldTime;
extern cvar_t* cl_disable_ads;
extern cvar_t* cl_freeze;
extern cvar_t* cl_viewPitchCompensate;
extern cvar_t* cl_viewYawCompensate;

// ============================================================================
// Functions
// ============================================================================
int Con_ToggleConsole_f();
void Con_Dump_f();
void Con_RunConsole();
int Con_PageUp();
int Con_PageDown();
int Con_Top();
int Con_Bottom();
int Con_ClearNotify();
void Con_ClearMiniConsole();
void Con_ClearSubtitles();
int Con_CheckResize();
void Con_UpdateNotifyLine(print_msg_type_t type, int bLineFeed,
                          int duration, int flags);
void Con_Linefeed(print_msg_type_t type, int duration, int flags);
unsigned char Con_Clear_f();
void Con_Close();
void Con_Init();

// extern helpers (defined elsewhere in cl.o / core)
void Field_Clear(field_t* edit);
void Field_Draw(field_t* edit, int x, int y, int showCursor);
void Field_AdjustScroll(field_t* edit);
unsigned char ColorIndex(unsigned char c);
void CL_LookupColor(unsigned char c, float* color);
