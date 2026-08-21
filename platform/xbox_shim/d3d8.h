// ============================================================================
// d3d8.h — Xbox D3D8 API shim declarations (d3d8d library, NEVER reconstruct).
// Source: Xbox XDK Dec 2003 d3d8.h
//
// Opaque types + the handful of D3D8 entry points the ported game/engine
// objects call. Implementations land in the Win32 D3D9 backend
// (Phase 6); until then the linker satisfies them via /FORCE:UNRESOLVED.
// ============================================================================
#pragma once

#include <cstddef>
#include <stdint.h>

#ifdef __cplusplus

// ---- Opaque D3D8 objects ------------------------------------------------
// D3DResource is the common base (12 bytes, verified against IDA); the engine
// stores D3D8 textures in nglTexture and casts them to D3DResource*.
struct D3DResource {
    unsigned int Common;  // +0x00
    unsigned int Data;    // +0x04
    unsigned int Lock;    // +0x08
    void __stdcall Register(void* pBase);
};
static_assert(sizeof(D3DResource) == 0x0C, "D3DResource size mismatch");

// ---- Locked rect (Xbox D3D8, 8 bytes, verified against IDA) ---------------
struct _D3DLOCKED_RECT {
    int  Pitch;   // +0x00
    void* pBits;  // +0x04
};
static_assert(sizeof(_D3DLOCKED_RECT) == 8, "_D3DLOCKED_RECT size mismatch");
typedef _D3DLOCKED_RECT D3DLOCKED_RECT;

// ---- Viewport (Xbox D3D8, 24 bytes, verified against IDA) ----------------
struct _D3DVIEWPORT8 {
    unsigned int X;       // +0x00
    unsigned int Y;       // +0x04
    unsigned int Width;   // +0x08
    unsigned int Height;  // +0x0C
    float MinZ;            // +0x10
    float MaxZ;            // +0x14
};
static_assert(sizeof(_D3DVIEWPORT8) == 0x18, "_D3DVIEWPORT8 size mismatch");
typedef _D3DVIEWPORT8 D3DVIEWPORT8;

// Xbox D3D8 clear-rectangle type (from the IDA local type database).  The
// native D3D9 headers provide the same layout when they are included first.
#ifndef D3DRECT_DEFINED
struct _D3DRECT {
    int x1;
    int y1;
    int x2;
    int y2;
};
typedef _D3DRECT D3DRECT;
#define D3DRECT_DEFINED
#endif

// ---- Cube-map face selector (D3DCUBEMAP_FACES) ---------------------------
enum _D3DCUBEMAP_FACES {
    D3DCUBEMAP_FACE_POSITIVE_X = 0,
    D3DCUBEMAP_FACE_NEGATIVE_X = 1,
    D3DCUBEMAP_FACE_POSITIVE_Y = 2,
    D3DCUBEMAP_FACE_NEGATIVE_Y = 3,
    D3DCUBEMAP_FACE_POSITIVE_Z = 4,
    D3DCUBEMAP_FACE_NEGATIVE_Z = 5,
};

