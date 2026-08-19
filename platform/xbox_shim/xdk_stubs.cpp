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
    unsigned int SizeBytes;
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

// APK textures retain their Xbox D3D object in the serialized image section.
// Keep host state beside those objects instead of changing the 20-byte XDK
// resource layout.  The table is deliberately bounded like the fixed-size
// resource tables used by the original frontend path.
struct nullD3DExternalTexture {
    D3DBaseTexture* Object;
    unsigned int PackedFormat;
    unsigned int PackedSize;
    nullD3DInfo Info;
};

static nullD3DExternalTexture gNullExternalTextures[256] = {};

static nullD3DTexture* gNullFrontBuffer = NULL;
static nullD3DTexture* gNullBackBuffer = NULL;
static nullD3DSurface* gNullDepthBuffer = NULL;
static IDirect3D9* gD3D9 = NULL;
static IDirect3DDevice9* gD3D9Device = NULL;
static IDirect3DSurface9* gD3D9RenderTarget = NULL;
static IDirect3DSurface9* gD3D9DepthStencil = NULL;
static IDirect3DVertexDeclaration9* gD3D9VertexDeclaration = NULL;
static IDirect3DIndexBuffer9* gD3D9IndexBuffer = NULL;
static nullD3DInfo* gD3D9IndexInfo = NULL;
static IDirect3DVertexBuffer9* gD3D9VertexBuffers[4] = {};
static nullD3DInfo* gD3D9VertexInfos[4] = {};
static unsigned int gD3D9VertexOffsets[4] = {};
static unsigned int gD3D9VertexStrides[4] = {};
static unsigned int gNullTextureWidths[4] = {};
static unsigned int gNullTextureHeights[4] = {};
static HWND gD3D9Window = NULL;
static unsigned int gNullWidth = 640;
static unsigned int gNullHeight = 480;
static void (*gNullVBlankCallback)(_D3DVBLANKDATA*) = NULL;
static unsigned int gNullFence = 0;
// NGL's ported quad/filter/font paths still write Xbox push packets before
// EndPush, so provide writable host storage for the D3D9 translation path.
static unsigned int* gNullPushBuffer = NULL;

static nullD3DInfo* nullD3DAdoptExternalTexture(D3DBaseTexture* Texture);
int __stdcall XGIsSwizzledFormat(unsigned int Format);

// NGL's Xbox render-state method cells are owned by the Win32 shim.  The
// source-side state code already identifies the slots that are used by the
// D3D9 path; bind those slots to their native render-state selectors once the
// device is created.
extern unsigned int dword_40300;
extern unsigned int dword_40304;
extern unsigned int dword_4033C;
extern unsigned int dword_40340;
extern unsigned int dword_40344;
extern unsigned int dword_40348;
extern unsigned int dword_4034C;
extern unsigned int dword_40350;
extern unsigned int dword_40354;
extern unsigned int dword_40358;
extern unsigned int dword_4035C;

static void nullD3DInitStateAliases() {
    dword_40300 = COD3_D3D9_RS_ALPHATESTENABLE;
    dword_40304 = COD3_D3D9_RS_ALPHABLENDENABLE;
    dword_4033C = COD3_D3D9_RS_ALPHAFUNC;
    dword_40340 = COD3_D3D9_RS_ALPHAREF;
    dword_40344 = COD3_D3D9_RS_SRCBLEND;
    dword_40348 = COD3_D3D9_RS_DESTBLEND;
    dword_4034C = D3DRS_BLENDFACTOR;
    dword_40350 = COD3_D3D9_RS_BLENDOP;
    dword_40354 = 23u; // D3DRS_ZFUNC in the native D3D9 enum.
    dword_40358 = COD3_D3D9_RS_COLORWRITEENABLE;
    dword_4035C = COD3_D3D9_RS_ZWRITEENABLE;
}

static DWORD nullD3DCompareFunc(unsigned int Value) {
    // Xbox D3D encodes compare functions as 0x200 + the eight D3D compare
    // values.  NGL emits these values directly in its render-state path.
    switch (Value) {
    case 0x200u: return D3DCMP_NEVER;
    case 0x201u: return D3DCMP_LESS;
    case 0x202u: return D3DCMP_EQUAL;
    case 0x203u: return D3DCMP_LESSEQUAL;
    case 0x204u: return D3DCMP_GREATER;
    case 0x205u: return D3DCMP_NOTEQUAL;
    case 0x206u: return D3DCMP_GREATEREQUAL;
    case 0x207u: return D3DCMP_ALWAYS;
    default: return Value;
    }
}

static DWORD nullD3DCullMode(unsigned int Value) {
    // The NGL paths use 0 for no culling and 0x900 for the Xbox back-face
    // selector.  Accept the native D3D8 numeric selectors as well.
    switch (Value) {
    case 0u: return D3DCULL_NONE;
    case 0x900u: return D3DCULL_CCW;
    case 1u: return D3DCULL_CW;
    case 2u:
    case 3u: return D3DCULL_CCW;
    default: return D3DCULL_NONE;
    }
}

static unsigned int nullD3DBytesPerPixel(unsigned int Format) {
    return (Format == D3DFMT_LIN_A8R8G8B8 || Format == D3DFMT_A8R8G8B8 ||
            Format == D3DFMT_LIN_X8R8G8B8 || Format == D3DFMT_X8R8G8B8) ? 4 : 4;
}

static COD3_D3D9_PRIMITIVETYPE nullD3DPrimitiveType(_D3DPRIMITIVETYPE Type) {
    switch (Type) {
    case D3DPT_POINTLIST: return COD3_D3D9_PT_POINTLIST;
    case D3DPT_LINELIST: return COD3_D3D9_PT_LINELIST;
    case D3DPT_LINESTRIP: return COD3_D3D9_PT_LINESTRIP;
    case D3DPT_TRIANGLELIST: return COD3_D3D9_PT_TRIANGLELIST;
    case D3DPT_TRIANGLESTRIP: return COD3_D3D9_PT_TRIANGLESTRIP;
    case D3DPT_TRIANGLEFAN: return COD3_D3D9_PT_TRIANGLEFAN;
    default: return COD3_D3D9_PT_FORCE_DWORD;
    }
}

static unsigned int nullD3DPrimitiveCount(_D3DPRIMITIVETYPE Type, unsigned int VertexCount) {
    switch (Type) {
    case D3DPT_POINTLIST: return VertexCount;
    case D3DPT_LINELIST: return VertexCount / 2;
    case D3DPT_LINESTRIP: return VertexCount > 1 ? VertexCount - 1 : 0;
    case D3DPT_TRIANGLELIST: return VertexCount / 3;
    case D3DPT_TRIANGLESTRIP:
    case D3DPT_TRIANGLEFAN: return VertexCount > 2 ? VertexCount - 2 : 0;
    default: return 0;
    }
}

