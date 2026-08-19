// ============================================================================
// ngl_font.cpp - NGL font resource directory + string rendering helpers.
// Source: src/ngl_font.cpp (ngl_xboxr)
// Verified against IDA (ngl_xboxr:ngl_font.o). Data globals here match the
// map attribution: nglFontDirectory (0x14d2a88), nglFontBuffer (0x10e3180).
// nglBuildScene/nglListWork/nglListWorkPos/nglListWorkSize/nglLastListAllocWarnFrame
// belong to ngl_scene.o and live here only until ngl_scene ports.
// ============================================================================

#include "core/tlSkipList.h"
#include "core/tlFixedString.h"
#include "ngl/nglFont.h"
#include "ngl/nglScene.h"
#include "filesystem/apk.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <new>

// ============================================================================
// Cross-object externs
// ============================================================================
extern void* nglGetResource(const tlFixedString& FileName, unsigned int FourCC);
extern bool _tlAssert(const char* file, int line, const char* expr, const char* desc);
extern void* nglListAlloc(unsigned int Bytes, unsigned int Alignment);
extern void nglListAddNode_Translucent(class nglRenderNode* Node);
extern void tlMemFree(void* Ptr);
extern void* tlMemAlloc(unsigned int Size, unsigned int Align, unsigned int Flags);

// ============================================================================
// Globals (data)
// ============================================================================
tlSkipList<nglFont, tlFixedString> nglFontDirectory;
char nglFontBuffer[0x400];
nglFont* nglSysFont = NULL;

// Skip-list key accessor (free function, used by tlSkipList<nglFont>).
const tlFixedString* GetKey(const nglFont* f) {
    return f->FileName;
}

// ============================================================================
// nglGetFont - ea: 0x842400
// ============================================================================
nglFont* nglGetFont(const tlFixedString& FileName) {
    return (nglFont*)nglGetResource(FileName, 0x544E4F46);  // 'FONT'
}

// ============================================================================
// nglFontParseToken (uint) - ea: 0x842420
// ============================================================================
void nglFontParseToken(unsigned char*& Text, unsigned int* Token) {
    if (*Text != '[' && _tlAssert("src/ngl_font.cpp", 31, "*Text == '['",
                                   "Invalid character found in Token.  Should be '['.\n"))
        __debugbreak();
    unsigned char* v4 = Text + 1;
    Text = v4;
    *Token = (unsigned int)strtoul((const char*)v4, (char**)&Text, 16);
    if (*Text == ']') {
        Text += 1;
    } else {
        if (_tlAssert("src/ngl_font.cpp", 33, "*Text == ']'",
                      "Invalid character found in Token.  Should be ']'.\n")) {
            __debugbreak();
            Text += 1;
            return;
        }
        Text += 1;
    }
}

// ============================================================================
// nglFontParseToken (float) - ea: 0x8424A0
// ============================================================================
void nglFontParseToken(unsigned char*& Text, float* Token) {
    if (*Text != '[' && _tlAssert("src/ngl_font.cpp", 39, "*Text == '['",
                                   "Invalid character found in Token.  Should be '['.\n"))
        __debugbreak();
    unsigned char* v4 = Text + 1;
    Text = v4;
    *Token = (float)strtod((const char*)v4, (char**)&Text);
    if (*Text == ']') {
        Text += 1;
    } else {
        if (_tlAssert("src/ngl_font.cpp", 41, "*Text == ']'",
                      "Invalid character found in Token.  Should be ']'.\n")) {
            __debugbreak();
            Text += 1;
            return;
        }
        Text += 1;
    }
}

// ============================================================================
// nglFontParseToken (float, float) - ea: 0x842520
// ============================================================================
void nglFontParseToken(unsigned char*& Text, float* TokenA, float* TokenB) {
    if (*Text != '[' && _tlAssert("src/ngl_font.cpp", 47, "*Text == '['",
                                   "Invalid character found in Token.  Should be '['.\n"))
        __debugbreak();
    unsigned char* v5 = Text + 1;
    Text = v5;
    *TokenA = (float)strtod((const char*)v5, (char**)&Text);
    if (*Text != ',' && _tlAssert("src/ngl_font.cpp", 49, "*Text == ','",
                                   "Invalid character found in Token.  Should be ','.\n"))
        __debugbreak();
    unsigned char* v6 = Text + 1;
    Text = v6;
    *TokenB = (float)strtod((const char*)v6, (char**)&Text);
    if (*Text == ']') {
        Text += 1;
    } else {
        if (_tlAssert("src/ngl_font.cpp", 51, "*Text == ']'",
                      "Invalid character found in Token.  Should be ']'.\n")) {
            __debugbreak();
            Text += 1;
            return;
        }
        Text += 1;
    }
}