// ---- Pixel format selector (D3DFMT_*, Xbox D3D8) -------------------------
enum _D3DFORMAT {
    D3DFMT_L8 = 0x0,
    D3DFMT_AL8 = 0x1,
    D3DFMT_A1R5G5B5 = 0x2,
    D3DFMT_X1R5G5B5 = 0x3,
    D3DFMT_A4R4G4B4 = 0x4,
    D3DFMT_R5G6B5 = 0x5,
    D3DFMT_A8R8G8B8 = 0x6,
    D3DFMT_X8R8G8B8 = 0x7,
    D3DFMT_P8 = 0xB,
    D3DFMT_DXT1 = 0xC,
    D3DFMT_DXT3 = 0xE,
    D3DFMT_DXT5 = 0xF,
    D3DFMT_LIN_A1R5G5B5 = 0x10,
    D3DFMT_LIN_R5G6B5 = 0x11,
    D3DFMT_LIN_A8R8G8B8 = 0x12,
    D3DFMT_LIN_L8 = 0x13,
    D3DFMT_LIN_R8B8 = 0x16,
    D3DFMT_LIN_G8B8 = 0x17,
    D3DFMT_LIN_A4R4G4B4 = 0x1D,
    D3DFMT_LIN_X1R5G5B5 = 0x1C,
    D3DFMT_LIN_X8R8G8B8 = 0x1E,
    D3DFMT_LIN_A8 = 0x1F,
    D3DFMT_LIN_A8L8 = 0x20,
    D3DFMT_A8 = 0x19,
    D3DFMT_A8L8 = 0x1A,
    D3DFMT_LIN_AL8 = 0x1B,
    D3DFMT_YUY2 = 0x24,
    D3DFMT_UYVY = 0x25,
    D3DFMT_V8U8 = 0x28,
    D3DFMT_L6V5U5 = 0x27,
    D3DFMT_D24S8 = 0x2A,
    D3DFMT_F24S8 = 0x2B,
    D3DFMT_D16 = 0x2C,
    D3DFMT_F16 = 0x2D,
    D3DFMT_LIN_D24S8 = 0x2E,
    D3DFMT_LIN_F24S8 = 0x2F,
    D3DFMT_LIN_D16 = 0x30,
    D3DFMT_LIN_F16 = 0x31,
    D3DFMT_L16 = 0x32,
    D3DFMT_V16U16 = 0x33,
    D3DFMT_LIN_L16 = 0x35,
    D3DFMT_LIN_V16U16 = 0x36,
    D3DFMT_LIN_R6G5B5 = 0x37,
    D3DFMT_R6G5B5 = 0x27,
    D3DFMT_R4G4B4A4 = 0x39,
    D3DFMT_A8B8G8R8 = 0x3A,
    D3DFMT_B8G8R8A8 = 0x3B,
    D3DFMT_R8G8B8A8 = 0x3C,
    D3DFMT_R5G5B5A1 = 0x38,
    D3DFMT_LIN_R4G4B4A4 = 0x3E,
    D3DFMT_LIN_A8B8G8R8 = 0x3F,
    D3DFMT_LIN_B8G8R8A8 = 0x40,
    D3DFMT_LIN_R8G8B8A8 = 0x41,
    D3DFMT_UNKNOWN = 0xFFFFFFFF,
};


// ---- Surface description (Xbox D3D8, 28 bytes, verified against IDA) ------
struct _D3DSURFACE_DESC {
    _D3DFORMAT Format;          // +0x00
    unsigned int Type;          // +0x04
    unsigned int Usage;         // +0x08
    unsigned int Size;          // +0x0C
    unsigned int MultiSampleType;// +0x10
    unsigned int Width;         // +0x14
    unsigned int Height;        // +0x18
};
static_assert(sizeof(_D3DSURFACE_DESC) == 0x1C, "_D3DSURFACE_DESC size mismatch");

// Forward decls so the entry-point block below can use these as pointers.
struct D3DBaseTexture;
struct D3DTexture;
struct D3DCubeTexture;
struct D3DSurface;

// Forward decls used by the inline wrapper methods below.
extern "C" {
D3DSurface* __stdcall D3DTexture_GetSurfaceLevel2(D3DBaseTexture* pTexture, unsigned int Level);
D3DSurface* __stdcall D3DCubeTexture_GetCubeMapSurface2(D3DBaseTexture* pTexture,
                                                        _D3DCUBEMAP_FACES FaceType,
                                                        unsigned int Level);
unsigned int __stdcall D3DBaseTexture_GetLevelCount(D3DBaseTexture* pTexture);
int         __stdcall D3DSurface_GetDesc(D3DSurface* pSurface, _D3DSURFACE_DESC* pDesc);
void*       __stdcall D3DSurface_LockRect(D3DSurface* pSurface, D3DLOCKED_RECT* pLockedRect,
                                          const void* pRect, unsigned int Flags);
}

