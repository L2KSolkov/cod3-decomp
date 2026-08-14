// ============================================================================
// tr_font.cpp - render.o console text width (tr_font.cpp)
// Types and bodies verified against IDA (codmp_xboxr.xbe).
// ============================================================================

#include "core/math_types.h"
#include "ngl/ngl_dx_quad.h"

#include <string.h>

// AeAssert (game.o)
namespace AeAssert {
enum ECoderId { COD3 = 0 };
extern ECoderId gCurrentAuthor;
extern const char* gCurrentFile;
extern int gCurrentLine;
extern const char* gCurrentExpr;
bool IsIgnored();
bool Assert(const char* fmtstring, ...);
bool Warning(const char* fmtstring, ...);
}

struct nglTexture;

static inline int HIBYTE(unsigned short v)
{
    return (v >> 8) & 0xFF;
}

unsigned char ColorIndex(unsigned char c);  // ?ColorIndex@@YAEE@Z (game.o)
int RE_Text_Width(const char* text, int font, float scale, float charWidth,
                  int limit);  // ?RE_Text_Width@@YAHPBDHMMH@Z (render.o 0x6C5B40)
nglTexture* GetTextureData(const char* name, int image_type,
                           const char* fromPak);

extern void RE_Text_PaintWithCursor(float x, float y, int font, float scale,
                                    const float* const color,
                                    const char* text, int cursorPos,
                                    char cursor, float depth, int limit,
                                    int style);  // tr_text.cpp
extern void RE_SetColor(const float* rgba);  // r_stubs.cpp
extern nglScene* nglBuildScene;   // ?nglBuildScene@@3PAUnglScene@@A
extern int sCurColor;             // ?sCurColor@@3IA @ 0xF74290
extern void nglInitQuad(nglQuad* Quad);  // ngl.o
extern void nglSetQuadRect(nglQuad* Quad, float x1, float y1, float x2,
                           float y2);  // ngl.o
extern void nglSetQuadUV(nglQuad* Quad, float u1, float v1, float u2,
                         float v2);  // ngl.o
extern void nglSetQuadColor(nglQuad* Quad, unsigned int c);  // ngl.o
extern void nglSetQuadZ(nglQuad* Quad, float z);  // ngl.o
extern void nglListAddQuad(nglQuad* Quad);  // ngl.o

// refimport_t view (AdjustFrom640 +0x80)
struct refimport_t {
    uint8_t _pad[0x80];
    void (*AdjustFrom640)(float* x, float* y, float* w, float* h);  // +0x80
};
extern refimport_t ri;  // ?ri@@3Urefimport_t@@A @ 0xF741E8

// static buffer (render.o @ 0xF78370)
static char szText[256];

// ============================================================================
// R_Text_GetConsoleString (file-static; tr_font.cpp) - ea: 0x006C0060
// ============================================================================
static int R_Text_GetConsoleString(const short* psString, int* piLimit,
                                   const char** ppszOutString,
                                   const short** psStringEnd,
                                   float* vColor)
{
    int* v5 = piLimit;
    *ppszOutString = szText;
    if (*piLimit > 255)
        *piLimit = 255;
    int v6 = 0;
    int v7 = 0;
    szText[0] = 0;
    unsigned char v8 = ColorIndex(0x37u);
    int iMarkedEnd = -1;
    int bIconFound = 0;
    const short* v9 = psString;
    if (*piLimit > 0)
    {
        while (1)
        {
            v9 = psString;
            int v10 = HIBYTE(psString[v6]);
            if (v10 == v8)
                goto label24;
            switch (v10)
            {
            case 13:
            case 16:
            case 17:
            case 18:
                v5 = piLimit;
                *psStringEnd = &psString[v6];
                iMarkedEnd = -1;
                bIconFound = 1;
                goto label33;
            case 10:
                if (vColor != nullptr)
                    *vColor = (float)psString[v6] * 0.0039215689f;
                break;
            case 11:
                if (vColor != nullptr)
                    vColor[1] = (float)psString[v6] * 0.0039215689f;
                break;
            case 12:
                if (vColor != nullptr)
                    vColor[2] = (float)psString[v6] * 0.0039215689f;
                break;
            default:
                if (v10 < ColorIndex(0x30u) || v10 > ColorIndex(0x39u))
                {
                    AeAssert::gCurrentAuthor = AeAssert::COD3;
                    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\tr_font.cpp";
                    AeAssert::gCurrentLine = 525;
                    AeAssert::gCurrentExpr =
                        "(iInfoValue >= ColorIndex(((unsigned char)'0'))) && (iInfoValue <= ColorIndex(((unsigned char)'9')))";
                    if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
                        __debugbreak();
                }
                v9 = psString;
                szText[v7] = 94;
                v8 = (unsigned char)v10;
                int v11 = v7 + 1;
                szText[v11] = (char)(v10 + 48);
                v7 = v11 + 1;
label24:
                {
                    char v12 = (char)v9[v6];
                    szText[v7] = v12;
                    if (v12 == 32)
                    {
                        if (iMarkedEnd == -1)
                            iMarkedEnd = v7;
                    }
                    else
                    {
                        iMarkedEnd = -1;
                    }
                    ++v7;
                }
                break;
            }
            if (++v6 >= *piLimit)
            {
                v5 = piLimit;
                goto label33;
            }
        }
    }
    v9 = psString;
label33:
    szText[v7] = 0;
    if (iMarkedEnd >= 0)
        szText[iMarkedEnd] = 0;
    if (v6 == *v5)
        *psStringEnd = nullptr;
    else
        *psStringEnd = &v9[v6];
    *v5 -= v6;
    return bIconFound;
}