static bool nullD3DBuildVertexDeclaration(_D3DVERTEXATTRIBUTEFORMAT* Format) {
    if (gD3D9Device == NULL || Format == NULL)
        return false;
    D3DVERTEXELEMENT9 Elements[17] = {};
    unsigned int Count = 0;
    for (unsigned int i = 0; i < 16 && Format->Input[i].Format != 2; ++i) {
        const _D3DVERTEXSHADERINPUT& Input = Format->Input[i];
        D3DVERTEXELEMENT9& Element = Elements[Count];
        Element.Stream = 0;
        Element.Offset = (WORD)Input.Offset;
        Element.Method = D3DDECLMETHOD_DEFAULT;
        Element.UsageIndex = (BYTE)(Input.StreamIndex);
        if (Input.Format == 50) {
            Element.Type = D3DDECLTYPE_FLOAT3;
            Element.Usage = D3DDECLUSAGE_POSITION;
        } else if (Input.Format == 64) {
            Element.Type = D3DDECLTYPE_D3DCOLOR;
            Element.Usage = D3DDECLUSAGE_COLOR;
            Element.UsageIndex = 0;
        } else if (Input.Format == 34) {
            Element.Type = D3DDECLTYPE_FLOAT2;
            Element.Usage = D3DDECLUSAGE_TEXCOORD;
        } else {
            return false;
        }
        ++Count;
    }
    Elements[Count] = D3DDECL_END();
    IDirect3DVertexDeclaration9* Declaration = NULL;
    if (FAILED(gD3D9Device->CreateVertexDeclaration(Elements, &Declaration)))
        return false;
    if (gD3D9VertexDeclaration != NULL)
        gD3D9VertexDeclaration->Release();
    gD3D9VertexDeclaration = Declaration;
    return true;
}

static void nullD3DSyncVertexBuffer(IDirect3DVertexBuffer9* Native, nullD3DInfo* Info) {
    if (Native == NULL || Info == NULL || Info->Bits == NULL || Info->SizeBytes == 0)
        return;
    void* Destination = NULL;
    if (SUCCEEDED(Native->Lock(0, Info->SizeBytes, &Destination, 0))) {
        memcpy(Destination, Info->Bits, Info->SizeBytes);
        Native->Unlock();
    }
}

static void nullD3DSyncIndexBuffer(IDirect3DIndexBuffer9* Native, nullD3DInfo* Info) {
    if (Native == NULL || Info == NULL || Info->Bits == NULL || Info->SizeBytes == 0)
        return;
    void* Destination = NULL;
    if (SUCCEEDED(Native->Lock(0, Info->SizeBytes, &Destination, 0))) {
        memcpy(Destination, Info->Bits, Info->SizeBytes);
        Native->Unlock();
    }
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
    if (Texture == NULL)
        return NULL;
    for (unsigned int i = 0; i < sizeof(gNullExternalTextures) / sizeof(gNullExternalTextures[0]); ++i) {
        if (gNullExternalTextures[i].Object == Texture)
            return &gNullExternalTextures[i].Info;
    }
    return &((nullD3DTexture*)Texture)->Info;
}

static nullD3DExternalTexture* nullD3DFindExternalTexture(D3DBaseTexture* Texture) {
    if (Texture == NULL)
        return NULL;
    for (unsigned int i = 0; i < sizeof(gNullExternalTextures) / sizeof(gNullExternalTextures[0]); ++i) {
        if (gNullExternalTextures[i].Object == Texture)
            return &gNullExternalTextures[i];
    }
    return NULL;
}

static unsigned int nullD3DExternalTextureWidth(unsigned int Format, unsigned int Size) {
    if (Size != 0)
        return (Size & 0xFFFu) + 1;
    unsigned int LogWidth = (Format >> 20) & 0xFu;
    return 1u << LogWidth;
}

static unsigned int nullD3DExternalTextureHeight(unsigned int Format, unsigned int Size) {
    if (Size != 0)
        return ((Size >> 12) & 0xFFFu) + 1;
    unsigned int LogHeight = (Format >> 24) & 0xFu;
    return 1u << LogHeight;
}

static unsigned int nullD3DExternalTextureLevels(unsigned int Format) {
    unsigned int Levels = (Format >> 16) & 0xFu;
    return Levels == 0 ? 1 : Levels;
}

static unsigned int nullD3DLinearBytesPerPixel(unsigned int Format) {
    switch (Format) {
    case D3DFMT_LIN_L8:
    case D3DFMT_LIN_A8:
        return 1;
    case D3DFMT_LIN_A8L8:
    case D3DFMT_LIN_R5G6B5:
    case D3DFMT_LIN_A1R5G5B5:
    case D3DFMT_LIN_X1R5G5B5:
    case D3DFMT_LIN_A4R4G4B4:
        return 2;
    case D3DFMT_LIN_A8R8G8B8:
    case D3DFMT_LIN_X8R8G8B8:
    case D3DFMT_LIN_A8B8G8R8:
    case D3DFMT_LIN_B8G8R8A8:
    case D3DFMT_LIN_R8G8B8A8:
        return 4;
    default:
        return 0;
    }
}

static unsigned int nullD3DFormatBytesPerPixel(unsigned int Format) {
    unsigned int BytesPerPixel = nullD3DLinearBytesPerPixel(Format);
    if (BytesPerPixel != 0)
        return BytesPerPixel;
    switch (Format) {
    case D3DFMT_A8R8G8B8:
    case D3DFMT_X8R8G8B8:
    case D3DFMT_A8B8G8R8:
    case D3DFMT_B8G8R8A8:
    case D3DFMT_R8G8B8A8:
        return 4;
    case D3DFMT_R5G6B5:
    case D3DFMT_A1R5G5B5:
    case D3DFMT_X1R5G5B5:
    case D3DFMT_A4R4G4B4:
    case D3DFMT_A8L8:
        return 2;
    case D3DFMT_L8:
    case D3DFMT_A8:
    case D3DFMT_P8:
        return 1;
    default:
        return 0;
    }
}

static unsigned int nullD3DLog2(unsigned int Value) {
    unsigned int Result = 0;
    while (Value > 1) {
        Value >>= 1;
        ++Result;
    }
    return Result;
}

static void nullD3DGetSwizzleMasks(unsigned int Width, unsigned int Height,
                                   unsigned int* MaskU, unsigned int* MaskV) {
    // XGRAPHICS::GetMasks2, translated from the release XBE decompile.
    unsigned int LogWidth = nullD3DLog2(Width);
    unsigned int LogHeight = nullD3DLog2(Height);
    unsigned int MinLog = LogWidth < LogHeight ? LogWidth : LogHeight;
    unsigned int LowMask = (1u << (2 * MinLog)) - 1u;
    unsigned int HighMask = ~LowMask;
    unsigned int U = LogWidth <= LogHeight ? (LowMask & 0x55555555u)
                                           : (HighMask | 0x55555555u);
    unsigned int V = LogWidth >= LogHeight ? (LowMask & 0xAAAAAAAAu)
                                           : (HighMask | 0xAAAAAAAAu);
    unsigned int CombinedMask = (1u << (LogHeight + LogWidth)) - 1u;
    *MaskU = U & CombinedMask;
    *MaskV = V & CombinedMask;
}