struct D3DBaseTexture : D3DResource {
    unsigned int Format;  // +0x0C
    unsigned int Size;    // +0x10

    unsigned int __stdcall GetLevelCount() {
        return D3DBaseTexture_GetLevelCount(this);
    }
};
static_assert(sizeof(D3DBaseTexture) == 0x14, "D3DBaseTexture size mismatch");
struct D3DTexture : D3DBaseTexture {
    unsigned int __stdcall GetSurfaceLevel(unsigned int Level, D3DSurface** ppSurfaceLevel) {
        D3DSurface* SurfaceLevel2 = D3DTexture_GetSurfaceLevel2(this, Level);
        *ppSurfaceLevel = SurfaceLevel2;
        return SurfaceLevel2 != NULL ? 0 : 0x8007000E;
    }
};
struct D3DCubeTexture : D3DBaseTexture {
    unsigned int __stdcall GetCubeMapSurface(_D3DCUBEMAP_FACES FaceType, unsigned int Level,
                                             D3DSurface** ppCubeMapSurface) {
        D3DSurface* CubeMapSurface2 = D3DCubeTexture_GetCubeMapSurface2(this, FaceType, Level);
        *ppCubeMapSurface = CubeMapSurface2;
        return CubeMapSurface2 != NULL ? 0 : 0x8007000E;
    }
};
struct D3DVolumeTexture : D3DBaseTexture {};
struct D3DSurface {
    unsigned int Common;  // +0x00
    unsigned int Data;    // +0x04
    unsigned int Lock;    // +0x08
    unsigned int Format;  // +0x0C
    unsigned int Size;    // +0x10
    D3DBaseTexture* Parent;// +0x14
    int __stdcall GetDesc(_D3DSURFACE_DESC* pDesc) {
        D3DSurface_GetDesc(this, pDesc);
        return 0;
    }
    int __stdcall LockRect(D3DLOCKED_RECT* pLockedRect, const void* pRect, unsigned int Flags) {
        D3DSurface_LockRect(this, pLockedRect, pRect, Flags);
        return 0;
    }
    int __stdcall UnlockRect() {
        return 0;
    }
};
static_assert(sizeof(D3DSurface) == 0x18, "D3DSurface size mismatch");

// ---- Palette (Xbox D3D8, 12 bytes, verified against IDA) -----------------
struct D3DPalette {
    unsigned int Common;   // +0x00
    unsigned int Data;     // +0x04
    unsigned int Lock;     // +0x08
};
static_assert(sizeof(D3DPalette) == 0xC, "D3DPalette size mismatch");

enum _D3DPALETTESIZE {
    D3DPALETTE_256 = 0x0,
    D3DPALETTE_128 = 0x1,
    D3DPALETTE_64  = 0x2,
    D3DPALETTE_32  = 0x3,
    D3DPALETTE_MAX = 0x4,
};

