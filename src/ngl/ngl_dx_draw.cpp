// ============================================================================
// ngl_dx_draw.cpp — D3D render-target switching (1 non-inline func + 2 data).
// Source: source/ngl_dx_draw.cpp
// Verified against IDA (ngl_xboxr:ngl_dx_draw.o):
//   nglDxSetRenderTarget @0x851090  (?nglDxSetRenderTarget@@YAXPBUnglTexture@@0IH@Z)
//   nglCurSurface       @0x10E69C8 (data, D3DSurface*)
//   nglCurDepthBuffer   @0x10E69CC (data, D3DSurface*)
// ============================================================================
#include "nglTexture.h"

// ngl_dx_tex_create.o (data, not yet ported -> /FORCE:UNRESOLVED)
extern nglTexture nglBackBufferTex;

D3DSurface* nglCurSurface = 0;
D3DSurface* nglCurDepthBuffer = 0;

// ============================================================================
// nglDxSetRenderTarget — bind color + depth surfaces for the current pass.
// ea: 0x851090
// ============================================================================
void nglDxSetRenderTarget(const nglTexture* RenderTarget, const nglTexture* DepthTarget,
                          unsigned int MipLevel, int CubeMapFace) {
    D3DSurface* Texture = 0;
    D3DSurface* zBuffer = 0;

    if (RenderTarget == &nglBackBufferTex) {
        Texture = (D3DSurface*)RenderTarget->Texture;
    } else if (RenderTarget != 0) {
        D3DBaseTexture* pTex = RenderTarget->Texture;
        if ((RenderTarget->Flags & 0x100) != 0)
            Texture = D3DCubeTexture_GetCubeMapSurface2(pTex, (_D3DCUBEMAP_FACES)CubeMapFace, MipLevel);
        else
            Texture = D3DTexture_GetSurfaceLevel2(pTex, MipLevel);
    }

    const nglTexture* zTexture = DepthTarget;
    if (DepthTarget != 0 || (RenderTarget != 0 && (zTexture = RenderTarget->ZTexture) != 0))
        zBuffer = zTexture->RenderTarget;

    if (nglCurSurface != Texture || nglCurDepthBuffer != zBuffer) {
        D3DDevice_SetRenderTarget(Texture, zBuffer);
        nglCurSurface = Texture;
        nglCurDepthBuffer = zBuffer;
    }

    if (RenderTarget != &nglBackBufferTex && Texture != 0)
        D3DResource_Release((D3DResource*)Texture);
}
