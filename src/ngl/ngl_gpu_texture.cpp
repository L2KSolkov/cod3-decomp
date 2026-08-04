// ============================================================================
// ngl_gpu_texture.cpp — texture creation (1 non-inline func + 1 data global).
// Source: source/ngl_gpu_texture.cpp (ngl_xboxr)
// Verified against IDA (ngl_xboxr:ngl_gpu_texture.o):
//   nglCreateTexture @0x84AB00 (?nglCreateTexture@@YAPAUnglTexture@@IIHHHH@Z)
//   nglCreatedTexture_FileName (data, tlFixedString, static init "created")
//   All D3DDevice/gpuD3DDevice/gpuCreate* wrappers are inline COMDATs -> header.
// ============================================================================
#include "nglTexture.h"
#include "ngl_dx_gpu.h"
#include "core/tlFixedString.h"

#include <string.h>

// ============================================================================
// Data globals owned by ngl_gpu_texture.o
// ============================================================================
static tlFixedString nglCreatedTexture_FileName;
static unsigned char nglCreatedTexture_InitGuard;

// ============================================================================
// nglCreateTexture — allocate an nglTexture and create the underlying D3D
// resource based on Flags:
//   bit 0x10        — render-target surface
//   bit 0x20        — create a surface (render target / depth stencil)
//   bit 0x40        — also create a depth-stencil Z texture
//   bit 0x100       — cube texture
//   bit 0x200       — volume texture
//   NGL_TEX_YUY2 etc set by format handling.
// ea: 0x84AB00
// ============================================================================
nglTexture* nglCreateTexture(unsigned int Flags, unsigned int Format,
                             int Width, int Height, int Depth, int Levels) {
    nglTexture* tex = (nglTexture*)tlMemAlloc(0x2C, 8, 0x1000000);
    memset(tex, 0, 0x2C);

    if ((nglCreatedTexture_InitGuard & 1) == 0) {
        nglCreatedTexture_InitGuard |= 1;
        nglCreatedTexture_FileName = tlFixedString("created");
    }

    tex->FileName = &nglCreatedTexture_FileName;
    tex->Flags = Flags;
    tex->Width = Width;
    tex->Height = Height;

    if (XGIsSwizzledFormat(Format) != 0)
        tex->Flags |= 0x4000;
    if (Format == 36)  // D3DFMT_YUY2
        tex->Flags |= 0x8000;

    if ((Flags & 0x20) != 0) {
        D3DSurface* Surface;
        if ((Flags & 0x10) != 0)
            Surface = D3DDevice_CreateSurface2(Width, Height, 1, Format);
        else
            Surface = D3DDevice_CreateSurface2(Width, Height, 2, Format);
        tex->RenderTarget = Surface;
    }

    if ((Flags & 0x20) == 0) {
        if ((Flags & 0x100) != 0) {
            tex->Texture = (D3DBaseTexture*)D3DDevice_CreateTexture2(Width, Width, 1, Levels, 0x200, Format,
                                                    D3DRTYPE_CUBETEXTURE);
        } else if ((Flags & 0x200) != 0) {
            tex->Texture = gpuCreateVolumeTexture(Width, Height, Depth, Levels,
                                                  (gpuTextureFormat)Format, false,
                                                  NGL_GPU_MULTISAMPLE_NONE);
        } else {
            tex->Texture = gpuCreateTexture(Width, Height, Levels, (gpuTextureFormat)Format, false,
                                            NGL_GPU_MULTISAMPLE_NONE);
        }
    }

    if ((Flags & 0x40) != 0)
        tex->ZTexture = nglCreateTexture((Flags & 0xff) | 0x20, 0x2E, Width, Height, 0, 1);

    tex->LastFrameRef = -1;
    return tex;
}