// ---- Device caps (Xbox D3D8, 212 bytes, verified against IDA) ------------
struct _D3DCAPS8 {
    unsigned int DeviceType;
    unsigned int AdapterOrdinal;
    unsigned int Caps;
    unsigned int Caps2;
    unsigned int Caps3;
    unsigned int PresentationIntervals;
    unsigned int CursorCaps;
    unsigned int DevCaps;
    unsigned int PrimitiveMiscCaps;
    unsigned int RasterCaps;
    unsigned int ZCmpCaps;
    unsigned int SrcBlendCaps;
    unsigned int DestBlendCaps;
    unsigned int AlphaCmpCaps;
    unsigned int ShadeCaps;
    unsigned int TextureCaps;
    unsigned int TextureFilterCaps;
    unsigned int CubeTextureFilterCaps;
    unsigned int VolumeTextureFilterCaps;
    unsigned int TextureAddressCaps;
    unsigned int VolumeTextureAddressCaps;
    unsigned int LineCaps;
    unsigned int MaxTextureWidth;
    unsigned int MaxTextureHeight;
    unsigned int MaxVolumeExtent;
    unsigned int MaxTextureRepeat;
    unsigned int MaxTextureAspectRatio;
    unsigned int MaxAnisotropy;
    float MaxVertexW;
    float GuardBandLeft;
    float GuardBandTop;
    float GuardBandRight;
    float GuardBandBottom;
    float ExtentsAdjust;
    unsigned int StencilCaps;
    unsigned int FVFCaps;
    unsigned int TextureOpCaps;
    unsigned int MaxTextureBlendStages;
    unsigned int MaxSimultaneousTextures;
    unsigned int VertexProcessingCaps;
    unsigned int MaxActiveLights;
    unsigned int MaxUserClipPlanes;
    unsigned int MaxVertexBlendMatrices;
    unsigned int MaxVertexBlendMatrixIndex;
    float MaxPointSize;
    unsigned int MaxPrimitiveCount;
    unsigned int MaxVertexIndex;
    unsigned int MaxStreams;
    unsigned int MaxStreamStride;
    unsigned int VertexShaderVersion;
    unsigned int MaxVertexShaderConst;
    unsigned int PixelShaderVersion;
    float MaxPixelShaderValue;
};
static_assert(sizeof(_D3DCAPS8) == 0xD4, "_D3DCAPS8 size mismatch");

// ---- Vertex/index buffer objects (12 bytes each, verified against IDA) ----
struct D3DVertexBuffer {
    unsigned int Common;  // +0x00
    unsigned int Data;    // +0x04
    unsigned int Lock;    // +0x08
};
static_assert(sizeof(D3DVertexBuffer) == 0x0C, "D3DVertexBuffer size mismatch");

struct D3DIndexBuffer {
    unsigned int Common;  // +0x00
    unsigned int Data;    // +0x04
    unsigned int Lock;    // +0x08
};
static_assert(sizeof(D3DIndexBuffer) == 0x0C, "D3DIndexBuffer size mismatch");

// ---- Primitive type selector (D3DPT_*) -----------------------------------
enum _D3DPRIMITIVETYPE {
    D3DPT_POINTLIST = 1,
    D3DPT_LINELIST = 2,
    D3DPT_LINELOOP = 3,
    D3DPT_LINESTRIP = 4,
    D3DPT_TRIANGLELIST = 5,
    D3DPT_TRIANGLESTRIP = 6,
    D3DPT_TRIANGLEFAN = 7,
    D3DPT_QUADLIST = 8,
    D3DPT_QUADSTRIP = 9,
    D3DPT_POLYGON = 10,
    D3DPT_MAX = 11,
};

// ---- Vertex shader stream input (12 bytes, verified against IDA) ---------
struct _D3DSTREAM_INPUT {
    D3DVertexBuffer* VertexBuffer;  // +0x00
    unsigned int     Stride;        // +0x04
    unsigned int     Offset;        // +0x08
};
static_assert(sizeof(_D3DSTREAM_INPUT) == 0x0C, "_D3DSTREAM_INPUT size mismatch");

