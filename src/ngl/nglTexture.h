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

namespace apk {
class apkFile;
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
};
static_assert(sizeof(nglTexture) == 0x2C, "nglTexture size mismatch");

// ngl_dx_tex_create.o (data, not yet ported)
extern nglTexture nglBackBufferTex;

// ngl_dx_draw.o (data, defined in ngl_dx_draw.cpp)
extern D3DSurface* nglCurSurface;
extern D3DSurface* nglCurDepthBuffer;

#endif // COD3_NGL_NGLTEXTURE_H