// ============================================================================
// nglSetFontBlend - ea: 0x8425E0
// ============================================================================
void nglSetFontBlend(nglFont* Font, unsigned int BlendMode) {
    Font->BlendMode = BlendMode;
}

// ============================================================================
// nglSetFontMapFlags - ea: 0x8425F0
// ============================================================================
void nglSetFontMapFlags(nglFont* Font, unsigned int MapFlags) {
    Font->MapFlags = MapFlags;
}

// ============================================================================
// nglGetFontMapFlags - ea: 0x842600
// ============================================================================
unsigned int nglGetFontMapFlags(nglFont* Font) {
    return Font->MapFlags;
}

// ============================================================================
// nglBuildStringList - ea: 0x842610
// Builds a linked list of nglStringSection chunks from an inline-marked string.
// Control chars: 1=[color], 2=[scale], 3=[scaleX,scaleY], 9=tab, 10=newline,
// 32=space. Color is byte-rotated (Color>>8)|(Color<<24).
// ============================================================================
nglStringSection* nglBuildStringList(nglFont* Font, float x, float y, float ScaleX,
                                     float ScaleY, unsigned int Color, unsigned char* Text) {
    float Height = (float)Font->Header.CellHeight;
    float MaxScaleY = ScaleY;
    float Left = x;
    unsigned int v10 = Color;

    nglStringSection* v8 = (nglStringSection*)nglListAlloc(0x20u, 0x10u);
    v8->Next = NULL;
    nglStringSection* Head = v8;

    if (*Text == 0) {
        v8->Next = NULL;
        return v8->Next;
    }

    unsigned char v9 = *Text;
    nglStringSection* result = NULL;

process_char:
    {
        unsigned int v11 = v9 - 1;
        ++Text;
        while (1) {
            switch (v11) {
            case 0:  // [color]
                nglFontParseToken(Text, &Color);
                v10 = (Color >> 8) | (Color << 24);
                Color = v10;
                break;
            case 1:  // [scale]
                nglFontParseToken(Text, &ScaleX);
                ScaleY = ScaleX;
                if (ScaleX > MaxScaleY)
                    MaxScaleY = ScaleX;
                break;
            case 2:  // [scaleX,scaleY]
                nglFontParseToken(Text, &ScaleX, &ScaleY);
                if (ScaleY > MaxScaleY)
                    MaxScaleY = ScaleY;
                break;
            case 8:  // tab
            {
                int v13 = Font->Header.NumGlyphs - 1;
                if (97 - Font->Header.FirstGlyph >= 0) {
                    if (97 - Font->Header.FirstGlyph <= v13)
                        v13 = 97 - Font->Header.FirstGlyph;
                } else {
                    v13 = 0;
                }
                x = Font->GlyphInfo[v13].CellWidth * ScaleX * 4.0f + x;
                break;
            }
            case 9:  // newline
                x = Left;
                y = MaxScaleY * Height + y;
                MaxScaleY = ScaleY;
                break;
            case 0x1F:  // space
            {
                int v16 = Font->Header.NumGlyphs - 1;
                if (97 - Font->Header.FirstGlyph >= 0) {
                    if (97 - Font->Header.FirstGlyph <= v16)
                        v16 = 97 - Font->Header.FirstGlyph;
                } else {
                    v16 = 0;
                }
                x = Font->GlyphInfo[v16].CellWidth * ScaleX + x;
                break;
            }
            default:
                goto glyph_section;
            }

            v11 = *Text++ - 1;
            if (v11 > 0x1F)
                goto glyph_section;
        }
    }

glyph_section:
    {
        --Text;
        nglStringSection* v17 = (nglStringSection*)nglListAlloc(0x20u, 0x10u);
        v8->Next = v17;
        v8 = v17;
        unsigned char* v18 = Text;
        v8->x = x;
        v8->y = y;
        v8->ScaleX = ScaleX;
        v8->Text = v18;
        v8->ScaleY = ScaleY;
        v8->Color = v10;

        unsigned char* v20 = Text;
        while (1) {
            unsigned int v21 = *v20++;
            Text = v20;
            switch (v21) {
            case 0:
            case 1:
            case 2:
            case 3:
            case 9:
            case 10:
            case 32:
                Text = v20 - 1;
                v8->Length = (unsigned int)(v20 - 1 - v8->Text);
                v9 = *Text;
                if (*Text != 0) {
                    goto process_char;
                }
                v8->Next = NULL;
                result = Head->Next;
                goto done;
            default:
            {
                int v22 = Font->Header.NumGlyphs - 1;
                int v23 = v21 - Font->Header.FirstGlyph;
                if (v23 >= 0) {
                    if (v23 <= v22)
                        v22 = v23;
                } else {
                    v22 = 0;
                }
                x = Font->GlyphInfo[v22].CellWidth * ScaleX + x;
                break;
            }
            }
        }
    }

done:
    return result;
}