static unsigned int nullD3DSwizzleCoordinate(unsigned int Mask, unsigned int Value) {
    // Swizzler::SwizzleU/SwizzleV, translated from the release XBE decompile.
    unsigned int Result = 0;
    for (unsigned int Bit = 1; Bit <= Mask; Bit <<= 1) {
        if ((Mask & Bit) != 0)
            Result |= Value & Bit;
        else
            Value <<= 1;
    }
    return Result;
}

static void nullD3DUnswizzleBytes(const unsigned char* Source, unsigned char* Dest,
                                  unsigned int Width, unsigned int Height,
                                  unsigned int BytesPerPixel) {
    unsigned int MaskU = 0;
    unsigned int MaskV = 0;
    nullD3DGetSwizzleMasks(Width, Height, &MaskU, &MaskV);
    for (unsigned int y = 0; y < Height; ++y) {
        unsigned int V = nullD3DSwizzleCoordinate(MaskV, y);
        for (unsigned int x = 0; x < Width; ++x) {
            unsigned int U = nullD3DSwizzleCoordinate(MaskU, x);
            size_t SourceOffset = (size_t)(U | V) * BytesPerPixel;
            size_t DestinationOffset = ((size_t)y * Width + x) * BytesPerPixel;
            memcpy(Dest + DestinationOffset, Source + SourceOffset, BytesPerPixel);
        }
    }
}

static void nullD3DSwizzleBytes(const unsigned char* Source, unsigned int SourcePitch,
                                unsigned char* Dest, unsigned int Width,
                                unsigned int Height, unsigned int BytesPerPixel) {
    unsigned int MaskU = 0;
    unsigned int MaskV = 0;
    nullD3DGetSwizzleMasks(Width, Height, &MaskU, &MaskV);
    for (unsigned int y = 0; y < Height; ++y) {
        unsigned int V = nullD3DSwizzleCoordinate(MaskV, y);
        for (unsigned int x = 0; x < Width; ++x) {
            unsigned int U = nullD3DSwizzleCoordinate(MaskU, x);
            size_t SourceOffset = (size_t)y * SourcePitch +
                                  (size_t)x * BytesPerPixel;
            size_t DestinationOffset = (size_t)(U | V) * BytesPerPixel;
            memcpy(Dest + DestinationOffset, Source + SourceOffset, BytesPerPixel);
        }
    }
}

static void nullD3DUnswizzle32(const unsigned char* Source, unsigned int* Dest,
                               unsigned int Width, unsigned int Height) {
    unsigned int MaskU = 0;
    unsigned int MaskV = 0;
    nullD3DGetSwizzleMasks(Width, Height, &MaskU, &MaskV);
    unsigned int AddValU = MaskU & 0xFFFFFFC0u;
    unsigned int AddValV = MaskV & 0xFFFFFFE0u;
    const unsigned int* SourcePixels = (const unsigned int*)Source;
    for (unsigned int DestinationY = 0, V = 0;; DestinationY += 4) {
        for (unsigned int DestinationX = 0, U = 0;; DestinationX += 8) {
            const unsigned int* Block = SourcePixels + (V | U);
            unsigned int* Row0 = Dest + DestinationY * Width + DestinationX;
            unsigned int* Row1 = Row0 + Width;
            unsigned int* Row2 = Row1 + Width;
            unsigned int* Row3 = Row2 + Width;

            // The release XBE's unswiz2d_32bit writes this 8x4 block in
            // the following order after its movelh/movehl operations.
            Row0[0] = Block[0];
            Row0[1] = Block[1];
            Row0[2] = Block[4];
            Row0[3] = Block[5];
            Row0[4] = Block[16];
            Row0[5] = Block[17];
            Row0[6] = Block[20];
            Row0[7] = Block[21];
            Row1[0] = Block[6];
            Row1[1] = Block[7];
            Row1[2] = Block[2];
            Row1[3] = Block[3];
            Row1[4] = Block[22];
            Row1[5] = Block[23];
            Row1[6] = Block[18];
            Row1[7] = Block[19];
            Row2[0] = Block[8];
            Row2[1] = Block[9];
            Row2[2] = Block[12];
            Row2[3] = Block[13];
            Row2[4] = Block[24];
            Row2[5] = Block[25];
            Row2[6] = Block[28];
            Row2[7] = Block[29];
            Row3[0] = Block[14];
            Row3[1] = Block[15];
            Row3[2] = Block[10];
            Row3[3] = Block[11];
            Row3[4] = Block[30];
            Row3[5] = Block[31];
            Row3[6] = Block[26];
            Row3[7] = Block[27];

            U = MaskU & (U - AddValU);
            if (U == 0)
                break;
        }
        V = MaskV & (V - AddValV);
        if (V == 0)
            break;
    }
}

static void nullD3DUploadExternalTexture(nullD3DInfo* Info, unsigned int Size) {
    if (Info == NULL || Info->NativeTexture == NULL || Info->Bits == NULL)
        return;
    unsigned int BytesPerPixel = nullD3DFormatBytesPerPixel(Info->Format);
    if (BytesPerPixel == 0)
        return;
    if (XGIsSwizzledFormat(Info->Format)) {
        if (BytesPerPixel != 1 && BytesPerPixel != 2 && BytesPerPixel != 4)
            return;
        COD3_D3D9_LOCKED_RECT Locked = {};
        if (FAILED(Info->NativeTexture->LockRect(0, &Locked, NULL, 0)))
            return;
        size_t LinearBytes = (size_t)Info->Width * Info->Height * BytesPerPixel;
        unsigned char* Linear = (unsigned char*)calloc(1, LinearBytes);
        if (Linear != NULL) {
            if (BytesPerPixel == 4 && Info->Width >= 8 && Info->Height >= 8)
                nullD3DUnswizzle32(Info->Bits, (unsigned int*)Linear,
                                   Info->Width, Info->Height);
            else
                nullD3DUnswizzleBytes(Info->Bits, Linear, Info->Width, Info->Height,
                                      BytesPerPixel);
            for (unsigned int y = 0; y < Info->Height; ++y)
                memcpy((unsigned char*)Locked.pBits + y * Locked.Pitch,
                       Linear + (size_t)y * Info->Width * BytesPerPixel,
                       Info->Width * BytesPerPixel);
            free(Linear);
        }
        Info->NativeTexture->UnlockRect(0);
        return;
    }
    unsigned int SourcePitch = Info->Width * BytesPerPixel;
    if (Size != 0)
        SourcePitch = (((Size >> 24) & 0xFFu) + 1u) << 6;
    COD3_D3D9_LOCKED_RECT Locked = {};
    if (FAILED(Info->NativeTexture->LockRect(0, &Locked, NULL, 0)))
        return;
    unsigned int RowBytes = Info->Width * BytesPerPixel;
    unsigned int Rows = Info->Height;
    const unsigned char* Source = Info->Bits;
    unsigned char* Destination = (unsigned char*)Locked.pBits;
    for (unsigned int y = 0; y < Rows; ++y)
        memcpy(Destination + y * Locked.Pitch, Source + y * SourcePitch, RowBytes);
    Info->NativeTexture->UnlockRect(0);
}

