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
#include <stdint.h>

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

struct D3DBaseTexture : D3DResource {
    unsigned int Format;  // +0x0C
    unsigned int Size;    // +0x10
};
static_assert(sizeof(D3DBaseTexture) == 0x14, "D3DBaseTexture size mismatch");
struct D3DTexture : D3DBaseTexture {};
struct D3DCubeTexture : D3DBaseTexture {};
struct D3DVolumeTexture : D3DBaseTexture {};
struct D3DSurface {};

// ---- Locked rect (Xbox D3D8, 8 bytes, verified against IDA) ---------------
struct _D3DLOCKED_RECT {
    int  Pitch;   // +0x00
    void* pBits;  // +0x04
};
static_assert(sizeof(_D3DLOCKED_RECT) == 8, "_D3DLOCKED_RECT size mismatch");
typedef _D3DLOCKED_RECT D3DLOCKED_RECT;

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
    D3DRS_ALPHABLENDENABLE = 59,   // 0x3B
    D3DRS_CULLMODE = 147,          // 0x93
    D3DRS_MULTISAMPLEANTIALIAS = 152,  // 0x98
    D3DRS_SIMPLE_MAX = 92,         // 0x5C
    D3DRS_YUVENABLE = 160,         // 0xA0
};

// ---- Pixel format selector (D3DFMT_*) ------------------------------------
enum _D3DFORMAT {
    D3DFMT_YUY2 = 36,
};

// ---- Pixel shader definition (opaque, 240 bytes) -------------------------
struct _D3DPixelShaderDef {
    uint8_t data[240];
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
void         __fastcall D3DDevice_SetRenderState_Simple(unsigned int Method, unsigned int Value);
void         __fastcall D3DDevice_SetVertexShaderConstantNotInlineFast(int Register,
                                                                       const void* pConstantData,
                                                                       unsigned int DwordCount);
int          __stdcall D3DDevice_SetTextureState_ParameterCheck(unsigned int Stage,
                                                                _D3DTEXTURESTAGESTATETYPE Type,
                                                                unsigned int Value);
void         __stdcall D3DDevice_SetRenderState_YuvEnable(unsigned int Value);
void         __stdcall D3DDevice_SetIndices(D3DIndexBuffer* pIndexBuffer,
                                            unsigned int BaseVertexIndex);
unsigned int* __stdcall D3DDevice_BeginPush(unsigned int Count);
void         __stdcall D3DDevice_EndPush(unsigned int* p);
}

// ---- D3D8 internal state (XDK segment globals - declared, never implemented).
// Referenced directly by ngl_dx_texture.o's state-cache bypass.
extern unsigned int D3D__DirtyFlags;               // 0xBC2A08
extern unsigned int D3D__TextureState[4][32];      // 0xBC2A10 (base)
extern unsigned int DTE[4];                        // 0xCD6DE4 (pushbuffer encodes)

// ---- XGRPH entry points (xgraphicsd) --------------------------------------
extern "C" {
int __stdcall XGIsSwizzledFormat(unsigned int Format);
}

#endif // __cplusplus