// ============================================================================
// nglGetStringDimensions - ea: 0x842910
// ============================================================================
void nglGetStringDimensions(nglFont* Font, const char* _Text, unsigned int* Width,
                            unsigned int* Height, float ScaleX, float ScaleY) {
    if (_Text && *_Text && Font && Font->Texture) {
        unsigned char* Text = (unsigned char*)_Text;
        unsigned char v7 = 0;
        float MaxScaleY = ScaleY;
        float w = 0.0f;
        float x = 0.0f;
        float y = 0.0f;
        unsigned char PrevCharacter = 0;
        unsigned int Color = 0;

        while (1) {
            unsigned char v9 = *Text;
            Text = Text + 1;
            switch (v9) {
            case 1:
                nglFontParseToken(Text, &Color);
                break;
            case 2:
                nglFontParseToken(Text, &ScaleX);
                ScaleY = ScaleX;
                if (ScaleX > MaxScaleY)
                    MaxScaleY = ScaleX;
                break;
            case 3:
                nglFontParseToken(Text, &ScaleX, &ScaleY);
                if (ScaleY > MaxScaleY)
                    MaxScaleY = ScaleY;
                break;
            case 9:  // tab = 4 * space cell
                Color = Font->GetCellWidth(0x20u);
                v7 = 32;
                x = (float)Color * ScaleX * 4.0f + x;
                PrevCharacter = v7;
                break;
            case 10:  // newline
                if (v7) {
                    const nglGlyphInfo* gi = &Font->GetGlyphInfo(PrevCharacter);
                    unsigned int cw = Font->GetCellWidth(PrevCharacter);
                    x = (float)((gi->GlyphSize[0] + gi->GlyphOrigin[0]) - (int)cw) * ScaleX + x;
                }
                if (x > w)
                    w = x;
                x = 0.0f;
                y = (float)Font->Header.CellHeight * MaxScaleY + y;
                MaxScaleY = ScaleY;
                v7 = 0;
                PrevCharacter = v7;
                break;
            case 0x20:  // space uses 'a' cell width
                Color = Font->GetCellWidth(0x61u);
                v7 = 97;
                x = (float)Color * ScaleX + x;
                PrevCharacter = v7;
                break;
            default:
            {
                int v16 = v9 - Font->Header.FirstGlyph;
                if (v16 >= 0) {
                    if (v16 > Font->Header.NumGlyphs - 1)
                        v16 = Font->Header.NumGlyphs - 1;
                } else {
                    v16 = 0;
                }
                Color = Font->GlyphInfo[v16].CellWidth;
                v7 = v9;
                x = (float)Color * ScaleX + x;
                PrevCharacter = v7;
                break;
            }
            }
            if (*Text)
                continue;
            if (v7) {
                const nglGlyphInfo* gi = &Font->GetGlyphInfo(PrevCharacter);
                unsigned int cw = Font->GetCellWidth(PrevCharacter);
                x = (float)((gi->GlyphSize[0] + gi->GlyphOrigin[0]) - (int)cw) * ScaleX + x;
            }
            break;
        }

        if (Width) {
            if (x <= w)
                x = w;
            *Width = (unsigned int)x;
        }
        if (Height)
            *Height = (unsigned int)((float)Font->Header.CellHeight * MaxScaleY + y);
    } else {
        if (Width)
            *Width = 0;
        if (Height)
            *Height = 0;
    }
}

// ============================================================================
// nglListAddString - ea: 0x842BF0
// ============================================================================
void nglListAddString(nglFont* Font, const char* Text, float x, float y, float z,
                      unsigned int Color, float ScaleX, float ScaleY) {
    nglValidateMatrices(nglBuildScene);
    if (Text && *Text && Font && Font->Texture) {
        nglStringNode* v8 = (nglStringNode*)nglListAlloc(0x30u, 0x10u);
        if (v8 != NULL) {
            new (v8) nglStringNode();
            size_t v9 = strlen(Text);
            char* v10 = (char*)nglListAlloc((unsigned int)v9 + 1, 0x10u);
            v8->Text = (unsigned char*)v10;
            memcpy(v10, Text, v9 + 1);
            unsigned char* v11 = v8->Text;
            v8->Color = Color;
            v8->x = x;
            v8->y = y;
            v8->ScaleX = ScaleX;
            v8->Font = Font;
            v8->z = z;
            v8->ScaleY = ScaleY;
            v8->SortDist = z;
            v8->Section = nglBuildStringList(Font, x, y, ScaleX, ScaleY, Color, v11);
            nglListAddNode_Translucent(v8);
        }
    }
}

