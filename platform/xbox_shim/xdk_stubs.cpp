// ============================================================================
// xdk_stubs.cpp - XDK (Xbox) API link stubs (NEVER reconstruct; Phase 6
// backends replace these). Generated from the linker unresolved list.
// NOTE: this toolchain only accepts calling-convention keywords AFTER the
// return type on free functions.
// ============================================================================

#include "d3d8.h"
#include "d3d9_compat.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

// These records keep the D3D8-shaped ABI used by the ported NGL code while
// owning CPU fallback storage and native D3D9 resources for graphics calls.
struct nullD3DInfo {
    unsigned int Magic;
    unsigned int Kind;
    unsigned int Width;
    unsigned int Height;
    unsigned int Depth;
    unsigned int Levels;
    unsigned int Format;
    unsigned int Usage;
    unsigned int Persistent;
    unsigned char* Bits;
    IDirect3DResource9* NativeResource;
    IDirect3DTexture9* NativeTexture;
    IDirect3DSurface9* NativeSurface;
};

static const unsigned int NULL_D3D_MAGIC = 0x4E474C44;
static const unsigned int NULL_D3D_TEXTURE = 1;
static const unsigned int NULL_D3D_SURFACE = 2;
static const unsigned int NULL_D3D_BUFFER = 3;
static const unsigned int NULL_D3DRTYPE_TEXTURE = 3;
static const unsigned int NULL_D3DRTYPE_CUBETEXTURE = 5;

struct nullD3DTexture {
    D3DTexture Object;
    nullD3DInfo Info;
};

struct nullD3DSurface {
    D3DSurface Object;
    nullD3DInfo Info;
};

struct nullD3DBuffer {
    D3DResource Object;
    nullD3DInfo Info;
};

static nullD3DTexture* gNullFrontBuffer = NULL;
static nullD3DTexture* gNullBackBuffer = NULL;
static nullD3DSurface* gNullDepthBuffer = NULL;
static IDirect3D9* gD3D9 = NULL;
static IDirect3DDevice9* gD3D9Device = NULL;
static IDirect3DSurface9* gD3D9RenderTarget = NULL;
static IDirect3DSurface9* gD3D9DepthStencil = NULL;
static HWND gD3D9Window = NULL;
static unsigned int gNullWidth = 640;
static unsigned int gNullHeight = 480;
static void (*gNullVBlankCallback)(_D3DVBLANKDATA*) = NULL;
static unsigned int gNullFence = 0;

static unsigned int nullD3DBytesPerPixel(unsigned int Format) {
    return (Format == D3DFMT_LIN_A8R8G8B8 || Format == D3DFMT_A8R8G8B8 ||
            Format == D3DFMT_LIN_X8R8G8B8 || Format == D3DFMT_X8R8G8B8) ? 4 : 4;
}

static COD3_D3D9_FORMAT nullD3DNativeFormat(unsigned int Format) {
    switch (Format) {
    case D3DFMT_DXT1: return COD3_D3D9_FMT_DXT1;
    case D3DFMT_DXT3: return COD3_D3D9_FMT_DXT3;
    case D3DFMT_DXT5: return COD3_D3D9_FMT_DXT5;
    case D3DFMT_D16:
    case D3DFMT_LIN_D16:
    case D3DFMT_F16:
    case D3DFMT_LIN_F16: return COD3_D3D9_FMT_D16;
    case D3DFMT_D24S8:
    case D3DFMT_LIN_D24S8:
    case D3DFMT_F24S8:
    case D3DFMT_LIN_F24S8: return COD3_D3D9_FMT_D24S8;
    case D3DFMT_R5G6B5:
    case D3DFMT_LIN_R5G6B5: return COD3_D3D9_FMT_R5G6B5;
    case D3DFMT_A1R5G5B5:
    case D3DFMT_LIN_A1R5G5B5: return COD3_D3D9_FMT_A1R5G5B5;
    case D3DFMT_A4R4G4B4:
    case D3DFMT_LIN_A4R4G4B4: return COD3_D3D9_FMT_A4R4G4B4;
    case D3DFMT_L8:
    case D3DFMT_LIN_L8: return COD3_D3D9_FMT_L8;
    case D3DFMT_A8: return COD3_D3D9_FMT_A8;
    case D3DFMT_A8L8:
    case D3DFMT_LIN_A8L8: return COD3_D3D9_FMT_A8L8;
    case D3DFMT_YUY2: return COD3_D3D9_FMT_YUY2;
    case D3DFMT_UYVY: return COD3_D3D9_FMT_UYVY;
    default: return COD3_D3D9_FMT_A8R8G8B8;
    }
}

