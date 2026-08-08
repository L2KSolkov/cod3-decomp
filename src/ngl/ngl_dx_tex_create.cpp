// ============================================================================
// ngl_dx_tex_create.cpp - texture creation/destruction + framebuffer textures.
// Source: src/dx/ngl_dx_tex_create.cpp (ngl_xboxr)
// Verified against IDA (ngl_xboxr:ngl_dx_tex_create.o).
// Data: nglFrontBufferTex/nglBackBufferTex/nglDepthBufferTex +
// nglNormalCubeMapTex/nglNormalCubeMapTexWidth + static names live here.
// ============================================================================

#include "ngl/nglTexture.h"
#include "ngl/ngl_dx_gpu.h"
#include "ngl/ngl_dx_quad.h"
#include "core/tlFixedString.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

// ============================================================================
// Cross-object externs
// ============================================================================
extern nglTexture* nglCreateTexture(unsigned int Flags, unsigned int Format, int Width,
                                    int Height, int Depth, int Levels);  // ngl_gpu_texture.o
extern bool nglCanReleaseTexture(nglTexture* Tex);                       // ngl_texture.o (ported)
extern void ngliWaitForResource(void);                                   // ngl_dx_core.o
extern void nglUpdateFilterTextures(void);                               // ngl_dx_filters.o
extern void tlWarning(const char* fmt, ...);
extern void tlFatal(const char* fmt, ...);
extern void* tlMemAlloc(unsigned int Size, unsigned int Align, unsigned int Flags);
extern void tlMemFree(void* Ptr);
extern bool _tlAssert(const char* file, int line, const char* expr, const char* desc);

// ngl_dx_core.o (data, not yet ported)
extern class gpuD3DDevice* nglDev;
extern _D3DPRESENT_PARAMETERS_ nglPresentParams;

// ============================================================================
// Globals (data)
// ============================================================================
nglTexture nglFrontBufferTex;
nglTexture nglBackBufferTex;
nglTexture nglDepthBufferTex;
nglTexture* nglNormalCubeMapTex = NULL;
unsigned int nglNormalCubeMapTexWidth = 0;

static tlFixedString FrontBufferName;
static unsigned char FrontBufferName_InitGuard;
static tlFixedString nglAuxBufferName;
static tlFixedString nglBackBufferName;
static tlFixedString nglDepthBufferName;
static tlFixedString nglCreatedTexture_FileName_0;

static void InitFrontBufferName() {
    if ((FrontBufferName_InitGuard & 1) == 0) {
        FrontBufferName_InitGuard |= 1;
        FrontBufferName = tlFixedString("nglFrontBuffer");
    }
}

// ============================================================================
// nglSaveTexture - ea: 0x84AD10
// ============================================================================
void nglSaveTexture(nglTexture* Tex, const char* FileName) {
    char Path[260];
    _D3DSURFACE_DESC Desc;
    sprintf(Path, "D:\\%s.bmp", FileName);
    D3DSurface* SurfaceLevel2 = D3DTexture_GetSurfaceLevel2(Tex->Texture, 0);
    D3DSurface_GetDesc(SurfaceLevel2, &Desc);
    switch (Desc.Format) {
    case D3DFMT_LIN_R5G6B5:
    case D3DFMT_LIN_A8R8G8B8:
    case D3DFMT_LIN_X1R5G5B5:
    case D3DFMT_LIN_X8R8G8B8:
        XGWriteSurfaceToFile(SurfaceLevel2, Path);
        D3DResource_Release((D3DResource*)SurfaceLevel2);
        break;
    default:
        tlFatal("Texture format cannot be saved, only front-buffer formats supported.");
        D3DResource_Release((D3DResource*)SurfaceLevel2);
        break;
    }
}

// ============================================================================
// nglCreateTextureFromFile - ea: 0x84ADB0
// ============================================================================
nglTexture* nglCreateTextureFromFile(void* Data, unsigned int Size) {
    nglTexture* v2 = (nglTexture*)tlMemAlloc(0x2Cu, 8u, 0x1000000);
    memset(v2, 0, 0x2Cu);
    D3DXCreateTextureFromFileInMemoryEx(nglDev, Data, Size, 0xFFFFFFFF, 0xFFFFFFFF,
                                        0xFFFFFFFF, 0, D3DFMT_UNKNOWN, 0, 0xFFFFFFFF,
                                        0xFFFFFFFF, 0, NULL, NULL, (D3DTexture**)&v2->Texture);
    v2->FileName = &nglCreatedTexture_FileName_0;
    v2->LastFrameRef = -1;
    return v2;
}

