// ============================================================================
// ngl_debugdraw.cpp - CPU-side debug drawing onto the front buffer (11 funcs).
// Source: src/ngl_debugdraw.cpp (ngl_xboxr)
// Verified against IDA (ngl_xboxr:ngl_debugdraw.o).
// ============================================================================

#include "ngl/nglDebug.h"
#include "ngl/ngl_dx_quad.h"
#include "ngl/ngl_dx_gpu.h"
#include "ngl/nglFont.h"
#include "ngl/nglScene.h"
#include "d3d8.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

// ============================================================================
// Cross-object externs
// ============================================================================
extern nglTexture nglFrontBufferTex;                    // ngl_dx_tex_create.o
extern void nglInit();                                  // ngl_internal.o
extern bool nglIsInitialized();                         // ngl_internal.o
extern unsigned int nglGetDisplayMode();                // ngl_internal.o
extern void nglSetDisplayMode(unsigned int* Modes, unsigned int ModeCount);  // ngl_dx_core.o
extern void ngliWaitForResource();                      // ngl_dx_core.o
extern void nglFontParseToken(unsigned char** Text, unsigned int* Token);
extern void nglFontParseToken(unsigned char** Text, float* Token);
extern void nglFontParseToken(unsigned char** Text, float* TokenA, float* TokenB);
extern bool _tlAssert(const char* file, int line, const char* expr, const char* desc);

// ============================================================================
// Data (ngl_debugdraw.o)
// ============================================================================
static bool nglDebugDrawActive = false;
static unsigned int nglDebugDrawScale = 1;
static int Width = 0;
static int Height = 0;
static void* Screen = NULL;
static int Pitch = 0;
static unsigned int Format = 0;

// 8x8 bitmap font (first 64 glyphs).
static const unsigned char FontTable[64 * 8] = {
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x10,0x00,0x00,0x00,0x08,0x10,0x09,
    0x02,0x03,0x0A,0x11,0x18,0x20,0x19,0x12,
    0x0B,0x04,0x05,0x0C,0x13,0x1A,0x21,0x28,
    0x30,0x29,0x22,0x1B,0x14,0x0D,0x06,0x07,
    0x0E,0x15,0x1C,0x23,0x2A,0x31,0x38,0x39,
    0x32,0x2B,0x24,0x1D,0x16,0x0F,0x17,0x1E,
    0x25,0x2C,0x33,0x3A,0x3B,0x34,0x2D,0x26,
};

// ============================================================================
// Plot helpers (file-static)
// ============================================================================
static void Plot32_8_8_8_16_8_0_(int X, int Y, unsigned int Color, unsigned int Size) {
    unsigned int Scale = nglDebugDrawScale;
    int x = X * (int)Scale;
    int y = Y * (int)Scale;
    unsigned int s = Size * Scale;
    unsigned char R = Color & 0xFF;
    unsigned char G = (Color >> 8) & 0xFF;
    unsigned char B = (Color >> 16) & 0xFF;
    unsigned int* dst = (unsigned int*)((char*)Screen + Pitch * y + x * 4);
    for (unsigned int i = 0; i < s; ++i) {
        unsigned int* row = dst;
        for (unsigned int j = 0; j < s; ++j)
            *row++ = R | (G << 8) | (B << 16);
        dst = (unsigned int*)((char*)dst + Pitch);
    }
}

static void Plot32_5_6_5_11_5_0_(int X, int Y, unsigned int Color, unsigned int Size) {
    unsigned int Scale = nglDebugDrawScale;
    int x = X * (int)Scale;
    int y = Y * (int)Scale;
    unsigned int s = Size * Scale;
    unsigned char R = Color & 0xFF;
    unsigned char G = (Color >> 8) & 0xFF;
    unsigned char B = (Color >> 16) & 0xFF;
    unsigned short v = (unsigned short)((R >> 3) | ((G >> 2) << 5) | ((B >> 3) << 11));
    unsigned short* dst = (unsigned short*)((char*)Screen + Pitch * y + x * 2);
    for (unsigned int i = 0; i < s; ++i) {
        unsigned short* row = dst;
        for (unsigned int j = 0; j < s; ++j)
            *row++ = v;
        dst = (unsigned short*)((char*)dst + Pitch);
    }
}