// ---- Render-state selector (D3DRS_*, Xbox D3D8) --------------------------
enum _D3DRENDERSTATETYPE {
    D3DRS_ALPHAFUNC = 58,          // 0x3A
    D3DRS_ALPHABLENDENABLE = 59,   // 0x3B
    D3DRS_ALPHATESTENABLE = 60,    // 0x3C
    D3DRS_ALPHAREF = 61,           // 0x3D
    D3DRS_SRCBLEND = 62,           // 0x3E
    D3DRS_DESTBLEND = 63,          // 0x3F
    D3DRS_BLENDOP = 74,            // 0x4A
    D3DRS_BLENDCOLOR = 75,         // 0x4B
    D3DRS_PSALPHAINPUTS1 = 1,      // 0x01
    D3DRS_PS_MIN = 0,              // 0x00
    D3DRS_PSCONSTANT0_0 = 10,      // 0x0A
    D3DRS_PSCONSTANT0_1 = 11,      // 0x0B
    D3DRS_PSCONSTANT1_0 = 18,      // 0x12
    D3DRS_PSCONSTANT1_1 = 19,      // 0x13
    D3DRS_PSRGBINPUTS0 = 34,       // 0x22
    D3DRS_PSRGBINPUTS1 = 35,       // 0x23
    D3DRS_PS_MAX = 57,             // 0x39
    D3DRS_ZWRITEENABLE = 64,       // 0x40
    D3DRS_STENCILPASS = 69,        // 0x45
    D3DRS_STENCILFUNC = 70,        // 0x46
    D3DRS_STENCILMASK = 72,        // 0x48
    D3DRS_COLORWRITEENABLE = 67,   // 0x43
    D3DRS_SPECULARENABLE = 103,    // 0x67
    D3DRS_CULLMODE = 147,          // 0x93
    D3DRS_ZENABLE = 143,           // 0x8F
    D3DRS_ZBIAS = 149,              // 0x95
    D3DRS_STENCILENABLE = 144,     // 0x90
    D3DRS_ROPZCMPALWAYSREAD = 163, // 0xA3
    D3DRS_MULTISAMPLEANTIALIAS = 152,  // 0x98
    D3DRS_SIMPLE_MAX = 92,         // 0x5C
    D3DRS_FOGCOLOR = 0x8A,          // 0x8A
    D3DRS_PRESENTATIONINTERVAL = 127,  // 0x7F
    D3DRS_YUVENABLE = 160,         // 0xA0
};

// ---- Device type (D3DDEVTYPE) -------------------------------------------
enum _D3DDEVTYPE {
    D3DDEVTYPE_HAL = 1,
};

// ---- Swap effect (D3DSWAPEFFECT) ----------------------------------------
enum _D3DSWAPEFFECT {
    D3DSWAPEFFECT_DISCARD = 1,
};

// ---- GPU callback type (D3DCALLBACKTYPE) --------------------------------
enum _D3DCALLBACKTYPE {
    D3DCALLBACK_WRITE = 1,
};

// ---- Vertical-blank data (12 bytes, verified against IDA) ---------------
struct _D3DVBLANKDATA {
    unsigned int VBlank;  // +0x00
    unsigned int Swap;    // +0x04
    unsigned int Flags;   // +0x08
};
static_assert(sizeof(_D3DVBLANKDATA) == 0x0C, "_D3DVBLANKDATA size mismatch");

// ---- Gamma ramp (768 bytes, verified against IDA) -----------------------
struct _D3DGAMMARAMP {
    unsigned char red[256];    // +0x000
    unsigned char green[256];  // +0x100
    unsigned char blue[256];   // +0x200
};
static_assert(sizeof(_D3DGAMMARAMP) == 0x300, "_D3DGAMMARAMP size mismatch");

// ---- Pixel shader definition (opaque, 240 bytes) -------------------------
struct _D3DPixelShaderDef {
    uint8_t data[240];
};

// ---- Direct3D object (opaque; created by Direct3DCreate8) ---------------
struct Direct3D;

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

