// ============================================================================
// tr_text.cpp - render.o text paint with cursor (tr_font.cpp)
// Types and bodies verified against IDA (codmp_xboxr.xbe).
// ============================================================================

#include "core/math_types.h"
#include "ngl/nglFont.h"
#include "ngl/ngl_dx_quad.h"

#include <string.h>

struct nglTexture;
struct nglFont;

// CtrlIcon (core.o; full layout in core_systems.h)
class CtrlIcon {
public:
    char mScratchBuffer[2048];  // +0x00
    bool ContainsIconTag(const char* text);  // ?ContainsIconTag@CtrlIcon@@QAE_NPBD@Z
    bool ExtractIconTag(const char* text, char* preTagString,
                        char** postTagString, char** tagString);  // ?ExtractIconTag@CtrlIcon@@QAE_NPBDPADPAPAD2@Z
    static CtrlIcon* sInst;  // ?sInst@CtrlIcon@@2PAV1@A @ 0xF00EA8
};
CtrlIcon* CtrlIcon::sInst;

// refimport_t (UI_GetFontInfo +0x84)
struct refimport_t {
    uint8_t _pad[0x84];
    nglFont* (*UI_GetFontInfo)(int font, float scale);  // +0x84
};
extern refimport_t ri;  // ?ri@@3Urefimport_t@@A @ 0xF741E8
extern float sGlobalFontScale;  // ?sGlobalFontScale@@3MA @ 0xDFA444

nglTexture* GetTextureData(const char* name, int image_type,
                           const char* fromPak);  // ?GetTextureData@@YAPAUnglTexture@@PBDH0@Z
void R_Text_PaintConsoleIcon(float x, float y, float w, float h,
                             nglTexture* tex);  // ?R_Text_PaintConsoleIcon@@YAXMMMMPAUnglTexture@@@Z

// color tables (render.o data; const float[4])
extern const float colorBlack[4];
extern const float colorRed[4];
extern const float colorGreen[4];
extern const float colorYellow[4];
extern const float colorBlue[4];
extern const float colorCyan[4];
extern const float colorMagenta[4];
extern const float colorWhite[4];
extern const float colorOrange[4];
extern const float colorLtGreen[4];
extern const float colorLtOrange[4];

// IDA render.o data at 0xD0154C, 0xD0157C, 0xD0161C, 0xD0165C, 0xD0166C.
const float colorBlack[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
const float colorLtGreen[4] = { 0.0f, 0.7f, 0.0f, 1.0f };
const float colorWhite[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
const float colorOrange[4] = { 1.0f, 0.7f, 0.0f, 1.0f };
const float colorLtOrange[4] = { 0.75f, 0.525f, 0.0f, 1.0f };

// ============================================================================
// RE_Text_PaintWithCursor - ea: 0x006C6070
// ============================================================================
void RE_Text_PaintWithCursor(float x, float y, int font, float scale,
                             const float* const color, const char* text,
                             int cursorPos, char cursor, float depth,
                             int limit, int style)
{
    (void)cursorPos; (void)cursor; (void)depth; (void)limit; (void)style;

    const char* p_texta = text;
    const float* v8;
    if (text != nullptr)
    {
        if (*text == 94 && (text[1] - 48) < 10 && (text[1] - 48) > 0)
        {
            char buf[2];
            strcpy(buf, text);
            const float* colors[11];
            colors[0] = colorBlack;
            colors[1] = colorRed;
            colors[2] = colorGreen;
            colors[3] = colorYellow;
            colors[4] = colorBlue;
            colors[5] = colorCyan;
            colors[6] = colorMagenta;
            colors[7] = colorWhite;
            colors[8] = colorOrange;
            colors[9] = colorLtGreen;
            colors[10] = colorLtOrange;
            v8 = colors[text[1] - 48];
            p_texta = buf + 2;  // strip leading "^<digit>" color tag
            char* v9 = &buf[strlen(buf)];
            if (*v9 == 94 && v9[1] == 55)
                *v9 = 0;
        }
        else
        {
            v8 = color;
        }
    }
    else
    {
        v8 = color;
    }

    unsigned int v10 = (unsigned int)(v8[2] * 255.0f)
        | (((unsigned int)(v8[1] * 255.0f)
            | (((unsigned int)(v8[0] * 255.0f) | ((unsigned int)(v8[3] * 255.0f) << 8)) << 8)) << 8);
    nglFont* v11 = ri.UI_GetFontInfo(font, sGlobalFontScale * scale);
    if (v11 != nullptr)
    {
        bool hasIcon = !CtrlIcon::sInst->ContainsIconTag(p_texta);
        float iconscale = sGlobalFontScale * scale;
        if (hasIcon)
        {
            unsigned int w;
            unsigned int h;
            nglGetStringDimensions(v11, p_texta, &w, &h, iconscale, iconscale);
            float v19 = y - (float)h;
            nglListAddString(v11, p_texta, x, v19, 0.0f, v10,
                             sGlobalFontScale * scale, sGlobalFontScale * scale);
        }
        else
        {
            unsigned int w;
            unsigned int h;
            nglGetStringDimensions(v11, " ", &w, &h, iconscale, iconscale);
            char* postTagString = const_cast<char*>(p_texta);
            char buf[2];
            char* tagString = nullptr;
            if (CtrlIcon::sInst->ExtractIconTag(p_texta, buf, &postTagString, &tagString))
            {
                do
                {
                    if (buf[0] != 0)
                    {
                        nglGetStringDimensions(v11, buf, &w, &h,
                                               sGlobalFontScale * scale,
                                               sGlobalFontScale * scale);
                        float v20 = y - (float)h;
                        nglListAddString(v11, buf, x, v20, 0.0f, v10,
                                         sGlobalFontScale * scale,
                                         sGlobalFontScale * scale);
                        x = (float)w + x;
                    }
                    if (tagString != nullptr)
                    {
                        nglTexture* TextureData = GetTextureData(tagString, 0, "mp_frontEnd");
                        if (TextureData != nullptr)
                        {
                            float v28 = (float)TextureData->Height;
                            double v15 = (double)h;
                            if ((int)h < 0)
                                v15 = v15 + 4294967300.0;
                            float iconscaled = (float)(v15 / (double)v28);
                            float v29 = v28 * iconscaled;
                            float v23 = (float)TextureData->Width * iconscaled;
                            R_Text_PaintConsoleIcon(x, y - v29, v23, v29, TextureData);
                            double v18 = (double)((float)TextureData->Width * iconscaled);
                            x = (float)v18 + x;
                        }
                        else
                        {
                            nglGetStringDimensions(v11, tagString, &w, &h,
                                                   sGlobalFontScale * scale,
                                                   sGlobalFontScale * scale);
                            float v21 = y - (float)h;
                            nglListAddString(v11, tagString, x, v21, 0.0f, v10,
                                             sGlobalFontScale * scale,
                                             sGlobalFontScale * scale);
                            x = (float)w + x;
                        }
                    }
                } while (CtrlIcon::sInst->ExtractIconTag(postTagString, buf,
                                                          &postTagString, &tagString));
            }
            if (postTagString != nullptr)
            {
                nglGetStringDimensions(v11, postTagString, &w, &h,
                                       sGlobalFontScale * scale,
                                       sGlobalFontScale * scale);
                float v22 = y - (float)h;
                nglListAddString(v11, postTagString, x, v22, 0.0f, v10,
                                 sGlobalFontScale * scale,
                                 sGlobalFontScale * scale);
            }
        }
    }
}
