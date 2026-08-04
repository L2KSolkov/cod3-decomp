// ============================================================================
// d3d8.h — Xbox D3D8 API shim declarations (d3d8d library, NEVER reconstruct).
// Source: Xbox XDK Dec 2003 d3d8.h
//
// Opaque types + the handful of D3D8 entry points the ported game/engine
// objects call. Implementations land in the future D3D11 backend
// (Phase 6); until then the linker satisfies them via /FORCE:UNRESOLVED.
// ============================================================================
#pragma once

#include <cstddef>

#ifdef __cplusplus

// ---- Opaque D3D8 objects ------------------------------------------------
// D3DResource is the common base (12 bytes, verified against IDA); the engine
// stores D3D8 textures in nglTexture and casts them to D3DResource*.
struct D3DResource {
    unsigned int Common;  // +0x00
    unsigned int Data;    // +0x04
    unsigned int Lock;    // +0x08
};
static_assert(sizeof(D3DResource) == 0x0C, "D3DResource size mismatch");

struct D3DBaseTexture : D3DResource {};
struct D3DTexture : D3DBaseTexture {};
struct D3DCubeTexture : D3DBaseTexture {};
struct D3DVolumeTexture : D3DBaseTexture {};
struct D3DSurface {};

// ---- Cube-map face selector (D3DCUBEMAP_FACES) ---------------------------
enum _D3DCUBEMAP_FACES {
    D3DCUBEMAP_FACE_POSITIVE_X = 0,
    D3DCUBEMAP_FACE_NEGATIVE_X = 1,
    D3DCUBEMAP_FACE_POSITIVE_Y = 2,
    D3DCUBEMAP_FACE_NEGATIVE_Y = 3,
    D3DCUBEMAP_FACE_POSITIVE_Z = 4,
    D3DCUBEMAP_FACE_NEGATIVE_Z = 5,
};

// ---- Render-state selector (D3DRS_*, Xbox D3D8) --------------------------
enum _D3DRENDERSTATETYPE {
    D3DRS_MULTISAMPLEANTIALIAS = 152,  // 0x98
};

// ---- Pixel format selector (D3DFMT_*) ------------------------------------
enum _D3DFORMAT {
    D3DFMT_YUY2 = 36,
};

// ---- Vertex shader input element (16 bytes, verified against IDA) --------
struct _D3DVERTEXSHADERINPUT {
    unsigned int StreamIndex;  // +0x00
    unsigned int Offset;       // +0x04
    unsigned int Format;       // +0x08
    unsigned char TessType;    // +0x0C
    unsigned char TessSource;  // +0x0D
    // +0x0E..+0x0F padding
};
static_assert(sizeof(_D3DVERTEXSHADERINPUT) == 0x10, "_D3DVERTEXSHADERINPUT size mismatch");

// ---- Vertex attribute format table (256 bytes, verified against IDA) -----
struct _D3DVERTEXATTRIBUTEFORMAT {
    _D3DVERTEXSHADERINPUT Input[16];  // +0x00
};
static_assert(sizeof(_D3DVERTEXATTRIBUTEFORMAT) == 0x100, "_D3DVERTEXATTRIBUTEFORMAT size mismatch");

// ---- Present parameters (Xbox D3D8, 68 bytes, verified against IDA) ------
struct _D3DPRESENT_PARAMETERS_ {
    unsigned int BackBufferWidth;                 // +0x00
    unsigned int BackBufferHeight;                // +0x04
    unsigned int BackBufferFormat;                // +0x08 (_D3DFORMAT)
    unsigned int BackBufferCount;                 // +0x0C
    unsigned int MultiSampleType;                 // +0x10
    unsigned int SwapEffect;                      // +0x14 (_D3DSWAPEFFECT)
    void*        hDeviceWindow;                   // +0x18
    int          Windowed;                        // +0x1C
    int          EnableAutoDepthStencil;          // +0x20
    unsigned int AutoDepthStencilFormat;          // +0x24
    unsigned int Flags;                           // +0x28
    unsigned int FullScreen_RefreshRateInHz;      // +0x2C
    unsigned int FullScreen_PresentationInterval; // +0x30
    D3DSurface*  BufferSurfaces[3];               // +0x34
    D3DSurface*  DepthStencilSurface;             // +0x40
};
static_assert(sizeof(_D3DPRESENT_PARAMETERS_) == 0x44, "_D3DPRESENT_PARAMETERS_ size mismatch");

// ---- D3D8 entry points (stdcall, @N-decorated like the XDK exports) ------
extern "C" {
void         __stdcall D3DDevice_SetRenderTarget(D3DSurface* pRenderTarget, D3DSurface* pZBuffer);
D3DSurface*  __stdcall D3DCubeTexture_GetCubeMapSurface2(D3DBaseTexture* pTexture,
                                                        _D3DCUBEMAP_FACES FaceType,
                                                        unsigned int Level);
D3DSurface*  __stdcall D3DTexture_GetSurfaceLevel2(D3DBaseTexture* pTexture, unsigned int Level);
unsigned int __stdcall D3DResource_Release(D3DResource* pResource);
int          __stdcall D3DDevice_SetRenderState_ParameterCheck(unsigned int State, unsigned int Value);
void         __stdcall D3DDevice_SetRenderState_MultiSampleAntiAlias(unsigned int Value);
void         __stdcall D3DResource_Register(D3DResource* pResource, void* pBase);
void*          __stdcall D3DDevice_CreateTexture2(unsigned int Width, unsigned int Height,
                                                  unsigned int Depth, unsigned int Levels,
                                                  unsigned int Usage, unsigned int Format,
                                                  unsigned int Type);
D3DSurface*  __stdcall D3DDevice_CreateSurface2(unsigned int Width, unsigned int Height,
                                                unsigned int Usage, unsigned int Format);
}

// ---- XGRPH entry points (xgraphicsd) --------------------------------------
extern "C" {
int __stdcall XGIsSwizzledFormat(unsigned int Format);
}

#endif // __cplusplus