// ---- Texture-stage selector (D3DTSS_*) -----------------------------------
// Xbox D3D8 uses a compacted state enum starting at 0 (verified against IDA).
enum _D3DTEXTURESTAGESTATETYPE {
    D3DTSS_ADDRESSU = 0x0,
    D3DTSS_ADDRESSV = 0x1,
    D3DTSS_ADDRESSW = 0x2,
    D3DTSS_MAGFILTER = 0x3,
    D3DTSS_MINFILTER = 0x4,
    D3DTSS_MIPFILTER = 0x5,
    D3DTSS_MIPMAPLODBIAS = 0x6,
    D3DTSS_MAXMIPLEVEL = 0x7,
    D3DTSS_MAXANISOTROPY = 0x8,
    D3DTSS_COLORKEYOP = 0x9,
    D3DTSS_COLORSIGN = 0xA,
    D3DTSS_ALPHAKILL = 0xB,
    D3DTSS_COLOROP = 0xC,
    D3DTSS_COLORARG0 = 0xD,
    D3DTSS_COLORARG1 = 0xE,
    D3DTSS_COLORARG2 = 0xF,
    D3DTSS_ALPHAOP = 0x10,
    D3DTSS_ALPHAARG0 = 0x11,
    D3DTSS_ALPHAARG1 = 0x12,
    D3DTSS_ALPHAARG2 = 0x13,
    D3DTSS_RESULTARG = 0x14,
    D3DTSS_TEXTURETRANSFORMFLAGS = 0x15,
    D3DTSS_BUMPENVMAT00 = 0x16,
    D3DTSS_BUMPENVMAT01 = 0x17,
    D3DTSS_BUMPENVMAT11 = 0x18,
    D3DTSS_BUMPENVMAT10 = 0x19,
    D3DTSS_BUMPENVLSCALE = 0x1A,
    D3DTSS_BUMPENVLOFFSET = 0x1B,
    D3DTSS_TEXCOORDINDEX = 0x1C,
    D3DTSS_BORDERCOLOR = 0x1D,
    D3DTSS_COLORKEYCOLOR = 0x1E,
    D3DTSS_MAX = 0x20,
    D3DTSS_DEFERRED_TEXTURE_STATE_MAX = 0xB,
};

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
void         __stdcall D3DResource_BlockUntilNotBusy(D3DResource* pResource);
void         __stdcall D3DDevice_SetPalette(unsigned int Stage, D3DPalette* pPalette);
unsigned int __stdcall D3DPalette_Lock2(D3DPalette* pPalette, unsigned int Flags);
void         __stdcall XGSetPaletteHeader(_D3DPALETTESIZE Size, D3DPalette* pPalette, void* Data);
void         __stdcall D3DDevice_SetTexture(unsigned int Stage, D3DBaseTexture* pTexture);
void         __stdcall D3DDevice_SwitchTexture(unsigned int Method, unsigned int Data,
                                               unsigned int Format);
unsigned int __stdcall D3DBaseTexture_GetLevelCount(D3DBaseTexture* pTexture);
void*        __stdcall D3DTexture_LockRect(D3DTexture* pTexture, unsigned int Level,
                                           D3DLOCKED_RECT* pLockedRect, const void* pRect,
                                           unsigned int Flags);
D3DBaseTexture* __stdcall D3DDevice_GetBackBuffer2(int BackBuffer);
D3DSurface*     __stdcall D3DDevice_GetDepthStencilSurface2(void);
D3DSurface*     __stdcall D3DTexture_GetSurfaceLevel2(D3DBaseTexture* pTexture, unsigned int Level);
int             __stdcall D3DSurface_GetDesc(D3DSurface* pSurface, _D3DSURFACE_DESC* pDesc);
int             __stdcall D3DTexture_GetLevelDesc(D3DBaseTexture* pTexture, unsigned int Level,
                                                  _D3DSURFACE_DESC* pDesc);
void*           __stdcall D3DSurface_LockRect(D3DSurface* pSurface, D3DLOCKED_RECT* pLockedRect,
                                              const void* pRect, unsigned int Flags);
int             __stdcall D3DXCreateTextureFromFileInMemoryEx(void* pDevice, const void* pvSrcData,
                                                               unsigned int cbSrcData,
                                                               unsigned int cpWidth,
                                                               unsigned int cpHeight,
                                                               unsigned int cMipLevels,
                                                               unsigned int Usage,
                                                               _D3DFORMAT Format,
                                                               unsigned int Pool,
                                                               unsigned int dwFilter,
                                                               unsigned int dwMipFilter,
                                                               unsigned int ColorKey,
                                                               void* pSrcInfo,
                                                               void* pPalette,
                                                               D3DTexture** ppTexture);
void            __stdcall XGWriteSurfaceToFile(D3DSurface* pSurf, const char* cPath);
void            __stdcall XGSwizzleRect(const void* pSource, unsigned int Pitch, const void* pRect,
                                        void* pDest, unsigned int Width, unsigned int Height,
                                        const void* pPoint, unsigned int BytesPerPixel);