static nullD3DInfo* nullD3DFindInfo(void* Resource) {
    if (Resource == NULL)
        return NULL;
    nullD3DInfo* Candidate = (nullD3DInfo*)((unsigned char*)Resource + sizeof(D3DResource));
    if (Candidate->Magic == NULL_D3D_MAGIC)
        return Candidate;
    Candidate = (nullD3DInfo*)((unsigned char*)Resource + sizeof(D3DBaseTexture));
    if (Candidate->Magic == NULL_D3D_MAGIC)
        return Candidate;
    Candidate = (nullD3DInfo*)((unsigned char*)Resource + sizeof(D3DSurface));
    if (Candidate->Magic == NULL_D3D_MAGIC)
        return Candidate;
    return NULL;
}

static nullD3DInfo* nullD3DTextureInfo(D3DBaseTexture* Texture) {
    return Texture == NULL ? NULL : &((nullD3DTexture*)Texture)->Info;
}

static nullD3DInfo* nullD3DSurfaceInfo(D3DSurface* Surface) {
    return Surface == NULL ? NULL : &((nullD3DSurface*)Surface)->Info;
}

static nullD3DTexture* nullD3DCreateTexture(unsigned int Width, unsigned int Height,
                                            unsigned int Depth, unsigned int Levels,
                                            unsigned int Usage, unsigned int Format,
                                            unsigned int Type, unsigned int Persistent) {
    nullD3DTexture* Texture = (nullD3DTexture*)calloc(1, sizeof(nullD3DTexture));
    if (Texture == NULL)
        return NULL;
    if (Width == 0) Width = 1;
    if (Height == 0) Height = 1;
    if (Depth == 0) Depth = 1;
    if (Levels == 0) Levels = 1;
    unsigned int Faces = Type == NULL_D3DRTYPE_CUBETEXTURE ? 6 : 1;
    size_t Bytes = (size_t)Width * Height * Depth * Faces * nullD3DBytesPerPixel(Format);
    Texture->Object.Common = 0;
    Texture->Object.Data = (unsigned int)(uintptr_t)calloc(1, Bytes);
    Texture->Object.Lock = 0;
    Texture->Object.Format = Format;
    Texture->Object.Size = (unsigned int)Bytes;
    Texture->Info.Magic = NULL_D3D_MAGIC;
    Texture->Info.Kind = NULL_D3D_TEXTURE;
    Texture->Info.Width = Width;
    Texture->Info.Height = Height;
    Texture->Info.Depth = Depth;
    Texture->Info.Levels = Levels;
    Texture->Info.Format = Format;
    Texture->Info.Usage = Usage;
    Texture->Info.Persistent = Persistent;
    Texture->Info.Bits = (unsigned char*)(uintptr_t)Texture->Object.Data;
    Texture->Info.NativeResource = NULL;
    Texture->Info.NativeTexture = NULL;
    Texture->Info.NativeSurface = NULL;
    if (gD3D9Device != NULL && Type != NULL_D3DRTYPE_CUBETEXTURE && Depth == 1) {
        DWORD NativeUsage = (Usage & 1u) != 0 ? D3DUSAGE_RENDERTARGET : 0;
        D3DPOOL Pool = NativeUsage != 0 ? D3DPOOL_DEFAULT : D3DPOOL_MANAGED;
        if (SUCCEEDED(gD3D9Device->CreateTexture(Width, Height, Levels, NativeUsage,
                                                 nullD3DNativeFormat(Format), Pool,
                                                 &Texture->Info.NativeTexture, NULL))) {
            Texture->Info.NativeResource = Texture->Info.NativeTexture;
            if (NativeUsage != 0)
                Texture->Info.NativeTexture->GetSurfaceLevel(0, &Texture->Info.NativeSurface);
        }
    }
    return Texture;
}

static nullD3DSurface* nullD3DCreateSurface(unsigned int Width, unsigned int Height,
                                            unsigned int Usage, unsigned int Format,
                                            unsigned int Persistent) {
    nullD3DSurface* Surface = (nullD3DSurface*)calloc(1, sizeof(nullD3DSurface));
    if (Surface == NULL)
        return NULL;
    if (Width == 0) Width = 1;
    if (Height == 0) Height = 1;
    unsigned int BytesPerPixel = nullD3DBytesPerPixel(Format);
    size_t Bytes = (size_t)Width * Height * BytesPerPixel;
    Surface->Info.Magic = NULL_D3D_MAGIC;
    Surface->Info.Kind = NULL_D3D_SURFACE;
    Surface->Info.Width = Width;
    Surface->Info.Height = Height;
    Surface->Info.Depth = 1;
    Surface->Info.Levels = 1;
    Surface->Info.Format = Format;
    Surface->Info.Usage = Usage;
    Surface->Info.Persistent = Persistent;
    Surface->Info.Bits = (unsigned char*)calloc(1, Bytes);
    Surface->Info.NativeResource = NULL;
    Surface->Info.NativeTexture = NULL;
    Surface->Info.NativeSurface = NULL;
    if (gD3D9Device != NULL) {
        COD3_D3D9_FORMAT NativeFormat = nullD3DNativeFormat(Format);
        HRESULT Result;
        if ((Usage & 2u) != 0 || NativeFormat == COD3_D3D9_FMT_D16 ||
            NativeFormat == COD3_D3D9_FMT_D24S8) {
            Result = gD3D9Device->CreateDepthStencilSurface(Width, Height, NativeFormat,
                                                             D3DMULTISAMPLE_NONE, 0, FALSE,
                                                             &Surface->Info.NativeSurface, NULL);
        } else {
            Result = gD3D9Device->CreateRenderTarget(Width, Height, NativeFormat,
                                                      D3DMULTISAMPLE_NONE, 0, FALSE,
                                                      &Surface->Info.NativeSurface, NULL);
        }
        if (SUCCEEDED(Result))
            Surface->Info.NativeResource = Surface->Info.NativeSurface;
    }
    return Surface;
}

