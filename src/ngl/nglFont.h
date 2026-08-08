// ============================================================================
// nglFont - NGL font resource types + inline glyph accessors.
// Source: c:\cod\code\tl\ngl\include\nglFont.h
// Layouts from IDA local types (verified):
//   nglFontHeader    20 bytes  {Version, CellHeight, Ascent, FirstGlyph, NumGlyphs}
//   nglGlyphInfo     28 bytes  {TexOfs[2], GlyphSize[2], GlyphOrigin[2], CellWidth}
//   nglFont          56 bytes  (FileName, Texture, GlyphInfo, MapFlags, BlendMode,
//                               _BlendModePad, Header, TexCoords, System, Pad)
//   nglStringSection 32 bytes
//   nglStringNode    48 bytes  (12-byte nglRenderNode base; Text at +0xC etc.,
//                               verified against nglListAddString disasm)
// Inline COMDATs: GetGlyphInfo ea 0x5B2F10, GetCellWidth ea 0x843040,
// nglStringNode ctor/GetDesc/dtor ea 0x843080/0x843090/0x8430A0.
// ============================================================================

#ifndef COD3_NGL_NGL_FONT_H
#define COD3_NGL_NGL_FONT_H

#include "core/tlFixedString.h"
#include "ngl/nglTexture.h"
#include "ngl/nglRenderNode.h"

// ============================================================================
// nglFontHeader - 20 bytes
// ============================================================================
struct nglFontHeader {
    int Version;      // +0x00
    int CellHeight;   // +0x04
    int Ascent;       // +0x08
    int FirstGlyph;   // +0x0C
    int NumGlyphs;    // +0x10
};
static_assert(sizeof(nglFontHeader) == 0x14, "nglFontHeader size mismatch");

// ============================================================================
// nglGlyphInfo - 28 bytes
// ============================================================================
struct nglGlyphInfo {
    int TexOfs[2];      // +0x00
    int GlyphSize[2];   // +0x08
    int GlyphOrigin[2]; // +0x10
    int CellWidth;      // +0x18
};
static_assert(sizeof(nglGlyphInfo) == 0x1C, "nglGlyphInfo size mismatch");

// ============================================================================
// nglFont - 56 bytes
// ============================================================================
struct nglFont {
    tlFixedString* FileName;     // +0x00
    nglTexture*    Texture;      // +0x04
    nglGlyphInfo*  GlyphInfo;    // +0x08
    unsigned int   MapFlags;     // +0x0C
    unsigned int   BlendMode;    // +0x10
    unsigned int   _BlendModePad;// +0x14
    nglFontHeader  Header;       // +0x18
    float*         TexCoords;    // +0x2C
    bool           System;       // +0x30
    unsigned int   Pad;          // +0x34

    // ea: 0x5B2F10
    const nglGlyphInfo& GetGlyphInfo(unsigned char Character) {
        int FirstGlyph = Header.FirstGlyph;
        int v3 = Header.NumGlyphs - 1;
        if ((int)(Character - FirstGlyph) < 0)
            return GlyphInfo[0];
        if ((int)(Character - FirstGlyph) <= v3)
            v3 = Character - FirstGlyph;
        return GlyphInfo[v3];
    }

    // ea: 0x843040
    unsigned int GetCellWidth(unsigned char Character) {
        int FirstGlyph = Header.FirstGlyph;
        int v3 = Header.NumGlyphs - 1;
        if ((int)(Character - FirstGlyph) < 0)
            return GlyphInfo[0].CellWidth;
        if ((int)(Character - FirstGlyph) <= v3)
            v3 = Character - FirstGlyph;
        return GlyphInfo[v3].CellWidth;
    }
};
static_assert(sizeof(nglFont) == 0x38, "nglFont size mismatch");

// Skip-list key accessor (free function, defined in ngl_font.cpp)
const tlFixedString* GetKey(const nglFont* f);