// ============================================================================
// nglDebugDrawBegin - ea: 0x850790
// ============================================================================
void nglDebugDrawBegin() {
    if (nglDebugDrawActive
        && _tlAssert("src/ngl_debugdraw.cpp", 191, "nglDebugDrawActive == false",
                     "Already in debug draw."))
        __debugbreak();
    nglDebugDrawActive = true;
    if (!nglIsInitialized())
        nglInit();
    if (nglGetDisplayMode() == 0) {
        unsigned int Modes[5] = { 1, 2, 3, 4, 5 };
        nglSetDisplayMode(Modes, 5);
        nglGetDisplayMode();
    }
    nglDebugDrawScale = (nglFrontBufferTex.Width > 0x280u) + 1;
    Width = nglFrontBufferTex.Width / (int)nglDebugDrawScale;
    Height = nglFrontBufferTex.Height / (int)nglDebugDrawScale;
    ngliWaitForResource();
    D3DLOCKED_RECT Rect;
    D3DTexture_LockRect((D3DTexture*)nglFrontBufferTex.Texture, 0, &Rect, NULL, 0x40u);
    _D3DSURFACE_DESC Desc;
    D3DTexture_GetLevelDesc(nglFrontBufferTex.Texture, 0, &Desc);
    Screen = Rect.pBits;
    Pitch = Rect.Pitch;
    Format = Desc.Format;
}

// ============================================================================
// nglDebugDrawEnd - ea: 0x850890
// ============================================================================
void nglDebugDrawEnd() {
    if (!nglDebugDrawActive
        && _tlAssert("src/ngl_debugdraw.cpp", 304, "nglDebugDrawActive == true",
                     "nglDebugDrawBegin was not called."))
        __debugbreak();
    nglDebugDrawActive = false;
    Height = 0;
    Width = 0;
    Screen = NULL;
    Pitch = 0;
    Format = 0;
}

// ============================================================================
// nglDebugDrawGetWidth / nglDebugDrawGetHeight
// ============================================================================
int nglDebugDrawGetWidth() {
    if (!nglDebugDrawActive
        && _tlAssert("src/ngl_debugdraw.cpp", 340, "nglDebugDrawActive == true",
                     "nglDebugDrawBegin was not called."))
        __debugbreak();
    return Width;
}

int nglDebugDrawGetHeight() {
    if (!nglDebugDrawActive
        && _tlAssert("src/ngl_debugdraw.cpp", 347, "nglDebugDrawActive == true",
                     "nglDebugDrawBegin was not called."))
        __debugbreak();
    return Height;
}

// ============================================================================
// nglDebugDrawPlot - ea: 0x850A70
// ============================================================================
void nglDebugDrawPlot(int X, int Y, unsigned int Color,
                      unsigned int Size) {
    if (!nglDebugDrawActive
        && _tlAssert("src/ngl_debugdraw.cpp", 145, "nglDebugDrawActive == true",
                     "nglDebugDrawBegin was not called."))
        __debugbreak();
    if (X >= 0 && Y >= 0 && X < Width && Y < Height
        && ((Color >> 24) == 0xFF || ((Y ^ X) & 1) == 0)) {
        switch (Format) {
        case 0x11:
        case 0x1C:
            Plot32_5_6_5_11_5_0_(X, Y, Color, Size);
            break;
        case 0x12:
        case 0x1E:
            Plot32_8_8_8_16_8_0_(X, Y, Color, Size);
            break;
        default:
            _tlAssert("src/ngl_debugdraw.cpp", 182, "false", "Unknown frontbuffer format.");
            break;
        }
    }
}

// ============================================================================
// nglDebugDrawHLine / nglDebugDrawVLine / nglDebugDrawRectFill / nglDebugDrawRect
// ============================================================================
void nglDebugDrawHLine(int X, int Y, int X2, unsigned int Color) {
    if (!nglDebugDrawActive
        && _tlAssert("src/ngl_debugdraw.cpp", 354, "nglDebugDrawActive == true",
                     "nglDebugDrawBegin was not called."))
        __debugbreak();
    for (int i = X; i < X2; ++i)
        nglDebugDrawPlot((unsigned int)i, (unsigned int)Y, Color, 1);
}

void nglDebugDrawVLine(int X, int Y, int Y2, unsigned int Color) {
    if (!nglDebugDrawActive
        && _tlAssert("src/ngl_debugdraw.cpp", 389, "nglDebugDrawActive == true",
                     "nglDebugDrawBegin was not called."))
        __debugbreak();
    for (int i = Y; i < Y2; ++i)
        nglDebugDrawPlot((unsigned int)X, (unsigned int)i, Color, 1);
}

void nglDebugDrawRectFill(int X, int Y, int X2, int Y2, unsigned int Color) {
    if (!nglDebugDrawActive
        && _tlAssert("src/ngl_debugdraw.cpp", 399, "nglDebugDrawActive == true",
                     "nglDebugDrawBegin was not called."))
        __debugbreak();
    for (int i = Y; i < Y2; ++i)
        nglDebugDrawHLine(X, i, X2, Color);
}