// ============================================================================
// R_DrawStrlen (file-static; tr_font.cpp) - ea: 0x006C0280
// ============================================================================
static int R_DrawStrlen(const char* pszString)
{
    int result = 0;
    if (*pszString != 0)
    {
        do
        {
            if (*pszString == 94 && pszString[1] != 0 && pszString[1] != 94
                && pszString[1] >= 48 && pszString[1] <= 57)
                pszString += 2;
            else
                ++result;
        } while (*++pszString != 0);
    }
    return result;
}

// ============================================================================
// R_Text_GetConsoleIcon (file-static; tr_font.cpp) - ea: 0x006C5D10
// ============================================================================
static void R_Text_GetConsoleIcon(const short* psString,
                                  const short** psStringEnd,
                                  int* piLimit, float fFontScale,
                                  float* pfIconWidth, float* pfIconHeight,
                                  nglTexture** hIconShader, float* vColor)
{
    int v8 = psString[1];
    if (v8 != 13 && v8 != 16 && v8 != 17 && v8 != 18)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\tr_font.cpp";
        AeAssert::gCurrentLine = 566;
        AeAssert::gCurrentExpr =
            "(iInfoValue == 13) || (iInfoValue == 16) || (iInfoValue == 17) || (iInfoValue == 18)";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    *pfIconWidth = fFontScale * 24.0f;
    *pfIconHeight = fFontScale * 24.0f;
    char szShaderName[128];
    memset(szShaderName, 0, sizeof(szShaderName));
    *hIconShader = nullptr;
    int v9 = 0;
    if (*piLimit > 0)
    {
        while (1)
        {
            int v10 = HIBYTE(psString[v9]);
            int v11 = psString[v9];
            switch (v10)
            {
            case 13:
                if (vColor != nullptr)
                    *vColor = (float)v11 * 0.0039215689f;
                ++*psStringEnd;
                break;
            case 14:
                if (vColor != nullptr)
                    vColor[1] = (float)v11 * 0.0039215689f;
                ++*psStringEnd;
                break;
            case 15:
                if (vColor != nullptr)
                    vColor[2] = (float)v11 * 0.0039215689f;
                ++*psStringEnd;
                break;
            case 16:
                *pfIconWidth = ((float)v11 * 0.03125f) * *pfIconWidth;
                ++*psStringEnd;
                break;
            case 17:
                *pfIconHeight = ((float)v11 * 0.03125f) * *pfIconHeight;
                ++*psStringEnd;
                break;
            case 18:
            {
                char* v12 = szShaderName;
                int v13 = 0;
                do
                {
                    if (v11 == 0)
                        break;
                    if (v9 >= *piLimit - 1)
                        break;
                    *v12++ = (char)v11;
                    ++*psStringEnd;
                    v13 = psString[++v9];
                    v11 = v13;
                } while ((v13 & 0xFF00) == 0x1200);
                *hIconShader = GetTextureData(szShaderName, 5, "mp_frontEnd");
                *piLimit -= v9;
                return;
            }
            default:
                AeAssert::gCurrentAuthor = AeAssert::COD3;
                AeAssert::gCurrentFile = "c:\\cod\\code\\game\\tr_font.cpp";
                AeAssert::gCurrentLine = 629;
                AeAssert::gCurrentExpr = nullptr;
                if (!AeAssert::IsIgnored()
                    && AeAssert::Warning("Console icon info parsed without shader being specified"))
                    __debugbreak();
                break;
            }
            if (++v9 >= *piLimit)
                break;
        }
    }
    AeAssert::gCurrentAuthor = AeAssert::COD3;
    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\tr_font.cpp";
    AeAssert::gCurrentLine = 635;
    AeAssert::gCurrentExpr = nullptr;
    if (!AeAssert::IsIgnored()
        && AeAssert::Warning("Console icon info parsed without shader being specified"))
        __debugbreak();
}