static HWND nullD3DCreateWindow(void) {
    if (gD3D9Window != NULL)
        return gD3D9Window;
    static const char* ClassName = "COD3D3D9Window";
    WNDCLASSA Class = {};
    Class.lpfnWndProc = DefWindowProcA;
    Class.hInstance = GetModuleHandleA(NULL);
    Class.lpszClassName = ClassName;
    RegisterClassA(&Class);
    gD3D9Window = CreateWindowExA(0, ClassName, "cod3", WS_OVERLAPPED,
                                  0, 0, 1, 1, NULL, NULL, Class.hInstance, NULL);
    return gD3D9Window;
}

static void nullD3DInitDeviceResources(void) {
    if (gNullFrontBuffer != NULL)
        return;
    gNullFrontBuffer = nullD3DCreateTexture(gNullWidth, gNullHeight, 1, 1, 1,
                                            D3DFMT_LIN_A8R8G8B8,
                                            NULL_D3DRTYPE_TEXTURE, 1);
    gNullBackBuffer = nullD3DCreateTexture(gNullWidth, gNullHeight, 1, 1, 1,
                                           D3DFMT_LIN_A8R8G8B8,
                                           NULL_D3DRTYPE_TEXTURE, 1);
    gNullDepthBuffer = nullD3DCreateSurface(gNullWidth, gNullHeight, 2,
                                             D3DFMT_D24S8, 1);
}