void*          __stdcall D3DDevice_CreateTexture2(unsigned int Width, unsigned int Height,
                                                  unsigned int Depth, unsigned int Levels,
                                                  unsigned int Usage, unsigned int Format,
                                                  unsigned int Type);
D3DSurface*  __stdcall D3DDevice_CreateSurface2(unsigned int Width, unsigned int Height,
                                                unsigned int Usage, unsigned int Format);
D3DVertexBuffer* __stdcall D3DDevice_CreateVertexBuffer2(unsigned int uBytes);
D3DIndexBuffer*  __stdcall D3DDevice_CreateIndexBuffer2(unsigned int Length);
void*        __stdcall D3DVertexBuffer_Lock2(D3DVertexBuffer* pBuffer, unsigned int Flags);
void         __stdcall D3DDevice_DrawIndexedVertices(_D3DPRIMITIVETYPE PrimitiveType,
                                                     unsigned int VertexCount,
                                                     const unsigned short* pIndexData);
void         __stdcall D3DDevice_DrawVertices(_D3DPRIMITIVETYPE PrimitiveType,
                                               unsigned int StartVertex,
                                               unsigned int VertexCount);
void         __stdcall D3DDevice_SetVertexShaderInputDirect(void* pVAF, unsigned int StreamCount,
                                                            const _D3DSTREAM_INPUT* pStreamInputs);
void         __stdcall D3DDevice_DrawVerticesUP(_D3DPRIMITIVETYPE PrimitiveType,
                                                unsigned int VertexCount,
                                                const void* pVertexStreamZeroData,
                                                unsigned int VertexStreamZeroStride);
void         __stdcall D3DDevice_LoadVertexShaderProgram(const unsigned int* pFunction,
                                                         unsigned int Address);
void         __stdcall D3DDevice_SelectVertexShaderDirect(_D3DVERTEXATTRIBUTEFORMAT* pVAF,
                                                          unsigned int Address);
void         __stdcall D3DDevice_SetPixelShaderProgram(const _D3DPixelShaderDef* pPSDef);
void         __stdcall D3DDevice_SetRenderState_CullMode(unsigned int Value);
void         __stdcall D3DDevice_SetRenderState_FogColor(unsigned int Value);
void         __stdcall D3DDevice_SetRenderState_ZEnable(unsigned int Value);
void         __stdcall D3DDevice_SetRenderState_ZBias(unsigned int Value);
void         __stdcall D3DDevice_SetRenderState_StencilEnable(unsigned int Value);
void         __stdcall D3DDevice_SetRenderState_RopZCmpAlwaysRead(unsigned int Value);
void         __stdcall D3DDevice_SetViewport(const void* pViewport);
void         __stdcall D3DDevice_Clear(unsigned int Count, const _D3DRECT* pRects,
                                       unsigned int Flags, unsigned int Color,
                                       float Z, unsigned int Stencil);
void         __fastcall D3DDevice_SetRenderState_Simple(unsigned int Method, unsigned int Value);
void         __fastcall D3DDevice_SetVertexShaderConstantNotInlineFast(int Register,
                                                                       const void* pConstantData,
                                                                       unsigned int DwordCount);
int          __stdcall D3DDevice_SetTextureState_ParameterCheck(unsigned int Stage,
                                                                _D3DTEXTURESTAGESTATETYPE Type,
                                                                unsigned int Value);
void         __stdcall D3DDevice_SetRenderState_YuvEnable(unsigned int Value);
void         __stdcall D3DDevice_Swap(unsigned int Flags);
void         __stdcall D3DDevice_BeginScene(void);
void         __stdcall D3DDevice_EndScene(void);
void         __stdcall D3DDevice_BlockUntilIdle(void);
unsigned int __stdcall D3DDevice_InsertFence(void);
void         __stdcall D3DDevice_BlockOnFence(unsigned int Time);
void         __stdcall D3DDevice_InsertCallback(_D3DCALLBACKTYPE Type,
                                                void (*pCallback)(unsigned int),
                                                unsigned int Context);