static void nullD3DCreateExternalNative(nullD3DInfo* Info, unsigned int Size,
                                        unsigned int PackedFormat) {
    if (Info == NULL || Info->NativeTexture != NULL || gD3D9Device == NULL)
        return;
    if ((PackedFormat & 0x40u) != 0)
        return;
    if (nullD3DFormatBytesPerPixel(Info->Format) == 0)
        return;
    if (FAILED(gD3D9Device->CreateTexture(Info->Width, Info->Height, Info->Levels, 0,
                                          nullD3DNativeFormat(Info->Format), D3DPOOL_MANAGED,
                                          &Info->NativeTexture, NULL)))
        return;
    Info->NativeResource = Info->NativeTexture;
    nullD3DUploadExternalTexture(Info, Size);
}

static nullD3DInfo* nullD3DAdoptExternalTexture(D3DBaseTexture* Texture) {
    if (Texture == NULL || (Texture->Common & 0x70000u) != 0x40000u)
        return NULL;

    nullD3DExternalTexture* Slot = NULL;
    for (unsigned int i = 0; i < sizeof(gNullExternalTextures) / sizeof(gNullExternalTextures[0]); ++i) {
        if (gNullExternalTextures[i].Object == Texture) {
            Slot = &gNullExternalTextures[i];
            break;
        }
        if (Slot == NULL && gNullExternalTextures[i].Object == NULL)
            Slot = &gNullExternalTextures[i];
    }
    if (Slot == NULL)
        return NULL;

    Slot->Object = Texture;
    Slot->PackedFormat = Texture->Format;
    Slot->PackedSize = Texture->Size;
    nullD3DInfo* Info = &Slot->Info;
    if (Info->Magic != NULL_D3D_MAGIC) {
        memset(Info, 0, sizeof(*Info));
        Info->Magic = NULL_D3D_MAGIC;
        Info->Kind = NULL_D3D_TEXTURE;
    }
    Info->Width = nullD3DExternalTextureWidth(Texture->Format, Texture->Size);
    Info->Height = nullD3DExternalTextureHeight(Texture->Format, Texture->Size);
    Info->Depth = 1;
    Info->Levels = nullD3DExternalTextureLevels(Texture->Format);
    Info->Format = (Texture->Format >> 8) & 0xFFu;
    Info->SizeBytes = 0;
    Info->Bits = (unsigned char*)(uintptr_t)Texture->Data;
    nullD3DCreateExternalNative(Info, Slot->PackedSize, Slot->PackedFormat);
    return Info;
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
    Texture->Info.SizeBytes = (unsigned int)Bytes;
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
    Surface->Info.SizeBytes = (unsigned int)Bytes;
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
    // The Xbox presentation target has no host HWND.  Give the D3D9 shim a
    // real client area using the mode selected by NGL so Present() has a
    // visible frontend target on Win32.
    DWORD Style = WS_OVERLAPPEDWINDOW;
    RECT ClientRect = { 0, 0, (LONG)(gNullWidth != 0 ? gNullWidth : 640),
                        (LONG)(gNullHeight != 0 ? gNullHeight : 480) };
    AdjustWindowRect(&ClientRect, Style, FALSE);
    gD3D9Window = CreateWindowExA(0, ClassName, "Call of Duty 3",
                                  Style | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT,
                                  ClientRect.right - ClientRect.left,
                                  ClientRect.bottom - ClientRect.top,
                                  NULL, NULL, Class.hInstance, NULL);
    if (gD3D9Window != NULL)
        UpdateWindow(gD3D9Window);
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

static nullD3DBuffer* nullD3DCreateBuffer(unsigned int Bytes, bool IndexBuffer) {
    nullD3DBuffer* Buffer = (nullD3DBuffer*)calloc(1, sizeof(nullD3DBuffer));
    if (Buffer == NULL)
        return NULL;
    if (Bytes == 0)
        Bytes = 1;
    Buffer->Info.Magic = NULL_D3D_MAGIC;
    Buffer->Info.Kind = NULL_D3D_BUFFER;
    Buffer->Info.SizeBytes = Bytes;
    Buffer->Info.Bits = (unsigned char*)calloc(1, Bytes);
    Buffer->Object.Data = (unsigned int)(uintptr_t)Buffer->Info.Bits;
    if (gD3D9Device != NULL) {
        if (IndexBuffer) {
            gD3D9Device->CreateIndexBuffer(Bytes, 0, COD3_D3D9_FMT_INDEX16,
                                            D3DPOOL_MANAGED,
                                            (IDirect3DIndexBuffer9**)&Buffer->Info.NativeResource,
                                            NULL);
        } else {
            gD3D9Device->CreateVertexBuffer(Bytes, 0, 0, D3DPOOL_MANAGED,
                                             (IDirect3DVertexBuffer9**)&Buffer->Info.NativeResource,
                                             NULL);
        }
    }
    return Buffer;
}

extern "C" {

void __fastcall D3DDevice_SetRenderState_Simple(unsigned int Method, unsigned int Value) {
    if (gD3D9Device == NULL)
        return;

    COD3_D3D9_RENDERSTATETYPE NativeState;
    DWORD NativeValue = Value;
    if (Method == dword_40300) {
        NativeState = COD3_D3D9_RS_ALPHATESTENABLE;
    } else if (Method == dword_40304) {
        NativeState = COD3_D3D9_RS_ALPHABLENDENABLE;
    } else if (Method == dword_4033C) {
        NativeState = COD3_D3D9_RS_ALPHAFUNC;
        NativeValue = nullD3DCompareFunc(Value);
    } else if (Method == dword_40340) {
        NativeState = COD3_D3D9_RS_ALPHAREF;
    } else if (Method == dword_40344) {
        NativeState = COD3_D3D9_RS_SRCBLEND;
    } else if (Method == dword_40348) {
        NativeState = COD3_D3D9_RS_DESTBLEND;
    } else if (Method == dword_4034C) {
        NativeState = D3DRS_BLENDFACTOR;
    } else if (Method == dword_40350) {
        NativeState = COD3_D3D9_RS_BLENDOP;
    } else if (Method == dword_40354) {
        NativeState = (COD3_D3D9_RENDERSTATETYPE)23;
        NativeValue = nullD3DCompareFunc(Value);
    } else if (Method == dword_40358) {
        NativeState = COD3_D3D9_RS_COLORWRITEENABLE;
    } else if (Method == dword_4035C) {
        NativeState = COD3_D3D9_RS_ZWRITEENABLE;
    } else {
        return;
    }
    gD3D9Device->SetRenderState(NativeState, NativeValue);
}
void __fastcall D3DDevice_SetVertexShaderConstant1Fast(unsigned int Register,
                                                       const void* Data) {
    if (gD3D9Device != NULL && Data != NULL && Register < 256)
        gD3D9Device->SetVertexShaderConstantF(Register, (const float*)Data, 1);
}
void __fastcall D3DDevice_SetVertexShaderConstantNotInlineFast(int Register,
                                                               const void* Data,
                                                               unsigned int DwordCount) {
    // The XDK entry point receives a DWORD count.  The IDA call sites pass
    // four DWORDs per float4 constant, so convert to D3D9's vector count.
    if (gD3D9Device == NULL || Data == NULL || Register < 0 || Register >= 256 ||
        DwordCount == 0 || (DwordCount & 3u) != 0)
        return;
    unsigned int VectorCount = DwordCount / 4u;
    if (VectorCount > 256u - (unsigned int)Register)
        VectorCount = 256u - (unsigned int)Register;
    if (VectorCount != 0)
        gD3D9Device->SetVertexShaderConstantF(Register, (const float*)Data, VectorCount);
}
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
unsigned int* __stdcall D3DDevice_BeginPush(unsigned int Count) {
    if (gNullPushBuffer != NULL) {
        free(gNullPushBuffer);
        gNullPushBuffer = NULL;
    }
    if (Count == 0)
        Count = 1;
    gNullPushBuffer = (unsigned int*)calloc(Count, sizeof(unsigned int));
    if (gNullPushBuffer == NULL)
        return NULL;
    return gNullPushBuffer;
}
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
    return (D3DIndexBuffer*)nullD3DCreateBuffer(Bytes, true);
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
    return (D3DVertexBuffer*)nullD3DCreateBuffer(Bytes, false);
}
void __stdcall D3DDevice_DrawIndexedVertices(_D3DPRIMITIVETYPE PrimitiveType,
                                              unsigned int VertexCount,
                                              const unsigned short* IndexData) {
    if (gD3D9Device == NULL || gD3D9VertexDeclaration == NULL ||
        gD3D9IndexBuffer == NULL || IndexData == NULL)
        return;
    COD3_D3D9_PRIMITIVETYPE NativePrimitive = nullD3DPrimitiveType(PrimitiveType);
    unsigned int PrimitiveCount = nullD3DPrimitiveCount(PrimitiveType, VertexCount);
    if (NativePrimitive == COD3_D3D9_PT_FORCE_DWORD || PrimitiveCount == 0)
        return;
    nullD3DInfo* Info = gD3D9IndexInfo;
    uintptr_t IndexAddress = (uintptr_t)IndexData;
    IDirect3DIndexBuffer9* NativeIndex = gD3D9IndexBuffer;
    unsigned int StartIndex = 0;
    if (Info != NULL && Info->Bits != NULL &&
        IndexAddress >= (uintptr_t)Info->Bits &&
        IndexAddress < (uintptr_t)Info->Bits + Info->SizeBytes)
        StartIndex = (unsigned int)((IndexAddress - (uintptr_t)Info->Bits) / sizeof(unsigned short));
    nullD3DSyncIndexBuffer(NativeIndex, Info);
    for (unsigned int i = 0; i < 4; ++i)
        nullD3DSyncVertexBuffer(gD3D9VertexBuffers[i], gD3D9VertexInfos[i]);
    gD3D9Device->SetVertexDeclaration(gD3D9VertexDeclaration);
    gD3D9Device->SetIndices(NativeIndex);
    unsigned int NumVertices = 0;
    if (gD3D9VertexInfos[0] != NULL && gD3D9VertexStrides[0] != 0 &&
        gD3D9VertexInfos[0]->SizeBytes > gD3D9VertexOffsets[0]) {
        NumVertices = (gD3D9VertexInfos[0]->SizeBytes - gD3D9VertexOffsets[0]) /
                      gD3D9VertexStrides[0];
    }
    if (NumVertices != 0)
        gD3D9Device->DrawIndexedPrimitive(NativePrimitive, 0, 0, NumVertices,
                                          StartIndex, PrimitiveCount);
}
void __stdcall D3DDevice_DrawVerticesUP(_D3DPRIMITIVETYPE PrimitiveType,
                                         unsigned int VertexCount,
                                         const void* VertexData,
                                         unsigned int VertexStride) {
    if (gD3D9Device == NULL || gD3D9VertexDeclaration == NULL || VertexData == NULL)
        return;
    COD3_D3D9_PRIMITIVETYPE NativePrimitive = nullD3DPrimitiveType(PrimitiveType);
    unsigned int PrimitiveCount = nullD3DPrimitiveCount(PrimitiveType, VertexCount);
    if (NativePrimitive == COD3_D3D9_PT_FORCE_DWORD || PrimitiveCount == 0)
        return;
    gD3D9Device->SetVertexDeclaration(gD3D9VertexDeclaration);
    gD3D9Device->DrawPrimitiveUP(NativePrimitive, PrimitiveCount, VertexData, VertexStride);
}
static void nullD3DSetScreenSpaceTransform() {
    if (gD3D9Device == NULL || gNullWidth == 0 || gNullHeight == 0)
        return;
    D3DMATRIX Identity = {};
    Identity._11 = 1.0f;
    Identity._22 = 1.0f;
    Identity._33 = 1.0f;
    Identity._44 = 1.0f;
    D3DMATRIX Projection = {};
    Projection._11 = 2.0f / (float)gNullWidth;
    Projection._22 = -2.0f / (float)gNullHeight;
    Projection._33 = 1.0f;
    Projection._41 = -1.0f;
    Projection._42 = 1.0f;
    Projection._44 = 1.0f;
    gD3D9Device->SetTransform(D3DTS_WORLD, &Identity);
    gD3D9Device->SetTransform(D3DTS_VIEW, &Identity);
    gD3D9Device->SetTransform(D3DTS_PROJECTION, &Projection);
}

static bool nullD3DNormalizeTexelCoordinates(unsigned int* Vertices,
                                             unsigned int VertexCount,
                                             unsigned int StrideDwords,
                                             unsigned int UIndex) {
    unsigned int Width = gNullTextureWidths[0];
    unsigned int Height = gNullTextureHeights[0];
    if (Width <= 1 || Height <= 1)
        return false;
    bool TexelCoordinates = false;
    for (unsigned int i = 0; i < VertexCount; ++i) {
        const float U = *(const float*)&Vertices[i * StrideDwords + UIndex];
        const float V = *(const float*)&Vertices[i * StrideDwords + UIndex + 1];
        if (U < -1.0f || U > 1.0f || V < -1.0f || V > 1.0f) {
            TexelCoordinates = true;
            break;
        }
    }
    if (!TexelCoordinates)
        return false;
    for (unsigned int i = 0; i < VertexCount; ++i) {
        float* U = (float*)&Vertices[i * StrideDwords + UIndex];
        float* V = (float*)&Vertices[i * StrideDwords + UIndex + 1];
        *U /= (float)Width;
        *V /= (float)Height;
    }
    return true;
}

static void nullD3DSubmitPush(const unsigned int* Begin, const unsigned int* End) {
    if (gD3D9Device == NULL || Begin == NULL || End == NULL || End <= Begin + 2)
        return;

    // nglRenderQuad emits one 4-vertex PCUV packet.  The packet layout is
    // the same layout used by nglStringNode::Render for each font glyph.
    const unsigned int* Cursor = Begin;
    if (Cursor[1] != 8)
        return;
    nullD3DSetScreenSpaceTransform();

    if (Cursor + 3 < End && Cursor[2] == 0x40601818u) {
        const unsigned int* Vertices = Cursor + 3;
        if (Vertices + 24 <= End) {
            unsigned int NormalizedVertices[24];
            memcpy(NormalizedVertices, Vertices, sizeof(NormalizedVertices));
            nullD3DNormalizeTexelCoordinates(NormalizedVertices, 4, 6, 4);
            gD3D9Device->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1);
            gD3D9Device->DrawPrimitiveUP(COD3_D3D9_PT_TRIANGLESTRIP, 2,
                                         NormalizedVertices, 24);
        }
        return;
    }

    // Filter copies emit a 4-vertex PUV packet.  They use the same command
    // header with a five-DWORD vertex (XYZ + UV) instead of PCUV.
    if (Cursor + 3 < End && Cursor[2] == 0x40501818u) {
        const unsigned int* Vertices = Cursor + 3;
        if (Vertices + 20 <= End) {
            unsigned int NormalizedVertices[20];
            memcpy(NormalizedVertices, Vertices, sizeof(NormalizedVertices));
            nullD3DNormalizeTexelCoordinates(NormalizedVertices, 4, 5, 3);
            gD3D9Device->SetFVF(D3DFVF_XYZ | D3DFVF_TEX1);
            gD3D9Device->DrawPrimitiveUP(COD3_D3D9_PT_TRIANGLESTRIP, 2,
                                         NormalizedVertices, 20);
        }
        return;
    }

    // Font strings start with the shared inline-array command and contain a
    // sequence of PCUV glyph quads.  The count encoded by each command is
    // the number of DWORDs in its following vertex array.
    Cursor += 2;
    gD3D9Device->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1);
    while (Cursor + 1 < End) {
        unsigned int Command = *Cursor++;
        if (Command == 0 && *Cursor == 0)
            break;
        if ((Command & 0x3FFFFu) != 0x18u || Command < 0x4000018u)
            return;
        unsigned int DwordCount = (Command - 0x4000018u) >> 18;
        if (DwordCount == 0 || (DwordCount % 24u) != 0 || Cursor + DwordCount > End)
            return;
        for (unsigned int Offset = 0; Offset < DwordCount; Offset += 24)
            gD3D9Device->DrawPrimitiveUP(COD3_D3D9_PT_TRIANGLESTRIP, 2,
                                         Cursor + Offset, 24);
        Cursor += DwordCount;
    }
}