extern "C" {

void __fastcall D3DDevice_SetRenderState_Simple(unsigned int, unsigned int) {}
void __fastcall D3DDevice_SetVertexShaderConstant1Fast(unsigned int, const void*) {}
void __fastcall D3DDevice_SetVertexShaderConstantNotInlineFast(int, const void*, unsigned int) {}
void __cdecl compress2(void) {}
unsigned int __stdcall D3DBaseTexture_GetLevelCount(D3DBaseTexture* Texture) {
    nullD3DInfo* Info = nullD3DTextureInfo(Texture);
    return Info != NULL && Info->Magic == NULL_D3D_MAGIC ? Info->Levels : 1;
}
D3DSurface* __stdcall D3DCubeTexture_GetCubeMapSurface2(D3DBaseTexture* Texture,
                                                        _D3DCUBEMAP_FACES, unsigned int) {
    nullD3DInfo* Info = nullD3DTextureInfo(Texture);
    return Info == NULL ? NULL : (D3DSurface*)nullD3DCreateSurface(Info->Width, Info->Height,
                                                                    Info->Usage, Info->Format, 0);
}
unsigned int* __stdcall D3DDevice_BeginPush(unsigned int) { return NULL; }
void __stdcall D3DDevice_BlockOnFence(unsigned int) {}
void __stdcall D3DDevice_BlockUntilIdle(void) {}
void __stdcall D3DDevice_Clear(unsigned int Count, unsigned int ClearFlags,
                               unsigned int Color, unsigned int Stencil, float Z,
                               unsigned int) {
    if (gD3D9Device == NULL)
        return;
    DWORD Flags = 0;
    if ((ClearFlags & 1u) != 0 && gD3D9RenderTarget != NULL)
        Flags |= D3DCLEAR_TARGET;
    if ((ClearFlags & 2u) != 0 && gD3D9DepthStencil != NULL)
        Flags |= D3DCLEAR_ZBUFFER;
    if ((ClearFlags & 4u) != 0 && gD3D9DepthStencil != NULL)
        Flags |= D3DCLEAR_STENCIL;
    if (Flags != 0)
        gD3D9Device->Clear(Count, NULL, Flags, Color, Z, Stencil);
}
D3DIndexBuffer* __stdcall D3DDevice_CreateIndexBuffer2(unsigned int Bytes) {
    nullD3DBuffer* Buffer = (nullD3DBuffer*)calloc(1, sizeof(nullD3DBuffer));
    if (Buffer == NULL) return NULL;
    Buffer->Info.Magic = NULL_D3D_MAGIC;
    Buffer->Info.Kind = NULL_D3D_BUFFER;
    Buffer->Info.Bits = (unsigned char*)calloc(1, Bytes == 0 ? 1 : Bytes);
    Buffer->Object.Data = (unsigned int)(uintptr_t)Buffer->Info.Bits;
    return (D3DIndexBuffer*)Buffer;
}
D3DSurface* __stdcall D3DDevice_CreateSurface2(unsigned int Width, unsigned int Height,
                                                unsigned int Usage, unsigned int Format) {
    return (D3DSurface*)nullD3DCreateSurface(Width, Height, Usage, Format, 0);
}
void* __stdcall D3DDevice_CreateTexture2(unsigned int Width, unsigned int Height,
                                         unsigned int Depth, unsigned int Levels,
                                         unsigned int Usage, unsigned int Format,
                                         unsigned int Type) {
    return nullD3DCreateTexture(Width, Height, Depth, Levels, Usage, Format, Type, 0);
}
D3DVertexBuffer* __stdcall D3DDevice_CreateVertexBuffer2(unsigned int Bytes) {
    return (D3DVertexBuffer*)D3DDevice_CreateIndexBuffer2(Bytes);
}
void __stdcall D3DDevice_DrawIndexedVertices(_D3DPRIMITIVETYPE, unsigned int, const unsigned short*) {}
void __stdcall D3DDevice_DrawVerticesUP(_D3DPRIMITIVETYPE, unsigned int, const void*, unsigned int) {}
void __stdcall D3DDevice_EndPush(unsigned int*) {}
D3DBaseTexture* __stdcall D3DDevice_GetBackBuffer2(int BackBuffer) {
    nullD3DInitDeviceResources();
    return (D3DBaseTexture*)(BackBuffer < 0 ? gNullFrontBuffer : gNullBackBuffer);
}
D3DSurface* __stdcall D3DDevice_GetDepthStencilSurface2(void) {
    nullD3DInitDeviceResources();
    return (D3DSurface*)gNullDepthBuffer;
}
int __stdcall D3DDevice_GetDeviceCaps(_D3DCAPS8* Caps) {
    if (Caps != NULL) {
        memset(Caps, 0, sizeof(*Caps));
        Caps->MaxTextureWidth = 4096;
        Caps->MaxTextureHeight = 4096;
        Caps->MaxStreams = 4;
        Caps->MaxStreamStride = 255;
        Caps->MaxVertexShaderConst = 256;
        Caps->MaxPixelShaderValue = 1.0f;
    }
    return 0;
}
void __stdcall D3DDevice_InsertCallback(_D3DCALLBACKTYPE, void (*)(unsigned int), unsigned int) {}
unsigned int __stdcall D3DDevice_InsertFence(void) { return ++gNullFence; }
void __stdcall D3DDevice_LoadVertexShaderProgram(const unsigned int*, unsigned int) {}
void __stdcall D3DDevice_PersistDisplay(void) {}
int __stdcall D3DDevice_Reset(_D3DPRESENT_PARAMETERS_* Params) {
    if (Params != NULL) {
        gNullWidth = Params->BackBufferWidth;
        gNullHeight = Params->BackBufferHeight;
    }
    return 0;
}
void __stdcall D3DDevice_SetGammaRamp(unsigned int, const _D3DGAMMARAMP*) {}
void __stdcall D3DDevice_SetIndices(D3DIndexBuffer*, unsigned int) {}
void __stdcall D3DDevice_SetPalette(unsigned int, D3DPalette*) {}
void __stdcall D3DDevice_SetRenderState_MultiSampleAntiAlias(unsigned int) {}
void __stdcall D3DDevice_SetRenderState_RopZCmpAlwaysRead(unsigned int) {}
void __stdcall D3DDevice_SetRenderState_StencilEnable(unsigned int) {}
void __stdcall D3DDevice_SetRenderState_YuvEnable(unsigned int) {}
void __stdcall D3DDevice_SetRenderState_ZEnable(unsigned int) {}
void __stdcall D3DDevice_SetRenderTarget(D3DSurface* RenderTarget, D3DSurface* ZBuffer) {
    IDirect3DSurface9* NativeRenderTarget = NULL;
    IDirect3DSurface9* NativeZBuffer = NULL;
    nullD3DInfo* RenderInfo = nullD3DFindInfo(RenderTarget);
    nullD3DInfo* ZInfo = nullD3DFindInfo(ZBuffer);
    if (RenderInfo != NULL)
        NativeRenderTarget = RenderInfo->NativeSurface;
    if (ZInfo != NULL)
        NativeZBuffer = ZInfo->NativeSurface;
    if (gD3D9Device != NULL &&
        (NativeRenderTarget != gD3D9RenderTarget || NativeZBuffer != gD3D9DepthStencil)) {
        gD3D9Device->SetRenderTarget(0, NativeRenderTarget);
        gD3D9Device->SetDepthStencilSurface(NativeZBuffer);
        gD3D9RenderTarget = NativeRenderTarget;
        gD3D9DepthStencil = NativeZBuffer;
    }
}
void __stdcall D3DDevice_SetShaderConstantMode(unsigned int) {}
void __stdcall D3DDevice_SetTexture(unsigned int, D3DBaseTexture*) {}
int __stdcall D3DDevice_SetTextureState_ParameterCheck(unsigned int, _D3DTEXTURESTAGESTATETYPE, unsigned int) { return 0; }
void __stdcall D3DDevice_SetVertexShader(unsigned int) {}
void __stdcall D3DDevice_SetVerticalBlankCallback(void (*Callback)(_D3DVBLANKDATA*)) { gNullVBlankCallback = Callback; }
void __stdcall D3DDevice_SetViewport(const void*) {}
void __stdcall D3DDevice_Swap(unsigned int) {
    if (gD3D9Device != NULL)
        gD3D9Device->Present(NULL, NULL, NULL, NULL);
    if (gNullVBlankCallback != NULL) {
        _D3DVBLANKDATA Data = { 0, 0, 0 };
        gNullVBlankCallback(&Data);
    }
}
void __stdcall D3DDevice_SwitchTexture(unsigned int, unsigned int, unsigned int) {}
unsigned int __stdcall D3DPalette_Lock2(D3DPalette*, unsigned int) { return 0; }
void __stdcall D3DResource_BlockUntilNotBusy(D3DResource*) {}
void __stdcall D3DResource_Register(D3DResource*, void*) {}
int __stdcall D3DSurface_GetDesc(D3DSurface* Surface, _D3DSURFACE_DESC* Desc) {
    nullD3DInfo* Info = nullD3DSurfaceInfo(Surface);
    if (Info == NULL || Desc == NULL) return 0x80004005;
    if (Info->NativeSurface != NULL) {
        COD3_D3D9_SURFACE_DESC NativeDesc = {};
        if (SUCCEEDED(Info->NativeSurface->GetDesc(&NativeDesc))) {
            memset(Desc, 0, sizeof(*Desc));
            Desc->Format = (_D3DFORMAT)Info->Format;
            Desc->Usage = Info->Usage;
            Desc->Width = NativeDesc.Width;
            Desc->Height = NativeDesc.Height;
            return 0;
        }
    }
    memset(Desc, 0, sizeof(*Desc));
    Desc->Format = (_D3DFORMAT)Info->Format;
    Desc->Usage = Info->Usage;
    Desc->Width = Info->Width;
    Desc->Height = Info->Height;
    return 0;
}
void* __stdcall D3DSurface_LockRect(D3DSurface* Surface, D3DLOCKED_RECT* LockedRect,
                                     const void*, unsigned int) {
    nullD3DInfo* Info = nullD3DSurfaceInfo(Surface);
    if (Info == NULL || LockedRect == NULL) return NULL;
    if (Info->NativeSurface != NULL) {
        COD3_D3D9_LOCKED_RECT NativeRect = {};
        if (SUCCEEDED(Info->NativeSurface->LockRect(&NativeRect, NULL, 0))) {
            LockedRect->Pitch = NativeRect.Pitch;
            LockedRect->pBits = NativeRect.pBits;
            return NativeRect.pBits;
        }
    }
    LockedRect->Pitch = (int)(Info->Width * nullD3DBytesPerPixel(Info->Format));
    LockedRect->pBits = Info->Bits;
    return Info->Bits;
}
int __stdcall D3DTexture_GetLevelDesc(D3DBaseTexture* Texture, unsigned int,
                                      _D3DSURFACE_DESC* Desc) {
    nullD3DInfo* Info = nullD3DTextureInfo(Texture);
    if (Info == NULL || Desc == NULL) return 0x80004005;
    if (Info->NativeTexture != NULL) {
        COD3_D3D9_SURFACE_DESC NativeDesc = {};
        if (SUCCEEDED(Info->NativeTexture->GetLevelDesc(0, &NativeDesc))) {
            memset(Desc, 0, sizeof(*Desc));
            Desc->Format = (_D3DFORMAT)Info->Format;
            Desc->Usage = Info->Usage;
            Desc->Width = NativeDesc.Width;
            Desc->Height = NativeDesc.Height;
            return 0;
        }
    }
    memset(Desc, 0, sizeof(*Desc));
    Desc->Format = (_D3DFORMAT)Info->Format;
    Desc->Width = Info->Width;
    Desc->Height = Info->Height;
    Desc->Usage = Info->Usage;
    return 0;
}
D3DSurface* __stdcall D3DTexture_GetSurfaceLevel2(D3DBaseTexture* Texture, unsigned int) {
    nullD3DInfo* Info = nullD3DTextureInfo(Texture);
    if (Info == NULL)
        return NULL;
    nullD3DSurface* Surface = nullD3DCreateSurface(Info->Width, Info->Height,
                                                    Info->Usage, Info->Format, 0);
    if (Surface != NULL && Info->NativeTexture != NULL) {
        if (Surface->Info.NativeSurface != NULL) {
            Surface->Info.NativeSurface->Release();
            Surface->Info.NativeSurface = NULL;
        }
        if (SUCCEEDED(Info->NativeTexture->GetSurfaceLevel(0, &Surface->Info.NativeSurface)))
            Surface->Info.NativeResource = Surface->Info.NativeSurface;
    }
    return (D3DSurface*)Surface;
}
void* __stdcall D3DTexture_LockRect(D3DTexture* Texture, unsigned int, D3DLOCKED_RECT* LockedRect,
                                    const void*, unsigned int) {
    nullD3DInfo* Info = nullD3DTextureInfo((D3DBaseTexture*)Texture);
    if (Info == NULL || LockedRect == NULL) return NULL;
    if (Info->NativeTexture != NULL) {
        COD3_D3D9_LOCKED_RECT NativeRect = {};
        if (SUCCEEDED(Info->NativeTexture->LockRect(0, &NativeRect, NULL, 0))) {
            LockedRect->Pitch = NativeRect.Pitch;
            LockedRect->pBits = NativeRect.pBits;
            return NativeRect.pBits;
        }
    }
    LockedRect->Pitch = (int)(Info->Width * nullD3DBytesPerPixel(Info->Format));
    LockedRect->pBits = Info->Bits;
    return Info->Bits;
}
void* __stdcall D3DVertexBuffer_Lock2(D3DVertexBuffer* Buffer, unsigned int) {
    return Buffer == NULL ? NULL : (void*)(uintptr_t)Buffer->Data;
}
int __stdcall D3DXCreateTextureFromFileInMemoryEx(void*, const void*, unsigned int,
                                                  unsigned int Width, unsigned int Height,
                                                  unsigned int Levels, unsigned int Usage,
                                                  _D3DFORMAT Format, unsigned int, unsigned int,
                                                  unsigned int, unsigned int, void*, void*,
                                                  D3DTexture** Texture) {
    if (Width == 0xFFFFFFFF) Width = 1;
    if (Height == 0xFFFFFFFF) Height = 1;
    if (Levels == 0xFFFFFFFF) Levels = 1;
    if (Format == D3DFMT_UNKNOWN) Format = D3DFMT_LIN_A8R8G8B8;
    if (Texture != NULL)
        *Texture = (D3DTexture*)nullD3DCreateTexture(Width, Height, 1, Levels, Usage,
                                                      Format, NULL_D3DRTYPE_TEXTURE, 0);
    return 0;
}
int __stdcall D3DXGetErrorStringA(unsigned int, char* Buffer, unsigned int Length) {
    if (Buffer != NULL && Length != 0) {
        strncpy_s(Buffer, Length, "null renderer", _TRUNCATE);
    }
    return 0;
}
unsigned int __stdcall Direct3D_CreateDevice(unsigned int, _D3DDEVTYPE,
                                             void*, unsigned int,
                                             _D3DPRESENT_PARAMETERS_* Params,
                                             void** Device) {
    if (Params != NULL) {
        gNullWidth = Params->BackBufferWidth;
        gNullHeight = Params->BackBufferHeight;
    }
    if (gD3D9Device == NULL) {
        gD3D9 = Direct3DCreate9(D3D_SDK_VERSION);
        if (gD3D9 != NULL) {
        COD3_D3D9_PRESENT_PARAMETERS NativeParams = {};
            NativeParams.BackBufferWidth = gNullWidth;
            NativeParams.BackBufferHeight = gNullHeight;
            NativeParams.BackBufferFormat = COD3_D3D9_FMT_X8R8G8B8;
            NativeParams.BackBufferCount = 1;
            NativeParams.SwapEffect = COD3_D3D9_SWP_DISCARD;
            NativeParams.hDeviceWindow = nullD3DCreateWindow();
            NativeParams.Windowed = TRUE;
            NativeParams.EnableAutoDepthStencil = TRUE;
            NativeParams.AutoDepthStencilFormat = COD3_D3D9_FMT_D24S8;
            NativeParams.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
            HRESULT Result = gD3D9->CreateDevice(D3DADAPTER_DEFAULT, COD3_D3D9_DEVTYPE_HAL,
                                                  NativeParams.hDeviceWindow,
                                                  D3DCREATE_SOFTWARE_VERTEXPROCESSING,
                                                  &NativeParams, &gD3D9Device);
            if (FAILED(Result)) {
                gD3D9->Release();
                gD3D9 = NULL;
            }
        }
    }
    nullD3DInitDeviceResources();
    if (gD3D9Device != NULL) {
        gD3D9Device->GetRenderTarget(0, &gD3D9RenderTarget);
        gD3D9Device->GetDepthStencilSurface(&gD3D9DepthStencil);
    }
    if (Device != NULL) *Device = (void*)1;
    return 0;
}
void __stdcall Direct3D_SetPushBufferSize(unsigned int, unsigned int) {}
Direct3D* __stdcall Direct3DCreate8(unsigned int) { return (Direct3D*)1; }
void __stdcall LiveEngine_DoWork(unsigned int a0, unsigned int a1) {}
void __stdcall LiveEngine_EnableFeature(unsigned int a0, unsigned int a1) {}
void __stdcall LiveEngine_EndFeature(unsigned int a0) {}
void __stdcall LiveEngine_GetExitInfo(unsigned int a0, unsigned int a1) {}
void* __stdcall LiveEngine_GetFeatureInterface(unsigned int a0, unsigned int a1, unsigned int a2, unsigned int a3) { return nullptr; }
void __stdcall LiveEngine_GetNotifications(unsigned int a0, unsigned int a1, unsigned int a2, unsigned int a3) {}
void __stdcall LiveEngine_LogOff(unsigned int a0) {}
void __stdcall LiveEngine_NotificationSetState(unsigned int a0, unsigned int a1, unsigned int a2, unsigned int a3, unsigned int a4, unsigned int a5, unsigned int a6) {}
void __stdcall LiveEngine_Reboot(unsigned int a0, unsigned int a1) {}
void __stdcall LiveEngine_Release(unsigned int a0) {}
void __stdcall LiveEngine_Render(unsigned int a0, unsigned int a1) {}
void __stdcall LiveEngine_SetInput(unsigned int a0, unsigned int a1, unsigned int a2) {}
void __stdcall LiveEngine_SetProperty(unsigned int a0, unsigned int a1, unsigned int a2) {}
void __stdcall LiveEngine_SetUIPlugin(unsigned int a0, unsigned int a1) {}
void __stdcall LiveEngine_StartFeature(unsigned int a0, unsigned int a1, unsigned int a2) {}
void __stdcall LiveEngine_UseVoiceMail(unsigned int a0, unsigned int a1) {}
void* __stdcall UIXCreateLiveEngine(unsigned int a0, unsigned int a1, unsigned int a2) { return nullptr; }
void* __stdcall UIXCreateUIPlugin(unsigned int a0, unsigned int a1) { return nullptr; }
void __cdecl uncompress(void) {}
unsigned int __stdcall XGetLanguage(void) { return 1; }
unsigned int __cdecl XGetVideoFlags(void) { return 0; }
int __stdcall XGIsSwizzledFormat(unsigned int) { return 0; }
void __stdcall XGSetPaletteHeader(_D3DPALETTESIZE, D3DPalette* Palette, void* Data) {
    if (Palette != NULL) Palette->Data = (unsigned int)(uintptr_t)Data;
}
void __stdcall XGSetTextureHeader(unsigned int Width, unsigned int Height, unsigned int Levels,
                                  unsigned int Usage, unsigned int Format, unsigned int,
                                  D3DBaseTexture* Texture, void* Data, unsigned int) {
    if (Texture != NULL) {
        Texture->Format = Format;
        Texture->Size = Width * Height;
        Texture->Data = (unsigned int)(uintptr_t)Data;
    }
    (void)Levels;
    (void)Usage;
}
void __stdcall XGSwizzleRect(const void* Source, unsigned int Pitch, const void*, void* Dest,
                             unsigned int Width, unsigned int Height, const void*, unsigned int BytesPerPixel) {
    if (Source == NULL || Dest == NULL) return;
    unsigned int RowBytes = Width * BytesPerPixel;
    for (unsigned int y = 0; y < Height; ++y)
        memcpy((unsigned char*)Dest + y * RowBytes, (const unsigned char*)Source + y * Pitch, RowBytes);
}
void __stdcall XGWriteSurfaceToFile(D3DSurface*, const char*) {}
void __stdcall XHVEngine_DoWork(unsigned int a0) {}
void __stdcall XHVEngine_EnableProcessingMode(unsigned int a0, unsigned int a1) {}
void __stdcall XHVEngine_IsTalking(unsigned int a0, unsigned int a1, unsigned int a2, unsigned int a3) {}
void __stdcall XHVEngine_RegisterLocalTalker(unsigned int a0, unsigned int a1) {}
void __stdcall XHVEngine_RegisterRemoteTalker(unsigned int a0, unsigned int a1, unsigned int a2, unsigned int a3) {}
void __stdcall XHVEngine_Release(unsigned int a0) {}
void __stdcall XHVEngine_SetCallbackInterface(unsigned int a0, unsigned int a1) {}
void __stdcall XHVEngine_SetMixBinMapping(unsigned int a0, unsigned int a1, unsigned int a2, unsigned int a3, unsigned int a4, unsigned int a5) {}
void __stdcall XHVEngine_SetPlaybackPriority(unsigned int a0, unsigned int a1, unsigned int a2, unsigned int a3, unsigned int a4, unsigned int a5) {}
void __stdcall XHVEngine_SetProcessingMode(unsigned int a0, unsigned int a1, unsigned int a2) {}
void __stdcall XHVEngine_SubmitIncomingVoicePacket(unsigned int a0, unsigned int a1, unsigned int a2, unsigned int a3, unsigned int a4, unsigned int a5) {}
void __stdcall XHVEngine_UnregisterRemoteTalker(unsigned int a0, unsigned int a1, unsigned int a2, unsigned int a3) {}
void* __stdcall XHVEngineCreate(unsigned int a0, unsigned int a1) { return nullptr; }
void __stdcall XInputGetState(unsigned int a0, unsigned int a1) {}
void __stdcall XInputSetState(unsigned int a0, unsigned int a1) {}
int __stdcall XNetQosListen(unsigned int a0, unsigned int a1, unsigned int a2, unsigned int a3, unsigned int a4) { return 0; }
void __stdcall XNetRegisterKey(unsigned int a0, unsigned int a1) {}
void __stdcall XNetUnregisterKey(unsigned int a0) {}
unsigned int __stdcall XOnlineGetLogonUsers(void) { return 0; }
void __stdcall XOnlineMatchSessionCreate(unsigned int a0, unsigned int a1, unsigned int a2, unsigned int a3, unsigned int a4, unsigned int a5, unsigned int a6, unsigned int a7) {}
void __stdcall XOnlineMatchSessionDelete(unsigned int a0, unsigned int a1, unsigned int a2, unsigned int a3) {}
void __stdcall XOnlineMatchSessionGetInfo(unsigned int a0, unsigned int a1, unsigned int a2) {}
void __stdcall XOnlineMatchSessionUpdate(unsigned int a0, unsigned int a1, unsigned int a2, unsigned int a3, unsigned int a4, unsigned int a5, unsigned int a6, unsigned int a7, unsigned int a8, unsigned int a9) {}
void __stdcall XOnlineMutelistGet(unsigned int a0, unsigned int a1, unsigned int a2, unsigned int a3, unsigned int a4, unsigned int a5) {}
void __stdcall XOnlineSaveLogonState(unsigned int a0) {}
void __stdcall XOnlineStartup(unsigned int a0) {}
void __stdcall XOnlineTaskClose(unsigned int a0) {}
void __stdcall XOnlineTaskContinue(unsigned int a0) {}

}  // extern "C"