void         __stdcall D3DDevice_SetVerticalBlankCallback(void (*pCallback)(_D3DVBLANKDATA*));
void         __stdcall D3DDevice_PersistDisplay(void);
int          __stdcall D3DDevice_Reset(_D3DPRESENT_PARAMETERS_* pPresentationParameters);
int          __stdcall D3DDevice_GetDeviceCaps(_D3DCAPS8* pCaps);
void         __stdcall D3DDevice_SetGammaRamp(unsigned int Flags, const _D3DGAMMARAMP* pRamp);
void         __stdcall D3DDevice_SetVertexShader(unsigned int Handle);
void         __stdcall D3DDevice_SetShaderConstantMode(unsigned int Mode);
void         __fastcall D3DDevice_SetVertexShaderConstant1Fast(unsigned int Register,
                                                               const void* pConstantData);
Direct3D*    __stdcall Direct3DCreate8(unsigned int SDKVersion);
unsigned int __stdcall Direct3D_CreateDevice(unsigned int Adapter, _D3DDEVTYPE DeviceType,
                                             void* pUnused, unsigned int Flags,
                                             _D3DPRESENT_PARAMETERS_* pPresentationParameters,
                                             void** ppNewInterface);
void         __stdcall Direct3D_SetPushBufferSize(unsigned int PushBufferSize,
                                                  unsigned int KickOffSize);
int          __stdcall D3DXGetErrorStringA(unsigned int hr, char* pBuffer,
                                           unsigned int cchBuffer);
void         __stdcall D3DDevice_SetIndices(D3DIndexBuffer* pIndexBuffer,
                                            unsigned int BaseVertexIndex);
unsigned int* __stdcall D3DDevice_BeginPush(unsigned int Count);
void         __stdcall D3DDevice_EndPush(unsigned int* p);
}

// ---- D3D8 internal state (XDK segment globals - declared, never implemented).
// Referenced directly by ngl_dx_texture.o's state-cache bypass.
extern unsigned int D3D__DirtyFlags;               // 0xBC2A08
extern unsigned int D3D__TextureState[4][32];      // 0xBC2A10 (base)
extern unsigned int D3D__RenderState[166];         // 0xBC2C10 (IDA XDK cache)
extern unsigned int DTE[4];                        // 0xCD6DE4 (pushbuffer encodes)
extern unsigned int dword_BC2E0C;                  // 0xBC2E0C (presentation-interval cache)
extern unsigned int dword_BC2D10;                  // 0xBC2D10 (ZWRITEENABLE cache)
extern unsigned int dword_BC2D1C;                  // 0xBC2D1C (COLORWRITEENABLE cache)
extern unsigned int dword_BC2C38;                  // 0xBC2C38 (PSCONSTANT0_0 cache)
extern unsigned int dword_BC2C3C;                  // 0xBC2C3C (PSCONSTANT0_1 cache)
extern unsigned int dword_BC2C58;                  // 0xBC2C58 (PSCONSTANT1_0 cache)
extern unsigned int dword_BC2C5C;                  // 0xBC2C5C (PSCONSTANT1_1 cache)
extern unsigned int dword_BC2C98;                  // 0xBC2C98 (PSRGBINPUTS0 cache)
extern unsigned int dword_BC2C9C;                  // 0xBC2C9C (PSRGBINPUTS1 cache)
extern unsigned int dword_BC2C14;                  // 0xBC2C14 (PSALPHAINPUTS1 cache)

// ---- XGRPH entry points (xgraphicsd) --------------------------------------
extern "C" {
int __stdcall XGIsSwizzledFormat(unsigned int Format);
void __stdcall XGSetTextureHeader(unsigned int Width, unsigned int Height,
                                  unsigned int Levels, unsigned int Usage,
                                  unsigned int Format, unsigned int Pool,
                                  D3DBaseTexture* pTexture, void* Data, unsigned int Pitch);
}

#endif // __cplusplus
