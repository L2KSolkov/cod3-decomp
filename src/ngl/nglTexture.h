// ============================================================================
// nglTexture — NGL texture object (44 bytes, verified against IDA local type).
// Source: c:\cod\code\tl\ngl_xboxr\include\nglTexture.h
//
// Owned by ngl_xboxr. Data globals live in various ngl .o files:
//   nglBackBufferTex    ngl_dx_tex_create.o (not yet ported -> extern here)
//   nglCurSurface       ngl_dx_draw.o       (defined in ngl_dx_draw.cpp)
//   nglCurDepthBuffer   ngl_dx_draw.o       (defined in ngl_dx_draw.cpp)
// ============================================================================
#ifndef COD3_NGL_NGLTEXTURE_H
#define COD3_NGL_NGLTEXTURE_H

#include <cstddef>

#include "d3d8.h"
#include "core/tlSkipList.h"

namespace apk {
class apkFile;
class apkFileEntry;
}
class tlFixedString;

struct nglTexture {
    int             Width;          // +0x00
    int             Height;         // +0x04
    apk::apkFile*   File;           // +0x08
    int             LastFrameRef;   // +0x0C
    tlFixedString*  FileName;       // +0x10
    D3DBaseTexture* Texture;        // +0x14
    unsigned int    Flags;          // +0x18
    D3DSurface*     RenderTarget;   // +0x1C
    nglTexture*     ZTexture;       // +0x20
    unsigned int    NFrames;        // +0x24
    nglTexture**    Frames;         // +0x28

    int GetWidth() const;   // ?GetWidth@nglTexture@@QBEHXZ (0x6816B0)
    int GetHeight() const;  // ?GetHeight@nglTexture@@QBEHXZ (0x6816C0)
};
static_assert(sizeof(nglTexture) == 0x2C, "nglTexture size mismatch");

inline int nglTexture::GetWidth() const
{
    return Width;
}

inline int nglTexture::GetHeight() const
{
    return Height;
}

// Skip-list key accessor (free function, defined in ngl_internal.cpp).
const tlFixedString* GetKey(const nglTexture* t);

// ngl_texture.o (data, defined in ngl_texture.cpp)
extern nglTexture* nglDefaultTex;
extern nglTexture* nglWhiteTex;
extern int nglTextureAnimFrame;
extern float nglIFLSpeed;
extern tlSkipList<nglTexture, tlFixedString> nglTextureDirectory;

// ngl_dx_texture.o (functions, not yet ported)
extern nglTexture* nglGetFrontBufferTex(void);
extern void nglSaveTexture(nglTexture* Tex, const char* FileName);
extern void ngliUnloadTexture(apk::apkFile* File, apk::apkFileEntry* Entry);

// ngl_dx_core.o (function, not yet ported)
extern void ngliWaitForResource(void);

// ngl_dx_tex_create.o (data, not yet ported)
extern nglTexture nglBackBufferTex;
extern nglTexture nglFrontBufferTex;

// ngl_dx_draw.o (data, defined in ngl_dx_draw.cpp)
extern D3DSurface* nglCurSurface;
extern D3DSurface* nglCurDepthBuffer;

#endif // COD3_NGL_NGLTEXTURE_H