// ngl_font.o (data, defined in ngl_font.cpp)
extern tlSkipList<nglFont, tlFixedString> nglFontDirectory;
extern char nglFontBuffer[0x400];

// Forward decl (full definition below)
struct nglStringSection;

// ngl_font.o (functions, defined in ngl_font.cpp)
nglFont* nglGetFont(const tlFixedString& FileName);
void nglFontParseToken(unsigned char** Text, unsigned int* Token);
void nglFontParseToken(unsigned char** Text, float* Token);
void nglFontParseToken(unsigned char** Text, float* TokenA, float* TokenB);
void nglSetFontBlend(nglFont* Font, unsigned int BlendMode);
void nglSetFontMapFlags(nglFont* Font, unsigned int MapFlags);
unsigned int nglGetFontMapFlags(nglFont* Font);
nglStringSection* nglBuildStringList(nglFont* Font, float x, float y, float ScaleX,
                                     float ScaleY, unsigned int Color, unsigned char* Text);
void nglGetStringDimensions(nglFont* Font, const char* Text, unsigned int* Width,
                            unsigned int* Height, float ScaleX, float ScaleY);
void nglGetStringDimensions(nglFont* Font, unsigned int* Width, unsigned int* Height,
                            const char* Fmt, ...);
void nglGetStringDimensions(nglFont* Font, unsigned int* Width, unsigned int* Height,
                            float ScaleX, float ScaleY, const char* Fmt, ...);
void nglListAddString(nglFont* Font, const char* Text, float x, float y, float z,
                      unsigned int Color, float ScaleX, float ScaleY);
void nglListAddString(nglFont* Font, float x, float y, float z, const char* Fmt, ...);
void nglListAddString(nglFont* Font, float x, float y, float z, unsigned int Color,
                      const char* Fmt, ...);
void nglListAddString(nglFont* Font, float x, float y, float z, float ScaleX, float ScaleY,
                      const char* Fmt, ...);
void nglListAddString(nglFont* Font, float x, float y, float z, unsigned int Color,
                      float ScaleX, float ScaleY, const char* Fmt, ...);
void nglFontInit();

namespace apk {
class apkFile;
class apkFileEntry;
}
void nglAPKFontLoadCallback(apk::apkFile* File, apk::apkFileEntry* Entry, void* UserData);
void nglAPKFontDeleteCallback(apk::apkFile* File, apk::apkFileEntry* Entry, void* UserData);

// ============================================================================
// nglStringSection - 32 bytes
// ============================================================================
struct nglStringSection {
    nglStringSection* Next;         // +0x00
    const unsigned char* Text;      // +0x04
    unsigned int      Length;       // +0x08
    float             x;            // +0x0C
    float             y;            // +0x10
    float             ScaleX;       // +0x14
    float             ScaleY;       // +0x18
    unsigned int      Color;        // +0x1C
};
static_assert(sizeof(nglStringSection) == 0x20, "nglStringSection size mismatch");

// ============================================================================
// nglStringNode - 48 bytes
// ============================================================================
class nglStringNode : public nglRenderNode {
public:
    unsigned char*    Text;         // +0x0C
    nglFont*          Font;         // +0x10
    float             x;            // +0x14
    float             y;            // +0x18
    float             z;            // +0x1C
    float             ScaleX;       // +0x20
    float             ScaleY;       // +0x24
    unsigned int      Color;        // +0x28
    nglStringSection* Section;      // +0x2C

    nglStringNode() {}

    virtual void Render();          // ngl_dx_font.o (push-buffer, not yet ported)
    virtual void GetDesc(char* Desc) { if (Desc) *Desc = 0; }
    virtual void GetSortInfo(nglSortInfo& Info) { (void)Info; }
};
static_assert(sizeof(nglStringNode) == 0x30, "nglStringNode size mismatch");

#endif // COD3_NGL_NGL_FONT_H