// ============================================================================
// nglGetStringDimensions (varargs) - ea: 0x842CF0 / 0x842D30
// ============================================================================
void nglGetStringDimensions(nglFont* Font, unsigned int* Width, unsigned int* Height,
                            const char* Fmt, ...) {
    va_list ap;
    va_start(ap, Fmt);
    vsprintf(nglFontBuffer, Fmt, ap);
    va_end(ap);
    nglGetStringDimensions(Font, nglFontBuffer, Width, Height, 1.0f, 1.0f);
}

void nglGetStringDimensions(nglFont* Font, unsigned int* Width, unsigned int* Height,
                            float ScaleX, float ScaleY, const char* Fmt, ...) {
    va_list ap;
    va_start(ap, Fmt);
    vsprintf(nglFontBuffer, Fmt, ap);
    va_end(ap);
    nglGetStringDimensions(Font, nglFontBuffer, Width, Height, ScaleX, ScaleY);
}

// ============================================================================
// nglAPKFontLoadCallback - ea: 0x842D70
// ============================================================================
void nglAPKFontLoadCallback(apk::apkFile* File, apk::apkFileEntry* Entry, void* UserData) {
    (void)UserData;
    tlFixedString name("image");
    int SectionIndex = File->GetSectionIndex(name);
    nglFont* Data = (nglFont*)Entry->GetData(File, SectionIndex, true);

    int* p_Width = &Data->Texture->Width;
    Data->MapFlags = 1;
    Data->BlendMode = 1691321856;
    float v5 = 1.0f / (float)*p_Width;
    int v6 = 0;
    float v7 = 1.0f / (float)p_Width[1];
    if (Data->Header.NumGlyphs > 0) {
        int v8 = 0;
        do {
            Data->TexCoords[v8] = v5 * Data->TexCoords[v8];
            Data->TexCoords[v8 + 1] = v7 * Data->TexCoords[v8 + 1];
            Data->TexCoords[v8 + 2] = v5 * Data->TexCoords[v8 + 2];
            Data->TexCoords[v8 + 3] = v7 * Data->TexCoords[v8 + 3];
            ++v6;
            v8 += 4;
        } while (v6 < Data->Header.NumGlyphs);
    }
    nglFontDirectory.Add(Data);
}

// ============================================================================
// nglAPKFontDeleteCallback - ea: 0x842E40
// ============================================================================
void nglAPKFontDeleteCallback(apk::apkFile* File, apk::apkFileEntry* Entry, void* UserData) {
    (void)UserData;
    tlFixedString name("image");
    int SectionIndex = File->GetSectionIndex(name);
    const nglFont* Data = (const nglFont*)Entry->GetData(File, SectionIndex, true);
    nglFontDirectory.Del(Data);
}

// ============================================================================
// nglFontInit - ea: 0x842E80
// ============================================================================
void nglFontInit() {
    nglFontDirectory.Level = 0;
    nglFontDirectory.Head =
        (tlSkipList<nglFont, tlFixedString>::Instance*)tlMemAlloc(0x44u, 8u, 0x1000000);
    for (int i = 0; i <= 15; ++i)
        nglFontDirectory.Head->Forward[i] = NULL;
    apk::apkRegisterFileType(0x544E4F46, 1u, nglAPKFontLoadCallback,
                             nglAPKFontDeleteCallback, NULL);
}

// ============================================================================
// nglListAddString (varargs) - ea: 0x842EE0 / 0x842F20 / 0x842F70 / 0x842FB0
// ============================================================================
void nglListAddString(nglFont* Font, float x, float y, float z, const char* Fmt, ...) {
    va_list ap;
    va_start(ap, Fmt);
    vsprintf(nglFontBuffer, Fmt, ap);
    va_end(ap);
    nglListAddString(Font, nglFontBuffer, x, y, z, 0xFFFFFFFF, 1.0f, 1.0f);
}

void nglListAddString(nglFont* Font, float x, float y, float z, unsigned int Color,
                      const char* Fmt, ...) {
    va_list ap;
    va_start(ap, Fmt);
    vsprintf(nglFontBuffer, Fmt, ap);
    va_end(ap);
    nglListAddString(Font, nglFontBuffer, x, y, z, Color, 1.0f, 1.0f);
}

void nglListAddString(nglFont* Font, float x, float y, float z, float ScaleX, float ScaleY,
                      const char* Fmt, ...) {
    va_list ap;
    va_start(ap, Fmt);
    vsprintf(nglFontBuffer, Fmt, ap);
    va_end(ap);
    nglListAddString(Font, nglFontBuffer, x, y, z, 0xFFFFFFFF, ScaleX, ScaleY);
}

void nglListAddString(nglFont* Font, float x, float y, float z, unsigned int Color,
                      float ScaleX, float ScaleY, const char* Fmt, ...) {
    va_list ap;
    va_start(ap, Fmt);
    vsprintf(nglFontBuffer, Fmt, ap);
    va_end(ap);
    nglListAddString(Font, nglFontBuffer, x, y, z, Color, ScaleX, ScaleY);
}