void __stdcall D3DDevice_EndPush(unsigned int* End) {
    if (gNullPushBuffer != NULL && End != NULL)
        nullD3DSubmitPush(gNullPushBuffer, End);
    free(gNullPushBuffer);
    gNullPushBuffer = NULL;
}
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
void __stdcall D3DDevice_SetIndices(D3DIndexBuffer* IndexBuffer, unsigned int) {
    gD3D9IndexBuffer = NULL;
    gD3D9IndexInfo = NULL;
    if (gD3D9Device == NULL || IndexBuffer == NULL)
        return;
    nullD3DInfo* Info = nullD3DFindInfo(IndexBuffer);
    if (Info != NULL) {
        gD3D9IndexInfo = Info;
        gD3D9IndexBuffer = (IDirect3DIndexBuffer9*)Info->NativeResource;
    }
    gD3D9Device->SetIndices(gD3D9IndexBuffer);
}
void __stdcall D3DDevice_SetPalette(unsigned int, D3DPalette*) {}
void __stdcall D3DDevice_SetRenderState_MultiSampleAntiAlias(unsigned int Value) {
    if (gD3D9Device != NULL)
        gD3D9Device->SetRenderState(COD3_D3D9_RS_MULTISAMPLEANTIALIAS, Value != 0);
}
void __stdcall D3DDevice_SetRenderState_RopZCmpAlwaysRead(unsigned int) {}
void __stdcall D3DDevice_SetRenderState_StencilEnable(unsigned int Value) {
    if (gD3D9Device != NULL)
        gD3D9Device->SetRenderState(COD3_D3D9_RS_STENCILENABLE, Value != 0);
}
void __stdcall D3DDevice_SetRenderState_YuvEnable(unsigned int) {}
void __stdcall D3DDevice_SetRenderState_ZEnable(unsigned int Value) {
    if (gD3D9Device != NULL)
        gD3D9Device->SetRenderState(COD3_D3D9_RS_ZENABLE, Value);
}
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
void __stdcall D3DDevice_SetTexture(unsigned int Stage, D3DBaseTexture* Texture) {
    if (gD3D9Device == NULL || Stage >= 4)
        return;
    IDirect3DBaseTexture9* NativeTexture = NULL;
    nullD3DInfo* Info = nullD3DTextureInfo(Texture);
    nullD3DExternalTexture* External = nullD3DFindExternalTexture(Texture);
    if (External != NULL)
        nullD3DCreateExternalNative(&External->Info, External->PackedSize,
                                    External->PackedFormat);
    if (Info != NULL)
        NativeTexture = Info->NativeTexture;
    gNullTextureWidths[Stage] = Info != NULL ? Info->Width : 0;
    gNullTextureHeights[Stage] = Info != NULL ? Info->Height : 0;
    gD3D9Device->SetTexture(Stage, NativeTexture);
}
int __stdcall D3DDevice_SetTextureState_ParameterCheck(unsigned int Stage,
                                                        _D3DTEXTURESTAGESTATETYPE Type,
                                                        unsigned int Value) {
    if (gD3D9Device == NULL || Stage >= 4)
        return 0;
    DWORD Sampler = 0;
    DWORD NativeValue = Value;
    bool SamplerState = true;
    switch (Type) {
    case D3DTSS_ADDRESSU: Sampler = D3DSAMP_ADDRESSU; break;
    case D3DTSS_ADDRESSV: Sampler = D3DSAMP_ADDRESSV; break;
    case D3DTSS_ADDRESSW: Sampler = D3DSAMP_ADDRESSW; break;
    case D3DTSS_MAGFILTER: Sampler = D3DSAMP_MAGFILTER; break;
    case D3DTSS_MINFILTER: Sampler = D3DSAMP_MINFILTER; break;
    case D3DTSS_MIPFILTER: Sampler = D3DSAMP_MIPFILTER; break;
    case D3DTSS_MIPMAPLODBIAS: Sampler = D3DSAMP_MIPMAPLODBIAS; break;
    case D3DTSS_MAXMIPLEVEL: Sampler = D3DSAMP_MAXMIPLEVEL; break;
    case D3DTSS_MAXANISOTROPY: Sampler = D3DSAMP_MAXANISOTROPY; break;
    case D3DTSS_COLOROP:
    case D3DTSS_COLORARG1:
    case D3DTSS_COLORARG2:
    case D3DTSS_ALPHAOP:
    case D3DTSS_ALPHAARG1:
    case D3DTSS_ALPHAARG2:
        SamplerState = false;
        break;
    default:
        return 0;
    }
    if (SamplerState)
        gD3D9Device->SetSamplerState(Stage, (D3DSAMPLERSTATETYPE)Sampler, NativeValue);
    else
        gD3D9Device->SetTextureStageState(Stage, (COD3_D3D9_TEXTURESTAGESTATETYPE)Type,
                                          NativeValue);
    return 0;
}
void __stdcall D3DDevice_SetVertexShader(unsigned int) {}
void __stdcall D3DDevice_SetVerticalBlankCallback(void (*Callback)(_D3DVBLANKDATA*)) { gNullVBlankCallback = Callback; }
void __stdcall D3DDevice_SetViewport(const void* ViewportData) {
    if (gD3D9Device == NULL || ViewportData == NULL)
        return;
    const _D3DVIEWPORT8* Viewport = (const _D3DVIEWPORT8*)ViewportData;
    D3DVIEWPORT9 NativeViewport;
    NativeViewport.X = Viewport->X;
    NativeViewport.Y = Viewport->Y;
    NativeViewport.Width = Viewport->Width;
    NativeViewport.Height = Viewport->Height;
    NativeViewport.MinZ = Viewport->MinZ;
    NativeViewport.MaxZ = Viewport->MaxZ;
    gD3D9Device->SetViewport(&NativeViewport);
}
void __stdcall D3DDevice_Swap(unsigned int) {
    if (gD3D9Device != NULL) {
        IDirect3DSurface9* SwapchainBackBuffer = NULL;
        if (SUCCEEDED(gD3D9Device->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO,
                                                  &SwapchainBackBuffer))) {
            if (gD3D9RenderTarget != NULL &&
                gD3D9RenderTarget != SwapchainBackBuffer) {
                gD3D9Device->StretchRect(gD3D9RenderTarget, NULL,
                                         SwapchainBackBuffer, NULL, D3DTEXF_NONE);
            }
            SwapchainBackBuffer->Release();
        }
        gD3D9Device->Present(NULL, NULL, NULL, NULL);
    }
    if (gNullVBlankCallback != NULL) {
        _D3DVBLANKDATA Data = { 0, 0, 0 };
        gNullVBlankCallback(&Data);
    }
}
void __stdcall D3DDevice_SwitchTexture(unsigned int, unsigned int, unsigned int) {}
unsigned int __stdcall D3DPalette_Lock2(D3DPalette*, unsigned int) { return 0; }
void __stdcall D3DResource_BlockUntilNotBusy(D3DResource*) {}
void __stdcall D3DResource_Register(D3DResource* Resource, void* Base) {
    if (Resource == NULL || Base == NULL)
        return;

    nullD3DExternalTexture* Existing =
        nullD3DFindExternalTexture((D3DBaseTexture*)Resource);
    if (Existing != NULL && Existing->Info.Magic == NULL_D3D_MAGIC) {
        nullD3DCreateExternalNative(&Existing->Info, Existing->PackedSize,
                                    Existing->PackedFormat);
        return;
    }

    // This is the host equivalent of the Xbox Register contract: the
    // serialized resource stores an offset into the physical section, while
    // callers subsequently expect Data to be an address.
    unsigned int DataOffset = Resource->Data;
    Resource->Data = (unsigned int)(uintptr_t)((unsigned char*)Base + DataOffset);

    if ((Resource->Common & 0x70000u) != 0x40000u)
        return;

    D3DBaseTexture* Texture = (D3DBaseTexture*)Resource;
    nullD3DExternalTexture* Slot = NULL;
    for (unsigned int i = 0; i < sizeof(gNullExternalTextures) / sizeof(gNullExternalTextures[0]); ++i) {
        if (gNullExternalTextures[i].Object == Texture) {
            Slot = &gNullExternalTextures[i];
            break;
        }
        if (Slot == NULL && gNullExternalTextures[i].Object == NULL)
            Slot = &gNullExternalTextures[i];
    }
    if (Slot == NULL)
        return;

    Slot->Object = Texture;
    Slot->PackedFormat = Texture->Format;
    Slot->PackedSize = Texture->Size;
    nullD3DInfo* Info = &Slot->Info;
    if (Info->Magic != NULL_D3D_MAGIC) {
        memset(Info, 0, sizeof(*Info));
        Info->Magic = NULL_D3D_MAGIC;
        Info->Kind = NULL_D3D_TEXTURE;
    }
    Info->Width = nullD3DExternalTextureWidth(Texture->Format, Texture->Size);
    Info->Height = nullD3DExternalTextureHeight(Texture->Format, Texture->Size);
    Info->Depth = 1;
    Info->Levels = nullD3DExternalTextureLevels(Texture->Format);
    Info->Format = (Texture->Format >> 8) & 0xFFu;
    Info->SizeBytes = 0;
    Info->Bits = (unsigned char*)(uintptr_t)Resource->Data;
    nullD3DCreateExternalNative(Info, Texture->Size, Slot->PackedFormat);
}
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
    nullD3DInitStateAliases();
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
int __stdcall XGIsSwizzledFormat(unsigned int Format) {
    // Exact XGRPH format ranges from _XGIsSwizzledFormat@4 in the release
    // XBE.  Linear formats and the reserved gaps return false; the Xbox
    // swizzled/compressed ranges return true.
    if (Format <= 7u || Format == 0xBu ||
        (Format >= 0x19u && Format <= 0x1Au) ||
        (Format >= 0x27u && Format <= 0x2Du) ||
        (Format >= 0x32u && Format <= 0x33u) ||
        (Format >= 0x38u && Format <= 0x3Cu))
        return 1;
    return 0;
}
void __stdcall XGSetPaletteHeader(_D3DPALETTESIZE, D3DPalette* Palette, void* Data) {
    if (Palette != NULL) Palette->Data = (unsigned int)(uintptr_t)Data;
}
void __stdcall XGSetTextureHeader(unsigned int Width, unsigned int Height, unsigned int Levels,
                                  unsigned int Usage, unsigned int Format, unsigned int,
                                  D3DBaseTexture* Texture, void* Data, unsigned int Pitch) {
    if (Texture != NULL) {
        // XGRAPHICS::EncodeTexture's non-swizzled branch (the branch used by
        // nglDxFilters for LIN_A8R8G8B8) stores the format, mip count, and
        // linear surface dimensions in the Xbox packed fields.
        unsigned int MipLevels = Levels == 0 ? 1 : Levels;
        unsigned int EncodedFormat = 9u | (16u * (2u | (16u *
            (Format | (MipLevels << 8)))));
        unsigned int LinearPitch = Pitch;
        if (LinearPitch == 0)
            LinearPitch = Width * nullD3DLinearBytesPerPixel(Format);
        Texture->Format = EncodedFormat;
        if ((Usage & 0x10000u) != 0)
            Texture->Format &= ~8u;
        Texture->Size = (Width - 1u) | ((Height - 1u) << 12) |
                        ((((LinearPitch >> 6) - 1u) & 0xFFu) << 24);
        Texture->Common = 0x40001u;
        Texture->Lock = 0;
        Texture->Data = (unsigned int)(uintptr_t)Data;
        nullD3DAdoptExternalTexture(Texture);
    }
}
void __stdcall XGSwizzleRect(const void* Source, unsigned int Pitch, const void* Rect,
                             void* Dest, unsigned int Width, unsigned int Height,
                             const void* Point, unsigned int BytesPerPixel) {
    if (Source == NULL || Dest == NULL || BytesPerPixel == 0)
        return;
    unsigned int RowBytes = Width * BytesPerPixel;
    if (Pitch == 0)
        Pitch = RowBytes;
    if (Rect == NULL && Point == NULL && Width != 0 && Height != 0 &&
        (Width & (Width - 1u)) == 0 && (Height & (Height - 1u)) == 0) {
        nullD3DSwizzleBytes((const unsigned char*)Source, Pitch,
                            (unsigned char*)Dest, Width, Height, BytesPerPixel);
        return;
    }
    for (unsigned int y = 0; y < Height; ++y)
        memcpy((unsigned char*)Dest + y * RowBytes,
               (const unsigned char*)Source + y * Pitch, RowBytes);
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
void __stdcall D3DDevice_SelectVertexShaderDirect(_D3DVERTEXATTRIBUTEFORMAT* Format,
                                                   unsigned int) {
    if (gD3D9Device == NULL || Format == NULL)
        return;
    nullD3DBuildVertexDeclaration(Format);
    if (gD3D9VertexDeclaration != NULL)
        gD3D9Device->SetVertexDeclaration(gD3D9VertexDeclaration);
}
void __stdcall D3DDevice_SetPixelShaderProgram(const _D3DPixelShaderDef*) {}
void __stdcall D3DDevice_SetRenderState_CullMode(unsigned int Value) {
    if (gD3D9Device != NULL)
        gD3D9Device->SetRenderState(COD3_D3D9_RS_CULLMODE,
                                     nullD3DCullMode(Value));
}
void __stdcall D3DDevice_SetRenderState_FogColor(unsigned int Value) {
    if (gD3D9Device != NULL)
        gD3D9Device->SetRenderState(COD3_D3D9_RS_FOGCOLOR, Value);
}
void __stdcall D3DDevice_SetRenderState_ZBias(unsigned int Value) {
    if (gD3D9Device != NULL)
        gD3D9Device->SetRenderState(D3DRS_DEPTHBIAS, Value);
}
int __stdcall D3DDevice_SetRenderState_ParameterCheck(unsigned int State, unsigned int Value) {
    if (gD3D9Device == NULL)
        return 0;

    COD3_D3D9_RENDERSTATETYPE NativeState;
    DWORD NativeValue = Value;
    switch (State) {
    case D3DRS_ALPHAFUNC:
        NativeState = COD3_D3D9_RS_ALPHAFUNC;
        NativeValue = nullD3DCompareFunc(Value);
        break;
    case D3DRS_ALPHABLENDENABLE: NativeState = COD3_D3D9_RS_ALPHABLENDENABLE; break;
    case D3DRS_ALPHATESTENABLE: NativeState = COD3_D3D9_RS_ALPHATESTENABLE; break;
    case D3DRS_ALPHAREF: NativeState = COD3_D3D9_RS_ALPHAREF; break;
    case D3DRS_SRCBLEND: NativeState = COD3_D3D9_RS_SRCBLEND; break;
    case D3DRS_DESTBLEND: NativeState = COD3_D3D9_RS_DESTBLEND; break;
    case D3DRS_BLENDOP: NativeState = COD3_D3D9_RS_BLENDOP; break;
    case D3DRS_BLENDCOLOR: NativeState = D3DRS_BLENDFACTOR; break;
    case D3DRS_FOGCOLOR: NativeState = COD3_D3D9_RS_FOGCOLOR; break;
    case D3DRS_ZWRITEENABLE: NativeState = COD3_D3D9_RS_ZWRITEENABLE; break;
    case D3DRS_COLORWRITEENABLE: NativeState = COD3_D3D9_RS_COLORWRITEENABLE; break;
    case D3DRS_SPECULARENABLE: NativeState = COD3_D3D9_RS_SPECULARENABLE; break;
    case D3DRS_CULLMODE:
        NativeState = COD3_D3D9_RS_CULLMODE;
        NativeValue = nullD3DCullMode(Value);
        break;
    case D3DRS_ZENABLE: NativeState = COD3_D3D9_RS_ZENABLE; break;
    case D3DRS_ZBIAS: NativeState = D3DRS_DEPTHBIAS; break;
    case D3DRS_STENCILENABLE: NativeState = COD3_D3D9_RS_STENCILENABLE; break;
    case D3DRS_STENCILFUNC: NativeState = COD3_D3D9_RS_STENCILFUNC; break;
    case D3DRS_STENCILMASK: NativeState = COD3_D3D9_RS_STENCILMASK; break;
    case D3DRS_STENCILPASS: NativeState = COD3_D3D9_RS_STENCILPASS; break;
    case D3DRS_MULTISAMPLEANTIALIAS: NativeState = COD3_D3D9_RS_MULTISAMPLEANTIALIAS; break;
    default:
        return 0;
    }
    gD3D9Device->SetRenderState(NativeState, NativeValue);
    return 0;
}
void __stdcall D3DDevice_SetVertexShaderInputDirect(void* VertexFormat,
                                                     unsigned int StreamCount,
                                                     const _D3DSTREAM_INPUT* StreamInputs) {
    if (gD3D9Device == NULL)
        return;
    if (VertexFormat != NULL)
        nullD3DBuildVertexDeclaration((_D3DVERTEXATTRIBUTEFORMAT*)VertexFormat);
    for (unsigned int i = 0; i < 4; ++i) {
        if (i >= StreamCount || StreamInputs == NULL) {
            gD3D9VertexBuffers[i] = NULL;
            gD3D9VertexInfos[i] = NULL;
            gD3D9VertexOffsets[i] = 0;
            gD3D9VertexStrides[i] = 0;
            gD3D9Device->SetStreamSource(i, NULL, 0, 0);
            continue;
        }
        nullD3DInfo* Info = nullD3DFindInfo(StreamInputs[i].VertexBuffer);
        IDirect3DVertexBuffer9* NativeBuffer =
            Info == NULL ? NULL : (IDirect3DVertexBuffer9*)Info->NativeResource;
        gD3D9VertexBuffers[i] = NativeBuffer;
        gD3D9VertexInfos[i] = Info;
        gD3D9VertexOffsets[i] = StreamInputs[i].Offset;
        gD3D9VertexStrides[i] = StreamInputs[i].Stride;
        gD3D9Device->SetStreamSource(i, NativeBuffer, StreamInputs[i].Offset,
                                      StreamInputs[i].Stride);
    }
    if (VertexFormat != NULL && gD3D9VertexDeclaration != NULL)
        gD3D9Device->SetVertexDeclaration(gD3D9VertexDeclaration);
    else if (VertexFormat == NULL && StreamCount == 0)
        gD3D9Device->SetVertexDeclaration(NULL);
}
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