// ============================================================================
// nglDestroyTexture - ea: 0x84AE20
// ============================================================================
void nglDestroyTexture(nglTexture* Tex) {
    if (Tex == NULL)
        return;
    if (!nglCanReleaseTexture(Tex)) {
        tlWarning("NGL: Texture %s destroyed while still referenced by the async renderer.\n",
                  Tex->FileName->str);
        ngliWaitForResource();
    }
    unsigned int Flags = Tex->Flags;
    if ((Flags & 4) != 0) {
        if ((Flags & 8) == 0) {
            tlMemFree(Tex->Frames);
            tlMemFree(Tex);
        }
    } else {
        if ((Flags & 0x10) != 0 && (Flags & 0x40) != 0) {
            nglTextureDirectory.Del(Tex->ZTexture);
            nglDestroyTexture(Tex->ZTexture);
        }
        if ((Tex->Flags & 8) == 0) {
            if (Tex->Texture != NULL)
                D3DResource_Release((D3DResource*)Tex->Texture);
            if (Tex->RenderTarget != NULL)
                D3DResource_Release((D3DResource*)Tex->RenderTarget);
            if ((Tex->Flags & 8) == 0)
                tlMemFree(Tex);
        }
    }
}

// ============================================================================
// nglCreateFrontBufferTexture - ea: 0x84AED0
// ============================================================================
void nglCreateFrontBufferTexture(nglTexture* FrontBuf, tlFixedString* FrontBufName) {
    _D3DSURFACE_DESC SurfDesc;
    memset(FrontBuf, 0, sizeof(nglTexture));
    FrontBuf->Flags |= 0x2000u;
    FrontBuf->FileName = FrontBufName;
    nglTextureDirectory.Add(FrontBuf);
    FrontBuf->Texture = D3DDevice_GetBackBuffer2(-1);
    D3DTexture_GetLevelDesc(FrontBuf->Texture, 0, &SurfDesc);
    FrontBuf->Width = SurfDesc.Width;
    FrontBuf->Height = SurfDesc.Height;
}

// ============================================================================
// nglCreateAuxBufferTexture - ea: 0x84AF30
// ============================================================================
void nglCreateAuxBufferTexture(nglTexture* AuxBuf, int w, int h) {
    memset(AuxBuf, 0, sizeof(nglTexture));
    AuxBuf->Flags |= 0x6010u;
    AuxBuf->FileName = &nglAuxBufferName;
    AuxBuf->Width = w;
    AuxBuf->Height = h;
    nglTextureDirectory.Add(AuxBuf);
    AuxBuf->Texture = (D3DBaseTexture*)D3DDevice_CreateTexture2(
        w, h, 1u, 1u, 1u, nglPresentParams.BackBufferFormat, D3DRTYPE_TEXTURE);
    AuxBuf->Flags &= ~0x1000u;
}

// ============================================================================
// nglCreatePrevBufferTexture - ea: 0x84AFA0
// ============================================================================
void nglCreatePrevBufferTexture(nglTexture* AuxBuf, int w, int h) {
    memset(AuxBuf, 0, sizeof(nglTexture));
    AuxBuf->Flags |= 0x6000u;
    AuxBuf->FileName = &nglAuxBufferName;
    AuxBuf->Width = w;
    AuxBuf->Height = h;
    nglTextureDirectory.Add(AuxBuf);
    AuxBuf->Texture = (D3DBaseTexture*)D3DDevice_CreateTexture2(
        w, h, 1u, 1u, 1u, nglPresentParams.BackBufferFormat, D3DRTYPE_TEXTURE);
    AuxBuf->Flags &= ~0x1000u;
}

// ============================================================================
// nglCreateBackBufferTexture - ea: 0x84B010
// ============================================================================
void nglCreateBackBufferTexture(nglTexture* BackBuf, nglTexture* DepthBuf) {
    _D3DSURFACE_DESC SurfDesc;
    memset(BackBuf, 0, sizeof(nglTexture));
    memset(DepthBuf, 0, sizeof(nglTexture));
    BackBuf->Flags = BackBuf->Flags | 0x2050;
    BackBuf->FileName = &nglBackBufferName;
    BackBuf->ZTexture = DepthBuf;
    nglTextureDirectory.Add(BackBuf);
    DepthBuf->Flags |= 0x2020u;
    DepthBuf->FileName = &nglDepthBufferName;
    nglTextureDirectory.Add(DepthBuf);
    BackBuf->Texture = D3DDevice_GetBackBuffer2(0);
    DepthBuf->RenderTarget = D3DDevice_GetDepthStencilSurface2();
    D3DTexture_GetLevelDesc(BackBuf->Texture, 0, &SurfDesc);
    BackBuf->Width = SurfDesc.Width;
    BackBuf->Height = SurfDesc.Height;
    DepthBuf->Width = BackBuf->Width;
    DepthBuf->Height = BackBuf->Height;
}