void nglDebugDrawRect(int X, int Y, int X2, int Y2, unsigned int Color) {
    if (!nglDebugDrawActive
        && _tlAssert("src/ngl_debugdraw.cpp", 409, "nglDebugDrawActive == true",
                     "nglDebugDrawBegin was not called."))
        __debugbreak();
    nglDebugDrawHLine(X, Y, X2 - 1, Color);
    nglDebugDrawVLine(X, Y + 1, Y2 - 1, Color);
    nglDebugDrawHLine(X, Y2 - 1, X2, Color);
    nglDebugDrawVLine(X2 - 1, Y, Y2 - 1, Color);
}

// ============================================================================
// nglDebugDrawTextInternal - ea: 0x850D00
// ============================================================================
static const char* aAbxy = "ABXY";

void nglDebugDrawTextInternal(int X, int Y, unsigned int Color, const char* Text, ...) {
    char Work[512];
    va_list ap;
    va_start(ap, Text);
    vsprintf(Work, Text, ap);
    va_end(ap);
    if (!nglDebugDrawActive
        && _tlAssert("src/ngl_debugdraw.cpp", 484, "nglDebugDrawActive == true",
                     "nglDebugDrawBegin was not called."))
        __debugbreak();
    int LineX = X;
    unsigned char* Ptr = (unsigned char*)Work;
    int Scale = 1;
    while (*Ptr != 0) {
        unsigned char ch = *Ptr;
        Ptr++;
        unsigned int Button = -1;
        unsigned int Token = -1;
        switch (ch) {
        case 1:  // color
            nglFontParseToken(&Ptr, &Color);
            Color = (Color >> 8) | (Color << 24);
            break;
        case 2:  // scale
            {
                float s;
                nglFontParseToken(&Ptr, &s);
                Scale = (int)s;
            }
            if (Scale < 1)
                Scale = 1;
            if (Scale > 10)
                Scale = 10;
            break;
        case 3:  // scale xy
            {
                float sx, sy;
                nglFontParseToken(&Ptr, &sx, &sy);
                Scale = (int)sx;
            }
            if (Scale < 1)
                Scale = 1;
            if (Scale > 10)
                Scale = 10;
            break;
        case 4:  // button
            nglFontParseToken(&Ptr, &Button);
            Token = Button;
            ch = 32;
            // fallthrough
        case 10:  // newline
            if (ch == 10) {
                Y += 8 * Scale;
                X = LineX;
                break;
            }
            // fallthrough
        default:
            if (ch >= 32 || Token != -1) {
                unsigned int TempColor = Color;
                unsigned char* Table;
                if (Token == -1) {
                    Table = (unsigned char*)(8 * ch + (size_t)FontTable);
                } else {
                    int v10 = aAbxy[Button];
                    switch (aAbxy[Button]) {
                    case 'A': TempColor = 0xFF4060C0u; break;
                    case 'B': TempColor = 0xFF3F40C0u; break;
                    case 'X': TempColor = 0xFF3F9F40u; break;
                    case 'Y': TempColor = 0xFFFF8000u; break;
                    default: break;
                    }
                    Table = (unsigned char*)(8 * v10 + (size_t)FontTable);
                }
                for (unsigned int row = 0; row < 8; ++row) {
                    int px = X;
                    unsigned char bits = Table[row];
                    for (unsigned int col = 0; col < 8; ++col) {
                        if (((128 >> col) & bits) != 0)
                            nglDebugDrawPlot((unsigned int)px, (unsigned int)Y,
                                             TempColor, (unsigned int)Scale);
                        px += Scale;
                    }
                    Y += Scale;
                }
                X += 8 * Scale;
            }
            break;
        }
    }
}

// ============================================================================
// nglDebugDrawText - ea: 0x850FB0
// ============================================================================
int nglDebugDrawText(int X, int Y, unsigned int Color, const char* Text, ...) {
    if (Text == NULL || *Text == 0)
        return 1;
    int v7 = (int)strlen(Text);
    int nLineCount = 0;
    int v5 = 0;
    char TempString[256];
    while (1) {
        memset(TempString, 0, sizeof(TempString));
        strncpy(TempString, &Text[v5], 0x44);
        char* v9 = strchr(TempString, 10);
        if (v9 != NULL) {
            v5 -= (int)(&TempString[67] - v9);
            *v9 = 0;
            v7 += (int)(&TempString[67] - v9);
        }
        nglDebugDrawTextInternal(X, Y, Color, TempString);
        int v10 = nLineCount + 1;
        v7 -= 68;
        Y += 8;
        v5 += 68;
        ++nLineCount;
        if (v7 <= 0)
            return v10;
    }
}