// ============================================================================
// RE_Text_ConsoleWidth - ea: 0x006C5F80
// ============================================================================
int RE_Text_ConsoleWidth(const short* psString, int font, float scale,
                         float charWidth, int limit)
{
    const short* v5 = psString;
    float fWidth = 0.0f;
    if (psString == nullptr)
        return 0;
    float v6 = scale;
    do
    {
        const char* pszConvertedString = nullptr;
        int ConsoleString = R_Text_GetConsoleString(
            v5, &limit, &pszConvertedString, &psString, nullptr);
        if (pszConvertedString != nullptr && *pszConvertedString != 0)
        {
            if (charWidth == 0.0f)
            {
                int v10 = RE_Text_Width(pszConvertedString, font, v6, charWidth, limit);
                fWidth = (float)v10 + fWidth;
            }
            else
            {
                int v9 = R_DrawStrlen(pszConvertedString);
                fWidth = ((float)v9 * charWidth) + fWidth;
            }
        }
        if (ConsoleString != 0)
        {
            float fIconHeight;
            nglTexture* hIconShader;
            float fIconWidth;
            const short* psStringEnd = nullptr;
            R_Text_GetConsoleIcon(psString, &psStringEnd, &limit, v6,
                                  &fIconWidth, &fIconHeight, &hIconShader, nullptr);
            fWidth = fIconWidth + fWidth;
            psString = psStringEnd;
        }
        v5 = psString;
    } while (psString != nullptr);
    return (int)fWidth;
}

// ============================================================================
// RE_Text_ConsolePaint - ea: 0x006D2020
// ============================================================================
void RE_Text_ConsolePaint(float x, float y, int font, float scale,
                          const float* color, const short* psString,
                          float charWidth, int limit, int style)
{
    float vColor[4];
    vColor[0] = color[0];
    vColor[1] = color[1];
    vColor[2] = color[2];
    vColor[3] = color[3];
    float fXOfs = 0.0f;
    int v11 = 0;
    if (psString == nullptr)
        return;
    while (1)
    {
        const char* pszConvertedString = nullptr;
        int ConsoleString = R_Text_GetConsoleString(
            psString, &limit, &pszConvertedString, &psString, vColor);
        const char* v10 = pszConvertedString;
        v11 = ConsoleString;
        if (pszConvertedString == nullptr || *pszConvertedString == 0)
            break;
        RE_Text_PaintWithCursor(fXOfs + x, y, font, scale, vColor,
                                pszConvertedString, -1, 0, charWidth, 0,
                                style);
        if (v11 != 0)
        {
            fXOfs = RE_Text_Width(v10, font, scale, charWidth, limit) + fXOfs;
            goto LABEL_7;
        }
    LABEL_11:
        if (psString == nullptr)
            return;
    }
LABEL_7:
    if (v11 != 0)
    {
        float fIconHeight;
        float fIconWidth;
        nglTexture* hIconShader = nullptr;
        const short* psStringEnd = nullptr;
        R_Text_GetConsoleIcon(psString, &psStringEnd, &limit, scale,
                              &fIconWidth, &fIconHeight, &hIconShader,
                              vColor);
        RE_SetColor(vColor);
        float v21 = fIconWidth;
        float v20 = fIconHeight;
        float v23 = y - (((scale * 19.200001f) + fIconHeight) * 0.5f);
        float v25 = fXOfs + x;
        ri.AdjustFrom640(&v25, &v23, &v21, &v20);
        float x1 = v25;
        float y1 = v23;
        float w = v21;
        float h = v20;
        if (nglBuildScene != nullptr)
        {
            nglQuad Quad;
            nglInitQuad(&Quad);
            nglSetQuadRect(&Quad, x1, y1, x1 + w, y1 + h);
            nglSetQuadUV(&Quad, 0.0f, 0.0f, 1.0f, 1.0f);
            nglSetQuadColor(&Quad, (unsigned int)sCurColor);
            nglSetQuadZ(&Quad, 0.0f);
            Quad.Tex = hIconShader;
            nglListAddQuad(&Quad);
        }
        fXOfs = fIconWidth + fXOfs;
    }
    goto LABEL_11;
}