// ============================================================================
// nglInitFrameBufferTexture - ea: 0x84B0B0
// ============================================================================
void nglInitFrameBufferTexture() {
    InitFrontBufferName();
    nglCreateFrontBufferTexture(&nglFrontBufferTex, &FrontBufferName);
    nglCreateBackBufferTexture(&nglBackBufferTex, &nglDepthBufferTex);
}

// ============================================================================
// nglUpdateInternalTextures - ea: 0x84B100
// ============================================================================
void nglUpdateInternalTextures() {
    D3DResource_Release((D3DResource*)nglFrontBufferTex.Texture);
    D3DResource_Release((D3DResource*)nglBackBufferTex.Texture);
    D3DResource_Release((D3DResource*)nglDepthBufferTex.RenderTarget);
    nglTextureDirectory.Del(&nglFrontBufferTex);
    nglTextureDirectory.Del(&nglBackBufferTex);
    nglTextureDirectory.Del(&nglDepthBufferTex);
    InitFrontBufferName();
    nglCreateFrontBufferTexture(&nglFrontBufferTex, &FrontBufferName);
    nglCreateBackBufferTexture(&nglBackBufferTex, &nglDepthBufferTex);
    nglUpdateFilterTextures();
}

// ============================================================================
// nglDestroyNormalCubeMap - ea: 0x84B1A0
// ============================================================================
void nglDestroyNormalCubeMap() {
    nglDestroyTexture(nglNormalCubeMapTex);
    nglNormalCubeMapTex = NULL;
    nglNormalCubeMapTexWidth = 0;
}

// ============================================================================
// nglCreateNormalCubeMap - ea: 0x84B1C0
// Generates a swizzled normal-map cube texture: each face stores normalized
// direction vectors encoded as 0xFFRRGGBB (alpha fixed to 0xFF).
// ============================================================================
void nglCreateNormalCubeMap(unsigned int Width) {
    if (nglNormalCubeMapTexWidth == Width)
        return;
    if (nglNormalCubeMapTex != NULL) {
        nglDestroyTexture(nglNormalCubeMapTex);
        nglNormalCubeMapTex = NULL;
        nglNormalCubeMapTexWidth = 0;
    }
    if (Width == 0 && _tlAssert("src/dx/ngl_dx_tex_create.cpp", 317, "Width >= 1",
                                "NormalCubeMap width has to be >= 1 !"))
        __debugbreak();

    nglTexture* Tex = nglCreateTexture(0x100u, 6u, (int)Width, (int)Width, 0, 1);
    // Name the cube-map texture (static tlFixedString "nglNormalCubeMapTex").
    static tlFixedString NormalCubeMapName("nglNormalCubeMapTex");
    Tex->FileName = &NormalCubeMapName;

    unsigned int* pBits = (unsigned int*)tlMemAlloc(4 * Width * Width, 8u, 0x1000000);
    for (unsigned int Face = 0; Face < 6; ++Face) {
        unsigned int* pDest = pBits;
        for (unsigned int i = 0; i < Width; ++i) {
            float y = (float)i / (float)(Width - 1) * 2.0f - 1.0f;
            for (unsigned int j = 0; j < Width; ++j) {
                float x = (float)j / (float)(Width - 1) * 2.0f - 1.0f;
                float nx, ny, nz;
                switch (Face) {
                case 0: nx = 1.0f; ny = -y; nz = -x; break;   // +X
                case 1: nx = -1.0f; ny = -y; nz = x; break;   // -X
                case 2: nx = x; ny = 1.0f; nz = y; break;     // +Y
                case 3: nx = x; ny = -1.0f; nz = -y; break;   // -Y
                case 4: nx = x; ny = -y; nz = 1.0f; break;    // +Z
                default: nx = -x; ny = -y; nz = -1.0f; break; // -Z
                }
                float len = sqrtf(nx*nx + ny*ny + nz*nz);
                nx /= len; ny /= len; nz /= len;
                unsigned int r = (unsigned int)((nx * 0.5f + 0.5f) * 255.0f);
                unsigned int g = (unsigned int)((ny * 0.5f + 0.5f) * 255.0f);
                unsigned int b = (unsigned int)((nz * 0.5f + 0.5f) * 255.0f);
                *pDest++ = 0xFF000000 | (r << 16) | (g << 8) | b;
            }
        }
        D3DSurface* CubeMapSurface2 = D3DCubeTexture_GetCubeMapSurface2(
            (D3DCubeTexture*)Tex->Texture, (_D3DCUBEMAP_FACES)Face, 0);
        D3DLOCKED_RECT lockedRect;
        D3DSurface_LockRect(CubeMapSurface2, &lockedRect, NULL, 0);
        XGSwizzleRect(pBits, 0, NULL, lockedRect.pBits, Width, Width, NULL, 4u);
        D3DResource_Release((D3DResource*)CubeMapSurface2);
    }
    tlMemFree(pBits);
    nglNormalCubeMapTex = Tex;
    nglNormalCubeMapTexWidth = Width;
}