// LNK2001-only imports surfaced after the first pass
extern "C" {
void __stdcall D3DDevice_SelectVertexShaderDirect(_D3DVERTEXATTRIBUTEFORMAT*, unsigned int) {}
void __stdcall D3DDevice_SetPixelShaderProgram(const _D3DPixelShaderDef*) {}
void __stdcall D3DDevice_SetRenderState_CullMode(unsigned int) {}
int __stdcall D3DDevice_SetRenderState_ParameterCheck(unsigned int, unsigned int) { return 0; }
void __stdcall D3DDevice_SetVertexShaderInputDirect(void*, unsigned int, const _D3DSTREAM_INPUT*) {}
unsigned int __stdcall D3DResource_Release(D3DResource* Resource) {
    if (Resource == NULL)
        return 0;
    if (Resource == (D3DResource*)gNullFrontBuffer ||
        Resource == (D3DResource*)gNullBackBuffer ||
        Resource == (D3DResource*)gNullDepthBuffer)
        return 0;
    nullD3DInfo* Info = NULL;
    nullD3DInfo* Candidate = (nullD3DInfo*)((unsigned char*)Resource + sizeof(D3DBaseTexture));
    if (Candidate->Magic == NULL_D3D_MAGIC)
        Info = Candidate;
    Candidate = (nullD3DInfo*)((unsigned char*)Resource + sizeof(D3DResource));
    if (Info == NULL && Candidate->Magic == NULL_D3D_MAGIC)
        Info = Candidate;
    Candidate = (nullD3DInfo*)((unsigned char*)Resource + sizeof(D3DSurface));
    if (Info == NULL && Candidate->Magic == NULL_D3D_MAGIC)
        Info = Candidate;
    if (Info != NULL) {
        if (Info->NativeSurface != NULL) {
            Info->NativeSurface->Release();
            Info->NativeSurface = NULL;
        }
        if (Info->NativeTexture != NULL) {
            Info->NativeTexture->Release();
            Info->NativeTexture = NULL;
        }
        Info->NativeResource = NULL;
        free(Info->Bits);
        free(Resource);
    }
    return 0;
}
int __cdecl __fpclass(double a0) { (void)a0; return 0; }
}
