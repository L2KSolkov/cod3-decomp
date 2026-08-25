// ============================================================================
// xdk_stubs.cpp - XDK (Xbox) API link stubs (NEVER reconstruct; Phase 6
// backends replace these). Generated from the linker unresolved list.
// NOTE: this toolchain only accepts calling-convention keywords AFTER the
// return type on free functions.
// ============================================================================

#include "d3d8.h"
#include "d3d9_compat.h"
#include "nv2a_vsh_d3d9.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <objbase.h>
#include <wincodec.h>
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
// resource layout.  The registry must cover all serialized textures retained
// by a loaded level, not only the small frontend resource set.
struct nullD3DExternalTexture {
    D3DBaseTexture* Object;
    unsigned int PackedFormat;
    unsigned int PackedSize;
    nullD3DInfo Info;
};

static nullD3DExternalTexture gNullExternalTextures[4096] = {};

static nullD3DTexture* gNullFrontBuffer = NULL;
static nullD3DTexture* gNullBackBuffer = NULL;
static nullD3DSurface* gNullDepthBuffer = NULL;
static IDirect3D9* gD3D9 = NULL;
static IDirect3DDevice9* gD3D9Device = NULL;
static HMODULE gD3D9SoftwareRasterizer = NULL;
static bool gD3D9SceneActive = false;
static bool gD3D9NV2APixelShaderActive = false;
static bool gD3D9ShaderNeedsViewportInverse = false;
static IDirect3DSurface9* gD3D9RenderTarget = NULL;
static IDirect3DSurface9* gD3D9DepthStencil = NULL;
static IDirect3DVertexDeclaration9* gD3D9VertexDeclaration = NULL;
static IDirect3DVertexDeclaration9* gD3D9PCUVDeclaration = NULL;
static IDirect3DVertexDeclaration9* gD3D9PUVDeclaration = NULL;
static _D3DVERTEXATTRIBUTEFORMAT gD3D9SelectedVertexFormat = {};
static bool gD3D9SelectedVertexFormatValid = false;
static _D3DVERTEXATTRIBUTEFORMAT gD3D9BuiltVertexFormat = {};
static bool gD3D9BuiltVertexFormatValid = false;
static float gD3D9VertexConstants[256][4] = {};
static bool gD3D9VertexConstantsValid[256] = {};
static IDirect3DIndexBuffer9* gD3D9IndexBuffer = NULL;
static nullD3DInfo* gD3D9IndexInfo = NULL;
static nullD3DBuffer* gNullBuffers[1024] = {};
static IDirect3DVertexBuffer9* gD3D9VertexBuffers[4] = {};
static nullD3DInfo* gD3D9VertexInfos[4] = {};
static const unsigned char* gD3D9VertexData[4] = {};
static unsigned int gD3D9VertexOffsets[4] = {};
static unsigned int gD3D9VertexStrides[4] = {};
static unsigned int gNullTextureWidths[4] = {};
static unsigned int gNullTextureHeights[4] = {};
static D3DBaseTexture* gNullBoundTextures[4] = {};
static D3DPalette* gNullPalettes[4] = {};
struct nullD3DPaletteInfo {
    D3DPalette* Object;
    unsigned int Entries;
};
static nullD3DPaletteInfo gNullPaletteInfos[64] = {};
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

// NGL's render-state method cells retain the Xbox push-buffer IDs. The
// release data stores these values as 0x403xx method IDs; translate them only
// when forwarding the state to D3D9.
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
extern unsigned int dword_40364;
extern unsigned int dword_4036C;
extern unsigned int dword_40378;

static void nullD3DInitStateAliases() {
    dword_40300 = 0x40300u;
    dword_40304 = 0x40304u;
    dword_4033C = 0x4033Cu;
    dword_40340 = 0x40340u;
    dword_40344 = 0x40344u;
    dword_40348 = 0x40348u;
    dword_4034C = 0x4034Cu;
    dword_40350 = 0x40350u;
    dword_40354 = 0x40354u;
    dword_40358 = 0x40358u;
    dword_4035C = 0x4035Cu;
    dword_40364 = 0x40364u;
    dword_4036C = 0x4036Cu;
    dword_40378 = 0x40378u;
    // D3DDevice_SwitchTexture validates these Xbox texture-stage method
    // cells as (8300 + stage) << 6 before emitting its three DWORD packet.
    for (unsigned int Stage = 0; Stage < 4; ++Stage)
        DTE[Stage] = (8300u + Stage) << 6;
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

static DWORD nullD3DBlendFactor(unsigned int Value) {
    // Xbox D3DBLEND uses the NV2A GL-style values while D3D9 uses a compact
    // sequential enum. NGL's packed blend modes feed these values directly
    // through the XDK render-state calls.
    switch (Value) {
    case 0x0000u: return D3DBLEND_ZERO;
    case 0x0001u: return D3DBLEND_ONE;
    case 0x0300u: return D3DBLEND_SRCCOLOR;
    case 0x0301u: return D3DBLEND_INVSRCCOLOR;
    case 0x0302u: return D3DBLEND_SRCALPHA;
    case 0x0303u: return D3DBLEND_INVSRCALPHA;
    case 0x0304u: return D3DBLEND_DESTALPHA;
    case 0x0305u: return D3DBLEND_INVDESTALPHA;
    case 0x0306u: return D3DBLEND_DESTCOLOR;
    case 0x0307u: return D3DBLEND_INVDESTCOLOR;
    case 0x0308u: return D3DBLEND_SRCALPHASAT;
    case 0x8001u: return D3DBLEND_BLENDFACTOR;
    case 0x8002u: return D3DBLEND_INVBLENDFACTOR;
    case 0x8003u: return D3DBLEND_BLENDFACTOR;
    case 0x8004u: return D3DBLEND_INVBLENDFACTOR;
    default: return Value;
    }
}

static DWORD nullD3DBlendOp(unsigned int Value) {
    switch (Value) {
    case 0x8006u: return D3DBLENDOP_ADD;
    case 0x800Au: return D3DBLENDOP_SUBTRACT;
    case 0x800Bu: return D3DBLENDOP_REVSUBTRACT;
    case 0x8007u: return D3DBLENDOP_MIN;
    case 0x8008u: return D3DBLENDOP_MAX;
    default: return Value;
    }
}

static DWORD nullD3DColorWriteMask(unsigned int Value) {
    // Xbox stores one byte per channel; D3D9 uses a four-bit mask.
    return ((Value & 0x00010000u) != 0 ? 0x1u : 0u) |
           ((Value & 0x00000100u) != 0 ? 0x2u : 0u) |
           ((Value & 0x00000001u) != 0 ? 0x4u : 0u) |
           ((Value & 0x01000000u) != 0 ? 0x8u : 0u);
}

static DWORD nullD3DCullMode(unsigned int Value) {
    // IDA's Xbox D3D type declares CW=0x900 and CCW=0x901.  D3D9 uses
    // compact enum values, so translate the Xbox selectors explicitly.
    switch (Value) {
    case 0u: return D3DCULL_NONE;
    case 0x900u: return D3DCULL_CW;
    case 0x901u: return D3DCULL_CCW;
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
    // These values are the Xbox D3D8 enum from d3d8.h.  The D3D9
    // compatibility header renames D3DPT_* macros to the host enum values,
    // so using those macros here would map Xbox 6 (triangle strip) as a fan.
    case 1u: return COD3_D3D9_PT_POINTLIST;
    case 2u: return COD3_D3D9_PT_LINELIST;
    case 4u: return COD3_D3D9_PT_LINESTRIP;
    case 5u: return COD3_D3D9_PT_TRIANGLELIST;
    case 6u: return COD3_D3D9_PT_TRIANGLESTRIP;
    case 7u: return COD3_D3D9_PT_TRIANGLEFAN;
    default: return COD3_D3D9_PT_FORCE_DWORD;
    }
}

static DWORD nullD3DStencilOp(unsigned int Value) {
    // Xbox D3DSTENCILOP uses NV2A method tokens; D3D9 uses 1..8.
    switch (Value) {
    case 0x1E00u: return D3DSTENCILOP_KEEP;
    case 0x0000u: return D3DSTENCILOP_ZERO;
    case 0x1E01u: return D3DSTENCILOP_REPLACE;
    case 0x1E02u: return D3DSTENCILOP_INCRSAT;
    case 0x1E03u: return D3DSTENCILOP_DECRSAT;
    case 0x150Au: return D3DSTENCILOP_INVERT;
    case 0x8507u: return D3DSTENCILOP_INCR;
    case 0x8508u: return D3DSTENCILOP_DECR;
    default: return Value;
    }
}

static unsigned int nullD3DPrimitiveCount(_D3DPRIMITIVETYPE Type, unsigned int VertexCount) {
    switch (Type) {
    case 1u: return VertexCount;
    case 2u: return VertexCount / 2;
    case 4u: return VertexCount > 1 ? VertexCount - 1 : 0;
    case 5u: return VertexCount / 3;
    case 6u:
    case 7u: return VertexCount > 2 ? VertexCount - 2 : 0;
    default: return 0;
    }
}

// Xbox vertex format values are the D3DVSDT encodings used by the NGL
// declarations.  Keep the conversion here in lockstep with the unpacker in
// src/ngl/ngl_gpu.cpp; the world shaders consume the packed normal/weight
// attributes instead of accepting a reduced fixed-function declaration.
static bool nullD3DVertexElementType(unsigned int Format, BYTE* Type) {
    if (Type == NULL)
        return false;
    switch (Format) {
    case 0x11: // SHORT1N
        *Type = D3DDECLTYPE_SHORT2N;
        return true;
    case 0x12: // FLOAT1
        *Type = D3DDECLTYPE_FLOAT1;
        return true;
    case 0x15: // SHORT1
        *Type = D3DDECLTYPE_SHORT2;
        return true;
    case 0x16: // Xbox packed signed 10:10:10 normal (DEC3N).
        *Type = D3DDECLTYPE_DEC3N;
        return true;
    case 0x21: // SHORT2N
        *Type = D3DDECLTYPE_SHORT2N;
        return true;
    case 0x22: // FLOAT2
        *Type = D3DDECLTYPE_FLOAT2;
        return true;
    case 0x25: // SHORT2
        *Type = D3DDECLTYPE_SHORT2;
        return true;
    case 0x31: // SHORT3N (three signed 16-bit normalized components)
        *Type = D3DDECLTYPE_SHORT4N;
        return true;
    case 0x32: // FLOAT3
        *Type = D3DDECLTYPE_FLOAT3;
        return true;
    case 0x35: // SHORT3; D3D9 has no 3-short declaration, use the padded form.
        *Type = D3DDECLTYPE_SHORT4;
        return true;
    case 0x40: // D3DCOLOR
        *Type = D3DDECLTYPE_D3DCOLOR;
        return true;
    case 0x41: // SHORT4N
        *Type = D3DDECLTYPE_SHORT4N;
        return true;
    case 0x42: // FLOAT4
        *Type = D3DDECLTYPE_FLOAT4;
        return true;
    case 0x45: // SHORT4
        *Type = D3DDECLTYPE_SHORT4;
        return true;
    default:
        return false;
    }
}

static void nullD3DInvalidateVertexDeclaration() {
    if (gD3D9VertexDeclaration != NULL) {
        gD3D9VertexDeclaration->Release();
        gD3D9VertexDeclaration = NULL;
    }
    gD3D9BuiltVertexFormatValid = false;
}

static bool nullD3DBuildVertexDeclaration(_D3DVERTEXATTRIBUTEFORMAT* Format) {
    if (gD3D9Device == NULL || Format == NULL)
        return false;
    if (gD3D9VertexDeclaration != NULL && gD3D9BuiltVertexFormatValid &&
        memcmp(&gD3D9BuiltVertexFormat, Format, sizeof(gD3D9BuiltVertexFormat)) == 0)
        return true;
    D3DVERTEXELEMENT9 Elements[17] = {};
    unsigned int Count = 0;
    unsigned int TexCoordIndex = 0;
    bool HasPosition = false;
    for (unsigned int i = 0; i < 16 && Format->Input[i].Format != 2; ++i) {
        const _D3DVERTEXSHADERINPUT& Input = Format->Input[i];
        D3DVERTEXELEMENT9 Element = {};
        Element.Stream = (WORD)Input.StreamIndex;
        Element.Offset = (WORD)Input.Offset;
        Element.Method = D3DDECLMETHOD_DEFAULT;
        Element.UsageIndex = 0;
        if (!nullD3DVertexElementType(Input.Format, &Element.Type)) {
            nullD3DInvalidateVertexDeclaration();
            return false;
        }
        if (i == 0) {
            Element.Usage = D3DDECLUSAGE_POSITION;
            HasPosition = true;
        } else {
            // The NV2A translator exposes vertex registers v1..v7 as
            // TEXCOORD0..6.  D3D9 matches declaration elements to those
            // semantics, so preserve the ordinal even when the source slot
            // is packed color/normal data.
            Element.Usage = D3DDECLUSAGE_TEXCOORD;
            Element.UsageIndex = (BYTE)TexCoordIndex++;
        }
        if (Count >= 16) {
            nullD3DInvalidateVertexDeclaration();
            return false;
        }
        Elements[Count] = Element;
        ++Count;
    }
    if (!HasPosition) {
        nullD3DInvalidateVertexDeclaration();
        return false;
    }
    Elements[Count] = D3DDECL_END();
    IDirect3DVertexDeclaration9* Declaration = NULL;
    if (FAILED(gD3D9Device->CreateVertexDeclaration(Elements, &Declaration))) {
        nullD3DInvalidateVertexDeclaration();
        return false;
    }
    if (gD3D9VertexDeclaration != NULL)
        gD3D9VertexDeclaration->Release();
    gD3D9VertexDeclaration = Declaration;
    memcpy(&gD3D9BuiltVertexFormat, Format, sizeof(gD3D9BuiltVertexFormat));
    gD3D9BuiltVertexFormatValid = true;
    return true;
}

static void nullD3DSetNV2AViewportConstants();

static void nullD3DSetShaderMatrixTransform() {
    nullD3DSetNV2AViewportConstants();
    if (gD3D9Device == NULL || !gD3D9VertexConstantsValid[6] ||
        !gD3D9VertexConstantsValid[7] || !gD3D9VertexConstantsValid[8] ||
        !gD3D9VertexConstantsValid[9])
        return;

    D3DMATRIX Identity = {};
    Identity._11 = 1.0f;
    Identity._22 = 1.0f;
    Identity._33 = 1.0f;
    Identity._44 = 1.0f;

    // NGL uploads transpose(LocalToScreen) to Xbox vertex constants.  D3D9
    // fixed-function transforms use the row-major LocalToScreen matrix.
    D3DMATRIX Projection = {};
    float* Native = &Projection._11;
    for (unsigned int Row = 0; Row < 4; ++Row) {
        for (unsigned int Column = 0; Column < 4; ++Column)
            Native[Row * 4 + Column] = gD3D9VertexConstants[6 + Column][Row];
    }
    gD3D9Device->SetTransform(D3DTS_WORLD, &Identity);
    gD3D9Device->SetTransform(D3DTS_VIEW, &Identity);
    gD3D9Device->SetTransform(D3DTS_PROJECTION, &Projection);
    gD3D9Device->SetRenderState(D3DRS_LIGHTING, FALSE);
}

static void nullD3DSetNV2AViewportConstants() {
    if (!gD3D9ShaderNeedsViewportInverse || gD3D9Device == NULL)
        return;
    D3DVIEWPORT9 viewport = {};
    if (FAILED(gD3D9Device->GetViewport(&viewport))) {
        viewport.Width = gNullWidth;
        viewport.Height = gNullHeight;
    }
    const float width = viewport.Width != 0 ? (float)viewport.Width
                                            : (float)gNullWidth;
    const float height = viewport.Height != 0 ? (float)viewport.Height
                                              : (float)gNullHeight;
    // DeviceXBox uses FSAA scale 1,1 for the active screen target and the
    // verified Xbox half-pixel offset 0.53125 on the X/Y translation rows.
    const float inverseViewport[4] = {
        0.53125f + width * 0.5f,
        0.53125f + height * 0.5f,
        2.0f / width,
        2.0f / height,
    };
    D3DDevice_SetVertexShaderConstant1Fast(191, inverseViewport);
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
    case D3DFMT_A8R8G8B8:
    case D3DFMT_LIN_A8R8G8B8: return COD3_D3D9_FMT_A8R8G8B8;
    case D3DFMT_X8R8G8B8:
    case D3DFMT_LIN_X8R8G8B8: return COD3_D3D9_FMT_X8R8G8B8;
    case D3DFMT_A8B8G8R8:
    case D3DFMT_LIN_A8B8G8R8: return COD3_D3D9_FMT_A8B8G8R8;
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

static nullD3DInfo* nullD3DFindBufferByData(const void* Data) {
    if (Data == NULL)
        return NULL;
    uintptr_t Address = (uintptr_t)Data;
    for (unsigned int i = 0; i < sizeof(gNullBuffers) / sizeof(gNullBuffers[0]); ++i) {
        nullD3DBuffer* Buffer = gNullBuffers[i];
        if (Buffer == NULL || Buffer->Info.Bits == NULL)
            continue;
        uintptr_t Begin = (uintptr_t)Buffer->Info.Bits;
        uintptr_t End = Begin + Buffer->Info.SizeBytes;
        if (Address >= Begin && Address < End)
            return &Buffer->Info;
    }
    return NULL;
}

static nullD3DInfo* nullD3DTextureInfo(D3DBaseTexture* Texture) {
    if (Texture == NULL)
        return NULL;
    for (unsigned int i = 0; i < sizeof(gNullExternalTextures) / sizeof(gNullExternalTextures[0]); ++i) {
        if (gNullExternalTextures[i].Object == Texture)
            return &gNullExternalTextures[i].Info;
    }
    if ((Texture->Common & 0x70000u) == 0x40000u)
        return nullD3DAdoptExternalTexture(Texture);
    nullD3DInfo* Info = &((nullD3DTexture*)Texture)->Info;
    return Info->Magic == NULL_D3D_MAGIC ? Info : NULL;
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

static D3DBaseTexture* nullD3DFindTextureByData(unsigned int Data,
                                                 unsigned int Format) {
    for (unsigned int i = 0; i < sizeof(gNullExternalTextures) / sizeof(gNullExternalTextures[0]); ++i) {
        nullD3DExternalTexture* External = &gNullExternalTextures[i];
        if (External->Object != NULL &&
            External->Info.Bits == (unsigned char*)(uintptr_t)Data &&
            External->PackedFormat == Format)
            return External->Object;
    }
    return NULL;
}

static unsigned int nullD3DPaletteEntryCount(D3DPalette* Palette) {
    if (Palette == NULL)
        return 0;
    for (unsigned int i = 0; i < sizeof(gNullPaletteInfos) / sizeof(gNullPaletteInfos[0]); ++i) {
        if (gNullPaletteInfos[i].Object == Palette)
            return gNullPaletteInfos[i].Entries;
    }
    return 256;
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

static unsigned int nullD3DFormatBlockBytes(unsigned int Format) {
    switch (Format) {
    case D3DFMT_DXT1:
        return 8;
    case D3DFMT_DXT3:
    case D3DFMT_DXT5:
        return 16;
    default:
        return 0;
    }
}

static unsigned int nullD3DAlignTexturePitch(unsigned int Bytes) {
    return (Bytes + 63u) & ~63u;
}

static unsigned int nullD3DMipDimension(unsigned int Base, unsigned int Level) {
    const unsigned int Dimension = Base >> Level;
    return Dimension == 0 ? 1 : Dimension;
}

static unsigned int nullD3DTextureLevelPitch(unsigned int Format,
                                             unsigned int Size,
                                             unsigned int Level,
                                             unsigned int RowBytes) {
    if (XGIsSwizzledFormat(Format))
        return RowBytes;
    if (Level == 0 && Size != 0) {
        const unsigned int EncodedPitch = (((Size >> 24) & 0xFFu) + 1u) << 6;
        if (EncodedPitch >= RowBytes)
            return EncodedPitch;
    }
    return nullD3DAlignTexturePitch(RowBytes);
}

static size_t nullD3DTextureLevelBytes(unsigned int Format,
                                       unsigned int Width,
                                       unsigned int Height,
                                       unsigned int Size,
                                       unsigned int Level) {
    const unsigned int BlockBytes = nullD3DFormatBlockBytes(Format);
    if (BlockBytes != 0) {
        // D3D::PixelJar::FindSurfaceWithinTexture clamps compressed mip
        // dimensions to the format's 4x4 minimum before advancing pbData.
        const unsigned int StorageWidth = Width < 4 ? 4 : Width;
        const unsigned int StorageHeight = Height < 4 ? 4 : Height;
        const unsigned int BitsPerPixel = BlockBytes == 8 ? 4 : 8;
        return ((size_t)StorageWidth * StorageHeight * BitsPerPixel) >> 3;
    }
    const unsigned int BytesPerPixel = nullD3DFormatBytesPerPixel(Format);
    if (BytesPerPixel == 0)
        return 0;
    if (XGIsSwizzledFormat(Format))
        return (size_t)Width * Height * BytesPerPixel;
    const unsigned int RowBytes = Width * BytesPerPixel;
    return (size_t)nullD3DTextureLevelPitch(Format, Size, Level, RowBytes) * Height;
}

static size_t nullD3DExternalTextureBytes(const nullD3DInfo* Info,
                                          unsigned int Size) {
    if (Info == NULL)
        return 0;
    size_t Total = 0;
    for (unsigned int Level = 0; Level < Info->Levels; ++Level) {
        const unsigned int Width = nullD3DMipDimension(Info->Width, Level);
        const unsigned int Height = nullD3DMipDimension(Info->Height, Level);
        Total += nullD3DTextureLevelBytes(Info->Format, Width, Height,
                                          Size, Level);
    }
    return Total;
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

static void nullD3DUploadExternalTexture(nullD3DInfo* Info, unsigned int Size,
                                         D3DPalette* Palette) {
    if (Info == NULL || Info->NativeTexture == NULL || Info->Bits == NULL)
        return;
    unsigned int BlockBytes = nullD3DFormatBlockBytes(Info->Format);
    if (BlockBytes != 0) {
        const unsigned char* Source = Info->Bits;
        for (unsigned int Level = 0; Level < Info->Levels; ++Level) {
            const unsigned int Width = nullD3DMipDimension(Info->Width, Level);
            const unsigned int Height = nullD3DMipDimension(Info->Height, Level);
            const unsigned int BlocksWide = (Width + 3u) / 4u;
            const unsigned int BlocksHigh = (Height + 3u) / 4u;
            const unsigned int RowBytes = BlocksWide * BlockBytes;
            const size_t SourceLevelBytes = nullD3DTextureLevelBytes(
                Info->Format, Width, Height, Size, Level);
            const unsigned int StorageHeight = Height < 4 ? 4 : Height;
            const unsigned int SourceBlocksHigh = (StorageHeight + 3u) / 4u;
            const unsigned int SourcePitch = (unsigned int)(SourceLevelBytes /
                                                              SourceBlocksHigh);
            COD3_D3D9_LOCKED_RECT Locked = {};
            if (SUCCEEDED(Info->NativeTexture->LockRect(Level, &Locked, NULL, 0))) {
                unsigned char* Destination = (unsigned char*)Locked.pBits;
                for (unsigned int y = 0; y < BlocksHigh; ++y)
                    memcpy(Destination + y * Locked.Pitch, Source + y * SourcePitch, RowBytes);
                Info->NativeTexture->UnlockRect(Level);
            }
            Source += SourceLevelBytes;
        }
        return;
    }
    unsigned int BytesPerPixel = nullD3DFormatBytesPerPixel(Info->Format);
    if (BytesPerPixel == 0)
        return;
    if (Info->Format == D3DFMT_P8) {
        if (Palette == NULL || Palette->Data == 0)
            return;
        unsigned int Entries = nullD3DPaletteEntryCount(Palette);
        if (Entries == 0)
            return;
        const unsigned char* Source = Info->Bits;
        const unsigned int* Colors = (const unsigned int*)(uintptr_t)Palette->Data;
        for (unsigned int Level = 0; Level < Info->Levels; ++Level) {
            const unsigned int Width = nullD3DMipDimension(Info->Width, Level);
            const unsigned int Height = nullD3DMipDimension(Info->Height, Level);
            const unsigned int SourcePitch = nullD3DTextureLevelPitch(
                Info->Format, Size, Level, Width);
            const size_t LinearBytes = (size_t)Width * Height;
            unsigned char* Linear = (unsigned char*)calloc(1, LinearBytes);
            if (Linear == NULL)
                return;
            if (XGIsSwizzledFormat(Info->Format))
                nullD3DUnswizzleBytes(Source, Linear, Width, Height, 1);
            else {
                for (unsigned int y = 0; y < Height; ++y)
                    memcpy(Linear + (size_t)y * Width,
                           Source + (size_t)y * SourcePitch, Width);
            }
            COD3_D3D9_LOCKED_RECT Locked = {};
            if (SUCCEEDED(Info->NativeTexture->LockRect(Level, &Locked, NULL, 0))) {
                for (unsigned int y = 0; y < Height; ++y) {
                    unsigned int* Destination = (unsigned int*)((unsigned char*)Locked.pBits +
                                                                 y * Locked.Pitch);
                    const unsigned char* Indices = Linear + (size_t)y * Width;
                    for (unsigned int x = 0; x < Width; ++x)
                        Destination[x] = Colors[Indices[x] < Entries ? Indices[x] : 0];
                }
                Info->NativeTexture->UnlockRect(Level);
            }
            free(Linear);
            Source += (size_t)SourcePitch * Height;
        }
        return;
    }
    if (XGIsSwizzledFormat(Info->Format)) {
        if (BytesPerPixel != 1 && BytesPerPixel != 2 && BytesPerPixel != 4)
            return;
        const unsigned char* Source = Info->Bits;
        for (unsigned int Level = 0; Level < Info->Levels; ++Level) {
            const unsigned int Width = nullD3DMipDimension(Info->Width, Level);
            const unsigned int Height = nullD3DMipDimension(Info->Height, Level);
            const size_t LinearBytes = (size_t)Width * Height * BytesPerPixel;
            unsigned char* Linear = (unsigned char*)calloc(1, LinearBytes);
            if (Linear == NULL)
                return;
            if (BytesPerPixel == 4 && Width >= 8 && Height >= 8)
                nullD3DUnswizzle32(Source, (unsigned int*)Linear, Width, Height);
            else
                nullD3DUnswizzleBytes(Source, Linear, Width, Height, BytesPerPixel);
            COD3_D3D9_LOCKED_RECT Locked = {};
            if (SUCCEEDED(Info->NativeTexture->LockRect(Level, &Locked, NULL, 0))) {
                for (unsigned int y = 0; y < Height; ++y)
                    memcpy((unsigned char*)Locked.pBits + y * Locked.Pitch,
                           Linear + (size_t)y * Width * BytesPerPixel,
                           Width * BytesPerPixel);
                Info->NativeTexture->UnlockRect(Level);
            }
            free(Linear);
            Source += LinearBytes;
        }
        return;
    }
    const unsigned char* Source = Info->Bits;
    for (unsigned int Level = 0; Level < Info->Levels; ++Level) {
        const unsigned int Width = nullD3DMipDimension(Info->Width, Level);
        const unsigned int Height = nullD3DMipDimension(Info->Height, Level);
        const unsigned int RowBytes = Width * BytesPerPixel;
        const unsigned int SourcePitch = nullD3DTextureLevelPitch(
            Info->Format, Size, Level, RowBytes);
        COD3_D3D9_LOCKED_RECT Locked = {};
        if (SUCCEEDED(Info->NativeTexture->LockRect(Level, &Locked, NULL, 0))) {
            unsigned char* Destination = (unsigned char*)Locked.pBits;
            for (unsigned int y = 0; y < Height; ++y)
                memcpy(Destination + y * Locked.Pitch, Source + y * SourcePitch, RowBytes);
            Info->NativeTexture->UnlockRect(Level);
        }
        Source += (size_t)SourcePitch * Height;
    }
}

static void nullD3DCreateExternalNative(nullD3DInfo* Info, unsigned int Size,
                                        unsigned int PackedFormat) {
    if (Info == NULL || Info->NativeTexture != NULL || gD3D9Device == NULL)
        return;
    if ((PackedFormat & 0x40u) != 0)
        return;
    if (nullD3DFormatBytesPerPixel(Info->Format) == 0 &&
        nullD3DFormatBlockBytes(Info->Format) == 0)
        return;
    if (FAILED(gD3D9Device->CreateTexture(Info->Width, Info->Height, Info->Levels, 0,
                                          nullD3DNativeFormat(Info->Format), D3DPOOL_MANAGED,
                                          &Info->NativeTexture, NULL)))
        return;
    Info->NativeResource = Info->NativeTexture;
    nullD3DUploadExternalTexture(Info, Size, NULL);
}

static void nullD3DReuploadPaletteTexture(unsigned int Stage) {
    if (Stage >= 4 || gNullBoundTextures[Stage] == NULL || gNullPalettes[Stage] == NULL)
        return;
    nullD3DInfo* Info = nullD3DTextureInfo(gNullBoundTextures[Stage]);
    nullD3DExternalTexture* External = nullD3DFindExternalTexture(gNullBoundTextures[Stage]);
    if (Info == NULL || Info->Format != D3DFMT_P8)
        return;
    nullD3DUploadExternalTexture(Info, External != NULL ? External->PackedSize : 0,
                                 gNullPalettes[Stage]);
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

struct nullD3DImageInfo {
    unsigned int Width;
    unsigned int Height;
    unsigned int Depth;
    unsigned int MipLevels;
    _D3DFORMAT Format;
};

static bool nullD3DDecodeImage(const void* Source, unsigned int Size,
                               unsigned int Usage, D3DTexture** Texture,
                               void* SourceInfo) {
    if (Source == NULL || Size == 0 || Texture == NULL)
        return false;

    HRESULT CoResult = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    bool CoInitialized = SUCCEEDED(CoResult);
    IWICImagingFactory* Factory = NULL;
    IWICStream* Stream = NULL;
    IWICBitmapDecoder* Decoder = NULL;
    IWICBitmapFrameDecode* Frame = NULL;
    IWICFormatConverter* Converter = NULL;
    unsigned char* Pixels = NULL;
    nullD3DTexture* HostTexture = NULL;
    unsigned int Width = 0;
    unsigned int Height = 0;
    bool Success = false;

    do {
        if (FAILED(CoResult) && CoResult != RPC_E_CHANGED_MODE)
            break;
        if (FAILED(CoCreateInstance(CLSID_WICImagingFactory, NULL,
                                    CLSCTX_INPROC_SERVER,
                                    IID_IWICImagingFactory,
                                    (void**)&Factory)))
            break;
        if (FAILED(Factory->CreateStream(&Stream)) ||
            FAILED(Stream->InitializeFromMemory((BYTE*)Source, Size)) ||
            FAILED(Factory->CreateDecoderFromStream(Stream, NULL, WICDecodeMetadataCacheOnLoad,
                                                     &Decoder)) ||
            FAILED(Decoder->GetFrame(0, &Frame)) ||
            FAILED(Frame->GetSize(&Width, &Height)) ||
            FAILED(Factory->CreateFormatConverter(&Converter)) ||
            FAILED(Converter->Initialize(Frame, GUID_WICPixelFormat32bppBGRA,
                                         WICBitmapDitherTypeNone, NULL, 0.0,
                                         WICBitmapPaletteTypeCustom)))
            break;
        if (Width == 0 || Height == 0 || Width > 0x3FFFFFFFu ||
            Height > 0x3FFFFFFFu || (size_t)Width * Height > SIZE_MAX / 4u)
            break;

        size_t PixelBytes = (size_t)Width * Height * 4u;
        Pixels = (unsigned char*)malloc(PixelBytes);
        if (Pixels == NULL || FAILED(Converter->CopyPixels(NULL, Width * 4u,
                                                            (UINT)PixelBytes, Pixels)))
            break;

        HostTexture = nullD3DCreateTexture(Width, Height, 1, 1, Usage,
                                            D3DFMT_LIN_A8R8G8B8,
                                            NULL_D3DRTYPE_TEXTURE, 0);
        if (HostTexture == NULL)
            break;
        memcpy(HostTexture->Info.Bits, Pixels, PixelBytes);
        if (HostTexture->Info.NativeTexture != NULL) {
            COD3_D3D9_LOCKED_RECT Locked = {};
            if (SUCCEEDED(HostTexture->Info.NativeTexture->LockRect(0, &Locked, NULL, 0))) {
                for (unsigned int y = 0; y < Height; ++y)
                    memcpy((unsigned char*)Locked.pBits + y * Locked.Pitch,
                           Pixels + (size_t)y * Width * 4u, Width * 4u);
                HostTexture->Info.NativeTexture->UnlockRect(0);
            }
        }
        *Texture = (D3DTexture*)HostTexture;
        if (SourceInfo != NULL) {
            nullD3DImageInfo* Info = (nullD3DImageInfo*)SourceInfo;
            Info->Width = Width;
            Info->Height = Height;
            Info->Depth = 1;
            Info->MipLevels = 1;
            Info->Format = D3DFMT_LIN_A8R8G8B8;
        }
        HostTexture = NULL;
        Success = true;
    } while (false);

    if (HostTexture != NULL) {
        if (HostTexture->Info.NativeSurface != NULL)
            HostTexture->Info.NativeSurface->Release();
        if (HostTexture->Info.NativeTexture != NULL)
            HostTexture->Info.NativeTexture->Release();
        free(HostTexture->Info.Bits);
        free(HostTexture);
    }
    free(Pixels);
    if (Converter != NULL) Converter->Release();
    if (Frame != NULL) Frame->Release();
    if (Decoder != NULL) Decoder->Release();
    if (Stream != NULL) Stream->Release();
    if (Factory != NULL) Factory->Release();
    if (CoInitialized) CoUninitialize();
    return Success;
}

static nullD3DSurface* nullD3DAllocateSurface(unsigned int Width, unsigned int Height,
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
    Surface->Object.Common = 0;
    Surface->Object.Data = (unsigned int)(uintptr_t)Surface->Info.Bits;
    Surface->Object.Lock = 0;
    Surface->Object.Format = Format;
    Surface->Object.Size = (unsigned int)Bytes;
    Surface->Object.Parent = NULL;
    Surface->Info.NativeResource = NULL;
    Surface->Info.NativeTexture = NULL;
    Surface->Info.NativeSurface = NULL;
    return Surface;
}

static nullD3DSurface* nullD3DCreateSurface(unsigned int Width, unsigned int Height,
                                            unsigned int Usage, unsigned int Format,
                                            unsigned int Persistent) {
    nullD3DSurface* Surface = nullD3DAllocateSurface(Width, Height, Usage, Format,
                                                     Persistent);
    if (Surface == NULL)
        return NULL;
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

static bool nullD3DRegisterSoftwareRasterizer(void) {
    if (gD3D9 == NULL)
        return false;

    HMODULE Rasterizer = LoadLibraryA("rgb9rast.dll");
    if (Rasterizer == NULL)
        Rasterizer = LoadLibraryA("d3dref9.dll");
    if (Rasterizer == NULL)
        return false;

    FARPROC Initialize = GetProcAddress(Rasterizer, "D3D9GetSWInfo");
    if (Initialize == NULL || FAILED(gD3D9->RegisterSoftwareDevice((void*)Initialize))) {
        FreeLibrary(Rasterizer);
        return false;
    }

    // The registered callback belongs to the loaded module and must remain
    // resident for the lifetime of the IDirect3D9 object.
    gD3D9SoftwareRasterizer = Rasterizer;
    return true;
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
    if (gD3D9Device != NULL) {
        if (gNullFrontBuffer != NULL && gNullFrontBuffer->Info.NativeSurface != NULL)
            gD3D9Device->ColorFill(gNullFrontBuffer->Info.NativeSurface, NULL, 0);
        if (gNullBackBuffer != NULL && gNullBackBuffer->Info.NativeSurface != NULL)
            gD3D9Device->ColorFill(gNullBackBuffer->Info.NativeSurface, NULL, 0);
        IDirect3DSurface9* SwapchainBackBuffer = NULL;
        if (SUCCEEDED(gD3D9Device->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO,
                                                  &SwapchainBackBuffer))) {
            gD3D9Device->ColorFill(SwapchainBackBuffer, NULL, 0);
            SwapchainBackBuffer->Release();
        }
    }
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
    for (unsigned int i = 0; i < sizeof(gNullBuffers) / sizeof(gNullBuffers[0]); ++i) {
        if (gNullBuffers[i] == NULL) {
            gNullBuffers[i] = Buffer;
            break;
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
        // NV097_SET_ALPHA_REF has an 8-bit value field, so the Xbox hardware
        // only ever sees the low byte.  SetBlendMode passes the whole packed
        // blend mode here, which is far outside D3D9's documented 0..255 and
        // leaves the driver comparing against garbage bits.
        NativeValue = Value & 0xFFu;
    } else if (Method == dword_40344) {
        NativeState = COD3_D3D9_RS_SRCBLEND;
        NativeValue = nullD3DBlendFactor(Value);
    } else if (Method == dword_40348) {
        NativeState = COD3_D3D9_RS_DESTBLEND;
        NativeValue = nullD3DBlendFactor(Value);
    } else if (Method == dword_4034C) {
        NativeState = D3DRS_BLENDFACTOR;
    } else if (Method == dword_40350) {
        NativeState = COD3_D3D9_RS_BLENDOP;
        NativeValue = nullD3DBlendOp(Value);
    } else if (Method == dword_40354) {
        NativeState = (COD3_D3D9_RENDERSTATETYPE)23;
        NativeValue = nullD3DCompareFunc(Value);
    } else if (Method == dword_40358) {
        NativeState = COD3_D3D9_RS_COLORWRITEENABLE;
        NativeValue = nullD3DColorWriteMask(Value);
    } else if (Method == dword_4035C) {
        NativeState = COD3_D3D9_RS_ZWRITEENABLE;
    } else if (Method == dword_40364) {
        NativeState = COD3_D3D9_RS_STENCILFUNC;
        NativeValue = nullD3DCompareFunc(Value);
    } else if (Method == dword_4036C) {
        NativeState = COD3_D3D9_RS_STENCILMASK;
    } else if (Method == dword_40378) {
        NativeState = COD3_D3D9_RS_STENCILPASS;
        NativeValue = nullD3DStencilOp(Value);
    } else {
        return;
    }
    gD3D9Device->SetRenderState(NativeState, NativeValue);
}
void __fastcall D3DDevice_SetVertexShaderConstant1Fast(unsigned int Register,
                                                       const void* Data) {
    if (Data == NULL || Register >= 256)
        return;
    memcpy(gD3D9VertexConstants[Register], Data, sizeof(gD3D9VertexConstants[Register]));
    gD3D9VertexConstantsValid[Register] = true;
    if (gD3D9Device != NULL)
        gD3D9Device->SetVertexShaderConstantF(Register, (const float*)Data, 1);
}
void __fastcall D3DDevice_SetVertexShaderConstantNotInlineFast(int Register,
                                                               const void* Data,
                                                               unsigned int DwordCount) {
    // The XDK entry point receives a DWORD count.  The IDA call sites pass
    // four DWORDs per float4 constant, so convert to D3D9's vector count.
    if (Data == NULL || Register < 0 || Register >= 256 || DwordCount == 0 ||
        (DwordCount & 3u) != 0)
        return;
    unsigned int VectorCount = DwordCount / 4u;
    if (VectorCount > 256u - (unsigned int)Register)
        VectorCount = 256u - (unsigned int)Register;
    if (VectorCount != 0) {
        memcpy(gD3D9VertexConstants[Register], Data,
               (size_t)VectorCount * sizeof(gD3D9VertexConstants[0]));
        for (unsigned int i = 0; i < VectorCount; ++i)
            gD3D9VertexConstantsValid[Register + i] = true;
    }
    if (gD3D9Device != NULL && VectorCount != 0)
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
void __stdcall D3DDevice_Clear(unsigned int Count, const _D3DRECT* pRects,
                               unsigned int ClearFlags, unsigned int Color,
                               float Z, unsigned int Stencil) {
    if (gD3D9Device == NULL)
        return;
    DWORD Flags = 0;
    // Xbox D3DCLEAR_TARGET is 0xF0; ZBUFFER and STENCIL retain bits 0 and 1.
    if ((ClearFlags & 0xF0u) != 0 && gD3D9RenderTarget != NULL)
        Flags |= D3DCLEAR_TARGET;
    if ((ClearFlags & 1u) != 0 && gD3D9DepthStencil != NULL)
        Flags |= D3DCLEAR_ZBUFFER;
    if ((ClearFlags & 2u) != 0 && gD3D9DepthStencil != NULL)
        Flags |= D3DCLEAR_STENCIL;
    if (Flags != 0)
        gD3D9Device->Clear(Count, pRects, Flags, Color, Z, Stencil);
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
    if (gD3D9SelectedVertexFormatValid)
        nullD3DBuildVertexDeclaration(&gD3D9SelectedVertexFormat);
    if (gD3D9Device == NULL || gD3D9VertexDeclaration == NULL || IndexData == NULL)
        return;
    COD3_D3D9_PRIMITIVETYPE NativePrimitive = nullD3DPrimitiveType(PrimitiveType);
    unsigned int PrimitiveCount = nullD3DPrimitiveCount(PrimitiveType, VertexCount);
    if (NativePrimitive == COD3_D3D9_PT_FORCE_DWORD || PrimitiveCount == 0)
        return;
    nullD3DInfo* Info = gD3D9IndexInfo;
    uintptr_t IndexAddress = (uintptr_t)IndexData;
    IDirect3DIndexBuffer9* NativeIndex = gD3D9IndexBuffer;
    if (Info == NULL || NativeIndex == NULL) {
        Info = nullD3DFindBufferByData(IndexData);
        NativeIndex = Info == NULL ? NULL :
            (IDirect3DIndexBuffer9*)Info->NativeResource;
    }
    if (Info == NULL || NativeIndex == NULL) {
        if (gD3D9VertexData[0] == NULL || gD3D9VertexStrides[0] == 0)
            return;
        unsigned int MaxIndex = 0;
        for (unsigned int i = 0; i < VertexCount; ++i) {
            if (IndexData[i] > MaxIndex)
                MaxIndex = IndexData[i];
        }
        nullD3DSetShaderMatrixTransform();
        gD3D9Device->SetVertexDeclaration(gD3D9VertexDeclaration);
        const void* VertexData = gD3D9VertexData[0] + gD3D9VertexOffsets[0];
        gD3D9Device->DrawIndexedPrimitiveUP(
            NativePrimitive, 0, MaxIndex + 1, PrimitiveCount, IndexData,
            COD3_D3D9_FMT_INDEX16, VertexData, gD3D9VertexStrides[0]);
        return;
    }
    unsigned int StartIndex = 0;
    if (Info != NULL && Info->Bits != NULL &&
        IndexAddress >= (uintptr_t)Info->Bits &&
        IndexAddress < (uintptr_t)Info->Bits + Info->SizeBytes)
        StartIndex = (unsigned int)((IndexAddress - (uintptr_t)Info->Bits) / sizeof(unsigned short));
    nullD3DSyncIndexBuffer(NativeIndex, Info);
    for (unsigned int i = 0; i < 4; ++i)
        nullD3DSyncVertexBuffer(gD3D9VertexBuffers[i], gD3D9VertexInfos[i]);
    nullD3DSetShaderMatrixTransform();
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
void __stdcall D3DDevice_DrawVertices(_D3DPRIMITIVETYPE PrimitiveType,
                                       unsigned int StartVertex,
                                       unsigned int VertexCount) {
    if (gD3D9SelectedVertexFormatValid)
        nullD3DBuildVertexDeclaration(&gD3D9SelectedVertexFormat);
    if (gD3D9Device == NULL || gD3D9VertexDeclaration == NULL)
        return;
    COD3_D3D9_PRIMITIVETYPE NativePrimitive = nullD3DPrimitiveType(PrimitiveType);
    unsigned int PrimitiveCount = nullD3DPrimitiveCount(PrimitiveType, VertexCount);
    if (NativePrimitive == COD3_D3D9_PT_FORCE_DWORD || PrimitiveCount == 0)
        return;
    nullD3DSetShaderMatrixTransform();
    gD3D9Device->SetVertexDeclaration(gD3D9VertexDeclaration);
    if (gD3D9VertexBuffers[0] == NULL || gD3D9VertexInfos[0] == NULL) {
        if (gD3D9VertexData[0] == NULL || gD3D9VertexStrides[0] == 0)
            return;
        const unsigned char* VertexData =
            gD3D9VertexData[0] + gD3D9VertexOffsets[0] +
            StartVertex * gD3D9VertexStrides[0];
        gD3D9Device->DrawPrimitiveUP(NativePrimitive, PrimitiveCount,
                                      VertexData, gD3D9VertexStrides[0]);
        return;
    }
    nullD3DSyncVertexBuffer(gD3D9VertexBuffers[0], gD3D9VertexInfos[0]);
    gD3D9Device->SetStreamSource(0, gD3D9VertexBuffers[0],
                                 gD3D9VertexOffsets[0], gD3D9VertexStrides[0]);
    gD3D9Device->DrawPrimitive(NativePrimitive, StartVertex, PrimitiveCount);
}
void __stdcall D3DDevice_DrawVerticesUP(_D3DPRIMITIVETYPE PrimitiveType,
                                         unsigned int VertexCount,
                                         const void* VertexData,
                                         unsigned int VertexStride) {
    if (gD3D9SelectedVertexFormatValid)
        nullD3DBuildVertexDeclaration(&gD3D9SelectedVertexFormat);
    if (gD3D9Device == NULL || gD3D9VertexDeclaration == NULL || VertexData == NULL)
        return;
    COD3_D3D9_PRIMITIVETYPE NativePrimitive = nullD3DPrimitiveType(PrimitiveType);
    unsigned int PrimitiveCount = nullD3DPrimitiveCount(PrimitiveType, VertexCount);
    if (NativePrimitive == COD3_D3D9_PT_FORCE_DWORD || PrimitiveCount == 0)
        return;
    nullD3DSetShaderMatrixTransform();
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

static void nullD3DNormalizeScreenDepth(unsigned int* Vertices,
                                        unsigned int VertexCount,
                                        unsigned int StrideDwords) {
    // nglDxViewToScreenZ produces the Xbox 24-bit screen-depth range.  The
    // Xbox vertex shader applies the 16777215 scale before rasterization;
    // the fixed-function D3D9 fallback needs the equivalent normalized Z.
    const float XboxDepthMax = 16777215.0f;
    for (unsigned int i = 0; i < VertexCount; ++i) {
        float* Z = reinterpret_cast<float*>(&Vertices[i * StrideDwords + 2]);
        if (*Z <= 0.0f)
            *Z = 0.0f;
        else if (*Z >= XboxDepthMax)
            *Z = 1.0f;
        else
            *Z /= XboxDepthMax;
    }
}

static void nullD3DSetFixedFunctionFVF(DWORD FVF, bool FontPacket = false) {
    // Push-buffer packets use the native D3D9 fixed-function declaration.
    // Clear both programmable stages before selecting the FVF so DrawPrimitiveUP
    // cannot retain an Xbox shader handle from the preceding NGL node.
    gD3D9Device->SetVertexShader(NULL);
    gD3D9Device->SetPixelShader(NULL);
    gD3D9Device->SetRenderState(D3DRS_LIGHTING, FALSE);
    gD3D9BuiltVertexFormatValid = false;
    gD3D9Device->SetVertexDeclaration(NULL);
    gD3D9Device->SetFVF(FVF);

    // The Xbox quad path selects either nglGpuTexColPixelShader or
    // nglGpuColPixelShader immediately before emitting this packet.  The
    // D3D9 fallback has no Xbox shader microcode, so reproduce that choice
    // with the equivalent fixed-function texture-stage operations.
    const bool HasTexture = gNullBoundTextures[0] != NULL;
    nullD3DInfo* TextureInfo = HasTexture
        ? nullD3DTextureInfo(gNullBoundTextures[0]) : NULL;
    for (unsigned int Stage = 1; Stage < 4; ++Stage) {
        gNullBoundTextures[Stage] = NULL;
        gNullTextureWidths[Stage] = 0;
        gNullTextureHeights[Stage] = 0;
        gNullPalettes[Stage] = NULL;
        gD3D9Device->SetTexture(Stage, NULL);
    }
    // NGL fonts use an A8 glyph mask.  The Xbox font pixel shader preserves
    // the vertex RGB and uses the sampled alpha for coverage; multiplying the
    // vertex RGB by an A8 texture's undefined/zero RGB channels makes text
    // disappear in the D3D9 fixed-function fallback.
    const bool AlphaMask = TextureInfo != NULL && TextureInfo->Format == D3DFMT_A8;
    const bool TexturedPCUV = FVF == (D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1);
    const DWORD ColorOp = TexturedPCUV
        ? (HasTexture && !AlphaMask ? D3DTOP_MODULATE : D3DTOP_SELECTARG2)
        : D3DTOP_SELECTARG1;
    // Alpha is always coverage * vertex alpha.  The Xbox TexCol shader
    // modulates both channels, and FEText/FEMultiLineText encode the
    // selection highlight purely in the vertex alpha (0xFF selected,
    // 0x80 dimmed), so selecting only the sampled texture alpha here
    // would collapse every string to full opacity.
    (void)FontPacket;
    const DWORD AlphaOp = TexturedPCUV
        ? (HasTexture ? D3DTOP_MODULATE : D3DTOP_SELECTARG2)
        : D3DTOP_SELECTARG1;
    gD3D9Device->SetTextureStageState(0, COD3_D3D9_TSS_COLOROP, ColorOp);
    gD3D9Device->SetTextureStageState(0, COD3_D3D9_TSS_ALPHAOP, AlphaOp);
    gD3D9Device->SetTextureStageState(0, COD3_D3D9_TSS_COLORARG1,
                                       HasTexture ? D3DTA_TEXTURE : D3DTA_DIFFUSE);
    gD3D9Device->SetTextureStageState(0, COD3_D3D9_TSS_COLORARG2, D3DTA_DIFFUSE);
    gD3D9Device->SetTextureStageState(0, COD3_D3D9_TSS_ALPHAARG1,
                                       HasTexture ? D3DTA_TEXTURE : D3DTA_DIFFUSE);
    gD3D9Device->SetTextureStageState(0, COD3_D3D9_TSS_ALPHAARG2, D3DTA_DIFFUSE);
    for (unsigned int Stage = 1; Stage < 4; ++Stage) {
        gD3D9Device->SetTextureStageState(Stage, COD3_D3D9_TSS_COLOROP,
                                           D3DTOP_DISABLE);
        gD3D9Device->SetTextureStageState(Stage, COD3_D3D9_TSS_ALPHAOP,
                                           D3DTOP_DISABLE);
    }
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
            nullD3DNormalizeScreenDepth(NormalizedVertices, 4, 6);
            nullD3DNormalizeTexelCoordinates(NormalizedVertices, 4, 6, 4);
            nullD3DSetFixedFunctionFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1);
            gD3D9Device->DrawPrimitiveUP(COD3_D3D9_PT_TRIANGLEFAN, 2,
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
            nullD3DNormalizeScreenDepth(NormalizedVertices, 4, 5);
            nullD3DNormalizeTexelCoordinates(NormalizedVertices, 4, 5, 3);
            nullD3DSetFixedFunctionFVF(D3DFVF_XYZ | D3DFVF_TEX1);
            gD3D9Device->DrawPrimitiveUP(COD3_D3D9_PT_TRIANGLEFAN, 2,
                                         NormalizedVertices, 20);
        }
        return;
    }

    // Font strings start with the shared inline-array command and contain a
    // sequence of PCUV glyph quads.  The count encoded by each command is
    // the number of DWORDs in its following vertex array.
    Cursor += 2;
    nullD3DSetFixedFunctionFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1, true);
    while (Cursor + 1 < End) {
        unsigned int Command = *Cursor++;
        if (Command == 0 && *Cursor == 0)
            break;
        // NGL emits 0x40001818 as the inline-array command base and stores
        // the vertex DWORD count in bits 18+.  Keep the full command base
        // here so multi-glyph strings are accepted and decoded correctly.
        if ((Command & 0x3FFFFu) != 0x1818u || Command < 0x40001818u)
            return;
        unsigned int DwordCount = (Command - 0x40001818u) >> 18;
        if (DwordCount == 0 || (DwordCount % 24u) != 0 || Cursor + DwordCount > End)
            return;
        for (unsigned int Offset = 0; Offset < DwordCount; Offset += 24) {
            unsigned int NormalizedVertices[24];
            memcpy(NormalizedVertices, Cursor + Offset, sizeof(NormalizedVertices));
            nullD3DNormalizeScreenDepth(NormalizedVertices, 4, 6);
            nullD3DNormalizeTexelCoordinates(NormalizedVertices, 4, 6, 4);
            gD3D9Device->DrawPrimitiveUP(COD3_D3D9_PT_TRIANGLEFAN, 2,
                                         NormalizedVertices, 24);
        }
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
void __stdcall D3DDevice_LoadVertexShaderProgram(const unsigned int* Microcode, unsigned int) {
    if (gD3D9Device == NULL)
        return;
    gD3D9ShaderNeedsViewportInverse = nullD3DProgramUsesHomogeneousDivide(Microcode);
    IDirect3DVertexShader9* shader = nullD3DCompileNV2AVertexShader(gD3D9Device, Microcode);
    gD3D9Device->SetVertexShader(shader);
}
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
void __stdcall D3DDevice_SetPalette(unsigned int Stage, D3DPalette* Palette) {
    if (Stage >= 4)
        return;
    gNullPalettes[Stage] = Palette;
    nullD3DReuploadPaletteTexture(Stage);
}
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
    bool ReleaseDefaultTarget = false;
    nullD3DInfo* RenderInfo = nullD3DFindInfo(RenderTarget);
    nullD3DInfo* ZInfo = nullD3DFindInfo(ZBuffer);
    if (RenderTarget == (D3DSurface*)gNullBackBuffer && gD3D9Device != NULL &&
        SUCCEEDED(gD3D9Device->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO,
                                              &NativeRenderTarget)))
        ReleaseDefaultTarget = true;
    else if (RenderInfo != NULL)
        NativeRenderTarget = RenderInfo->NativeSurface;
    else if (RenderTarget == NULL && gD3D9Device != NULL &&
             SUCCEEDED(gD3D9Device->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO,
                                                   &NativeRenderTarget)))
        ReleaseDefaultTarget = true;
    if (ZInfo != NULL)
        NativeZBuffer = ZInfo->NativeSurface;
    if (gD3D9Device != NULL && NativeRenderTarget != NULL &&
        (NativeRenderTarget != gD3D9RenderTarget || NativeZBuffer != gD3D9DepthStencil)) {
        gD3D9Device->SetRenderTarget(0, NativeRenderTarget);
        gD3D9Device->SetDepthStencilSurface(NativeZBuffer);
        gD3D9RenderTarget = NativeRenderTarget;
        gD3D9DepthStencil = NativeZBuffer;
    }
    if (ReleaseDefaultTarget)
        NativeRenderTarget->Release();
}
void __stdcall D3DDevice_SetShaderConstantMode(unsigned int) {}
static void nullD3DClearBoundTextures(void) {
    for (unsigned int Stage = 0; Stage < 4; ++Stage) {
        gNullBoundTextures[Stage] = NULL;
        gNullTextureWidths[Stage] = 0;
        gNullTextureHeights[Stage] = 0;
        gNullPalettes[Stage] = NULL;
        if (gD3D9Device != NULL)
            gD3D9Device->SetTexture(Stage, NULL);
    }
}
void __stdcall D3DDevice_SetTexture(unsigned int Stage, D3DBaseTexture* Texture) {
    if (Stage >= 4)
        return;
    gNullBoundTextures[Stage] = Texture;
    if (gD3D9Device == NULL)
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
    nullD3DReuploadPaletteTexture(Stage);
}
int __stdcall D3DDevice_SetTextureState_ParameterCheck(unsigned int Stage,
                                                        _D3DTEXTURESTAGESTATETYPE Type,
                                                        unsigned int Value) {
    if (gD3D9Device == NULL || Stage >= 4)
        return 0;
    DWORD Sampler = 0;
    DWORD NativeValue = Value;
    COD3_D3D9_TEXTURESTAGESTATETYPE NativeStageState = COD3_D3D9_TSS_COLOROP;
    bool SamplerState = true;
    // The callers use the Xbox enum in d3d8.h (ADDRESSU=0 through
    // MAXANISOTROPY=8).  d3d9_compat.h remaps the names above to D3D9's
    // texture-stage values while this file is compiled, so switching on the
    // names silently dispatches Xbox 3/4/5 to unrelated D3D9 states.
    switch ((unsigned int)Type) {
    case 0u: Sampler = D3DSAMP_ADDRESSU; break;
    case 1u: Sampler = D3DSAMP_ADDRESSV; break;
    case 2u: Sampler = D3DSAMP_ADDRESSW; break;
    case 3u: Sampler = D3DSAMP_MAGFILTER; break;
    case 4u: Sampler = D3DSAMP_MINFILTER; break;
    case 5u: Sampler = D3DSAMP_MIPFILTER; break;
    case 6u: Sampler = D3DSAMP_MIPMAPLODBIAS; break;
    case 7u: Sampler = D3DSAMP_MAXMIPLEVEL; break;
    case 8u: Sampler = D3DSAMP_MAXANISOTROPY; break;
    case 12u: // D3DTSS_DEFERRED_TEXTURE_STATE_MAX; no host state to emit.
    case 9u:  // COLORKEYOP
    case 10u: // COLORSIGN
    case 11u: // ALPHAKILL
        return 0;
    case 16u: // ALPHAOP
        SamplerState = false;
        NativeStageState = COD3_D3D9_TSS_ALPHAOP;
        NativeValue = Value;
        break;
    case 14u: // COLORARG1
    case 15u: // COLORARG2
    case 18u: // ALPHAARG1
    case 19u: // ALPHAARG2
        SamplerState = false;
        NativeStageState = (Type == (_D3DTEXTURESTAGESTATETYPE)14u) ?
            COD3_D3D9_TSS_COLORARG1 :
            (Type == (_D3DTEXTURESTAGESTATETYPE)15u) ? COD3_D3D9_TSS_COLORARG2 :
            (Type == (_D3DTEXTURESTAGESTATETYPE)18u) ? COD3_D3D9_TSS_ALPHAARG1 :
            COD3_D3D9_TSS_ALPHAARG2;
        NativeValue = Value;
        break;
    case 21u: // TEXTURETRANSFORMFLAGS; D3D9 has no sampler equivalent here.
        return 0;
    default:
        return 0;
    }
    if (SamplerState)
        gD3D9Device->SetSamplerState(Stage, (D3DSAMPLERSTATETYPE)Sampler, NativeValue);
    else
        gD3D9Device->SetTextureStageState(Stage, NativeStageState, NativeValue);
    return 0;
}
void __stdcall D3DDevice_SetVertexShader(unsigned int Handle) {
    if (gD3D9Device == NULL)
        return;
    if (Handle == 0)
        gD3D9ShaderNeedsViewportInverse = false;
    gD3D9Device->SetVertexShader(reinterpret_cast<IDirect3DVertexShader9*>(Handle));
}
void __stdcall D3DDevice_SetVerticalBlankCallback(void (*Callback)(_D3DVBLANKDATA*)) { gNullVBlankCallback = Callback; }
void __stdcall D3DDevice_BeginScene(void) {
    if (gD3D9Device == NULL)
        return;
    nullD3DClearBoundTextures();
    if (gD3D9SceneActive)
        return;
    if (SUCCEEDED(gD3D9Device->BeginScene()))
        gD3D9SceneActive = true;
}
void __stdcall D3DDevice_EndScene(void) {
    if (gD3D9Device == NULL || !gD3D9SceneActive)
        return;
    gD3D9Device->EndScene();
    gD3D9SceneActive = false;
}
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
    // The Xbox title receives its input/window servicing from the platform
    // shell.  The Win32 presentation boundary must drain the host queue so
    // the visible D3D9 window remains responsive while the game frame loop
    // continues through the same Swap entry point.
    MSG Message;
    while (PeekMessageA(&Message, NULL, 0, 0, PM_REMOVE) != FALSE) {
        TranslateMessage(&Message);
        DispatchMessageA(&Message);
    }
    if (gNullVBlankCallback != NULL) {
        _D3DVBLANKDATA Data = { 0, 0, 0 };
        gNullVBlankCallback(&Data);
    }
}
void __stdcall D3DDevice_SwitchTexture(unsigned int Method, unsigned int Data,
                                       unsigned int Format) {
    unsigned int Stage = (Method - (8300u << 6)) >> 6;
    if (Stage >= 4 || DTE[Stage] != Method)
        return;
    D3DBaseTexture* Texture = nullD3DFindTextureByData(Data, Format);
    if (Texture != NULL)
        D3DDevice_SetTexture(Stage, Texture);
}
unsigned int __stdcall D3DPalette_Lock2(D3DPalette* Palette, unsigned int) {
    return Palette == NULL ? 0 : Palette->Data;
}
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
    const size_t SourceBytes = nullD3DExternalTextureBytes(Info, Texture->Size);
    Info->SizeBytes = SourceBytes > 0xFFFFFFFFu ? 0xFFFFFFFFu :
                      (unsigned int)SourceBytes;
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
    nullD3DSurface* Surface = nullD3DAllocateSurface(Info->Width, Info->Height,
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
int __stdcall D3DXCreateTextureFromFileInMemoryEx(void*, const void* Source, unsigned int Size,
                                                  unsigned int Width, unsigned int Height,
                                                  unsigned int Levels, unsigned int Usage,
                                                  _D3DFORMAT Format, unsigned int, unsigned int,
                                                  unsigned int, unsigned int, void* SourceInfo,
                                                  void*,
                                                  D3DTexture** Texture) {
    if (Texture != NULL)
        *Texture = NULL;
    if (nullD3DDecodeImage(Source, Size, Usage, Texture, SourceInfo))
        return 0;
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
            // In a windowed D3D9 swap chain, zero lets the runtime derive the
            // back-buffer size from the client area.
            NativeParams.BackBufferWidth = 0;
            NativeParams.BackBufferHeight = 0;
            // D3D9 requires UNKNOWN for a windowed swap chain so it can use
            // the desktop format; the Xbox presentation format is not a
            // valid Win32 windowed CreateDevice contract.
            NativeParams.BackBufferFormat = COD3_D3D9_FMT_UNKNOWN;
            NativeParams.BackBufferCount = 1;
            NativeParams.SwapEffect = COD3_D3D9_SWP_DISCARD;
            NativeParams.hDeviceWindow = nullD3DCreateWindow();
            NativeParams.Windowed = TRUE;
            NativeParams.EnableAutoDepthStencil = FALSE;
            NativeParams.AutoDepthStencilFormat = COD3_D3D9_FMT_UNKNOWN;
            NativeParams.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;
            HRESULT Result = gD3D9->CreateDevice(D3DADAPTER_DEFAULT, COD3_D3D9_DEVTYPE_HAL,
                                                  NativeParams.hDeviceWindow,
                                                  D3DCREATE_HARDWARE_VERTEXPROCESSING,
                                                  &NativeParams, &gD3D9Device);
            if (FAILED(Result)) {
                Result = gD3D9->CreateDevice(D3DADAPTER_DEFAULT, COD3_D3D9_DEVTYPE_HAL,
                                             NativeParams.hDeviceWindow,
                                             D3DCREATE_SOFTWARE_VERTEXPROCESSING,
                                             &NativeParams, &gD3D9Device);
            }
            if (FAILED(Result) && nullD3DRegisterSoftwareRasterizer()) {
                Result = gD3D9->CreateDevice(D3DADAPTER_DEFAULT, COD3_D3D9_DEVTYPE_SW,
                                             NativeParams.hDeviceWindow,
                                             D3DCREATE_SOFTWARE_VERTEXPROCESSING,
                                             &NativeParams, &gD3D9Device);
            }
            if (FAILED(Result)) {
                gD3D9->Release();
                gD3D9 = NULL;
            }
            if (gD3D9Device != NULL)
                gD3D9Device->SetRenderState(D3DRS_LIGHTING, FALSE);
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
HRESULT __stdcall LiveEngine_DoWork(unsigned int a0, unsigned int a1) { return 0; }
HRESULT __stdcall LiveEngine_EnableFeature(unsigned int a0, unsigned int a1) { return 0; }
HRESULT __stdcall LiveEngine_EndFeature(unsigned int a0) { return 0; }
HRESULT __stdcall LiveEngine_GetExitInfo(unsigned int a0, unsigned int a1) { return 0; }
void* __stdcall LiveEngine_GetFeatureInterface(unsigned int a0, unsigned int a1, unsigned int a2, unsigned int a3) { return nullptr; }
HRESULT __stdcall LiveEngine_GetNotifications(unsigned int a0, unsigned int a1, unsigned int a2, unsigned int a3) { return 0; }
HRESULT __stdcall LiveEngine_LogOff(unsigned int a0) { return 0; }
void __stdcall LiveEngine_NotificationSetState(unsigned int a0, unsigned int a1, unsigned int a2, unsigned int a3, unsigned int a4, unsigned int a5, unsigned int a6) {}
HRESULT __stdcall LiveEngine_Reboot(unsigned int a0, unsigned int a1) { return 0; }
void __stdcall LiveEngine_Release(unsigned int a0) {}
HRESULT __stdcall LiveEngine_Render(unsigned int a0, unsigned int a1) { return 0; }
HRESULT __stdcall LiveEngine_SetInput(unsigned int a0, unsigned int a1, unsigned int a2) { return 0; }
HRESULT __stdcall LiveEngine_SetProperty(unsigned int a0, unsigned int a1, unsigned int a2) { return 0; }
HRESULT __stdcall LiveEngine_SetUIPlugin(unsigned int a0, unsigned int a1) { return 0; }
HRESULT __stdcall LiveEngine_StartFeature(unsigned int a0, unsigned int a1, unsigned int a2) { return 0; }
HRESULT __stdcall LiveEngine_UseVoiceMail(unsigned int a0, unsigned int a1) { return 0; }
HRESULT __stdcall UIXCreateLiveEngine(unsigned int a0, unsigned int a1, unsigned int a2) { return 0; }
HRESULT __stdcall UIXCreateUIPlugin(unsigned int a0, unsigned int a1) { return 0; }
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
void __stdcall XGSetPaletteHeader(_D3DPALETTESIZE Size, D3DPalette* Palette, void* Data) {
    if (Palette == NULL)
        return;
    unsigned int Entries = 256u >> (unsigned int)Size;
    for (unsigned int i = 0; i < sizeof(gNullPaletteInfos) / sizeof(gNullPaletteInfos[0]); ++i) {
        if (gNullPaletteInfos[i].Object == Palette || gNullPaletteInfos[i].Object == NULL) {
            gNullPaletteInfos[i].Object = Palette;
            gNullPaletteInfos[i].Entries = Entries;
            break;
        }
    }
    Palette->Data = (unsigned int)(uintptr_t)Data;
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
HRESULT __stdcall XHVEngine_DoWork(unsigned int a0) { return 0; }
void __stdcall XHVEngine_EnableProcessingMode(unsigned int a0, unsigned int a1) {}
int __stdcall XHVEngine_IsTalking(unsigned int a0, unsigned int a1, unsigned int a2, unsigned int a3) { return 0; }
HRESULT __stdcall XHVEngine_RegisterLocalTalker(unsigned int a0, unsigned int a1) { return 0; }
HRESULT __stdcall XHVEngine_RegisterRemoteTalker(unsigned int a0, unsigned int a1, unsigned int a2, unsigned int a3) { return 0; }
void __stdcall XHVEngine_Release(unsigned int a0) {}
void __stdcall XHVEngine_SetCallbackInterface(unsigned int a0, unsigned int a1) {}
void __stdcall XHVEngine_SetMixBinMapping(unsigned int a0, unsigned int a1, unsigned int a2, unsigned int a3, unsigned int a4, unsigned int a5) {}
void __stdcall XHVEngine_SetPlaybackPriority(unsigned int a0, unsigned int a1, unsigned int a2, unsigned int a3, unsigned int a4, unsigned int a5) {}
void __stdcall XHVEngine_SetProcessingMode(unsigned int a0, unsigned int a1, unsigned int a2) {}
void __stdcall XHVEngine_SubmitIncomingVoicePacket(unsigned int a0, unsigned int a1, unsigned int a2, unsigned int a3, unsigned int a4, unsigned int a5) {}
void __stdcall XHVEngine_UnregisterRemoteTalker(unsigned int a0, unsigned int a1, unsigned int a2, unsigned int a3) {}
HRESULT __stdcall XHVEngineCreate(unsigned int a0, unsigned int a1) { return 0; }
void __stdcall XInputGetState(unsigned int a0, unsigned int a1) {}
void __stdcall XInputSetState(unsigned int a0, unsigned int a1) {}
int __stdcall XNetQosListen(unsigned int a0, unsigned int a1, unsigned int a2, unsigned int a3, unsigned int a4) { return 0; }
int __stdcall XNetRegisterKey(unsigned int a0, unsigned int a1) { return 0; }
int __stdcall XNetUnregisterKey(unsigned int a0) { return 0; }
unsigned int __stdcall XOnlineGetLogonUsers(void) { return 0; }
HRESULT __stdcall XOnlineMatchSessionCreate(unsigned int a0, unsigned int a1, unsigned int a2, unsigned int a3, unsigned int a4, unsigned int a5, unsigned int a6, unsigned int a7) { return 0; }
HRESULT __stdcall XOnlineMatchSessionDelete(unsigned int a0, unsigned int a1, unsigned int a2, unsigned int a3) { return 0; }
HRESULT __stdcall XOnlineMatchSessionGetInfo(unsigned int a0, unsigned int a1, unsigned int a2) { return 0; }
HRESULT __stdcall XOnlineMatchSessionUpdate(unsigned int a0, unsigned int a1, unsigned int a2, unsigned int a3, unsigned int a4, unsigned int a5, unsigned int a6, unsigned int a7, unsigned int a8, unsigned int a9) { return 0; }
HRESULT __stdcall XOnlineMutelistGet(unsigned int a0, unsigned int a1, unsigned int a2, unsigned int a3, unsigned int a4, unsigned int a5) { return 0; }
HRESULT __stdcall XOnlineSaveLogonState(unsigned int a0) { return 0; }
HRESULT __stdcall XOnlineStartup(unsigned int a0) { return 0; }
HRESULT __stdcall XOnlineTaskClose(unsigned int a0) { return 0; }
DWORD __stdcall XOnlineTaskContinue(unsigned int a0) { return 0; }

}  // extern "C"

// LNK2001-only imports surfaced after the first pass
extern "C" {
void __stdcall D3DDevice_SelectVertexShaderDirect(_D3DVERTEXATTRIBUTEFORMAT* Format,
                                                   unsigned int) {
    if (gD3D9Device == NULL || Format == NULL)
        return;

    // Shader selection and stream selection are separate on the Xbox API.
    // The common gpuSetVertexShader::Inputs declaration is intentionally
    // all-END; the real mesh declaration arrives through
    // D3DDevice_SetVertexShaderInputDirect.  Do not replace that declaration
    // with an empty one, or the next mesh draw loses all of its inputs.
    bool hasInput = false;
    for (unsigned int i = 0; i < 16; ++i) {
        if (Format->Input[i].Format == 2)
            break;
        hasInput = true;
    }
    if (!hasInput)
        return;
    memcpy(&gD3D9SelectedVertexFormat, Format, sizeof(gD3D9SelectedVertexFormat));
    gD3D9SelectedVertexFormatValid = true;
}
void __stdcall D3DDevice_SetPixelShaderProgram(const _D3DPixelShaderDef* Definition) {
    // Xbox pixel-shader microcode is not present in the Win32 reconstruction.
    // Keep the native D3D9 path deterministic: the world/sky vertex programs
    // still provide position and interpolants, while the fixed-function stage
    // supplies the equivalent texture-times-diffuse base pass.
    if (gD3D9Device == NULL)
        return;
    IDirect3DPixelShader9* WorldShader =
        nullD3DCompileNV2AWorldPixelShader(gD3D9Device, Definition);
    gD3D9NV2APixelShaderActive = WorldShader != NULL;
    if (WorldShader != NULL) {
        gD3D9Device->SetPixelShader(WorldShader);
        DWORD FogColor = 0;
        gD3D9Device->GetRenderState(COD3_D3D9_RS_FOGCOLOR, &FogColor);
        const float FogColorF[4] = {
            ((FogColor >> 16) & 0xffu) / 255.0f,
            ((FogColor >> 8) & 0xffu) / 255.0f,
            (FogColor & 0xffu) / 255.0f,
            ((FogColor >> 24) & 0xffu) / 255.0f,
        };
        gD3D9Device->SetPixelShaderConstantF(0, FogColorF, 1);
        return;
    }
    gD3D9Device->SetPixelShader(NULL);
    gD3D9Device->SetRenderState(D3DRS_LIGHTING, FALSE);

    const bool HasTexture = gNullBoundTextures[0] != NULL;
    const bool HasLightmap = gNullBoundTextures[1] != NULL;
    const DWORD ColorOp = HasTexture ? D3DTOP_MODULATE : D3DTOP_SELECTARG2;
    const DWORD AlphaOp = HasTexture ? D3DTOP_MODULATE : D3DTOP_SELECTARG2;
    gD3D9Device->SetTextureStageState(0, COD3_D3D9_TSS_COLOROP, ColorOp);
    gD3D9Device->SetTextureStageState(0, COD3_D3D9_TSS_COLORARG1,
                                       HasTexture ? D3DTA_TEXTURE : D3DTA_DIFFUSE);
    gD3D9Device->SetTextureStageState(0, COD3_D3D9_TSS_COLORARG2, D3DTA_DIFFUSE);
    gD3D9Device->SetTextureStageState(0, COD3_D3D9_TSS_ALPHAOP, AlphaOp);
    gD3D9Device->SetTextureStageState(0, COD3_D3D9_TSS_ALPHAARG1,
                                       HasTexture ? D3DTA_TEXTURE : D3DTA_DIFFUSE);
    gD3D9Device->SetTextureStageState(0, COD3_D3D9_TSS_ALPHAARG2, D3DTA_DIFFUSE);
    // cdWorldPixel's lightmapped variants combine the diffuse stage with the
    // stage-1 lightmap.  Keep the fixed-function fallback equivalent when the
    // world material supplied both resources; other shaders must not inherit
    // a stale stage-1 operation.
    gD3D9Device->SetTextureStageState(1, COD3_D3D9_TSS_COLOROP,
                                       HasLightmap ? D3DTOP_MODULATE : D3DTOP_DISABLE);
    gD3D9Device->SetTextureStageState(1, COD3_D3D9_TSS_COLORARG1,
                                       HasLightmap ? D3DTA_CURRENT : D3DTA_DIFFUSE);
    gD3D9Device->SetTextureStageState(1, COD3_D3D9_TSS_COLORARG2,
                                       D3DTA_TEXTURE);
    gD3D9Device->SetTextureStageState(1, COD3_D3D9_TSS_ALPHAOP,
                                       HasLightmap ? D3DTOP_MODULATE : D3DTOP_DISABLE);
    gD3D9Device->SetTextureStageState(1, COD3_D3D9_TSS_ALPHAARG1,
                                       HasLightmap ? D3DTA_CURRENT : D3DTA_DIFFUSE);
    gD3D9Device->SetTextureStageState(1, COD3_D3D9_TSS_ALPHAARG2,
                                       D3DTA_TEXTURE);
}
void __stdcall D3DDevice_SetRenderState_CullMode(unsigned int Value) {
    if (gD3D9Device != NULL)
        gD3D9Device->SetRenderState(COD3_D3D9_RS_CULLMODE,
                                     nullD3DCullMode(Value));
}
void __stdcall D3DDevice_SetRenderState_FogColor(unsigned int Value) {
    if (gD3D9Device == NULL)
        return;
    gD3D9Device->SetRenderState(COD3_D3D9_RS_FOGCOLOR, Value);
    if (gD3D9NV2APixelShaderActive) {
        const float FogColorF[4] = {
            ((Value >> 16) & 0xffu) / 255.0f,
            ((Value >> 8) & 0xffu) / 255.0f,
            (Value & 0xffu) / 255.0f,
            ((Value >> 24) & 0xffu) / 255.0f,
        };
        gD3D9Device->SetPixelShaderConstantF(0, FogColorF, 1);
    }
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
    // State is the Xbox D3D8 selector.  d3d9_compat.h remaps the D3DRS_*
    // names in this translation unit to D3D9 values, so use the verified
    // Xbox numbers explicitly at this ABI boundary.
    switch (State) {
    case 0x3Au: // D3DRS_ALPHAFUNC
        NativeState = COD3_D3D9_RS_ALPHAFUNC;
        NativeValue = nullD3DCompareFunc(Value);
        break;
    case 0x3Bu: // D3DRS_ALPHABLENDENABLE
        NativeState = COD3_D3D9_RS_ALPHABLENDENABLE;
        break;
    case 0x3Cu: // D3DRS_ALPHATESTENABLE
        NativeState = COD3_D3D9_RS_ALPHATESTENABLE;
        break;
    case 0x3Du: // D3DRS_ALPHAREF
        NativeState = COD3_D3D9_RS_ALPHAREF;
        NativeValue = Value & 0xFFu;  // 8-bit field on NV2A; see SetRenderState_Simple
        break;
    case 0x3Eu: // D3DRS_SRCBLEND
        NativeState = COD3_D3D9_RS_SRCBLEND;
        NativeValue = nullD3DBlendFactor(Value);
        break;
    case 0x3Fu: // D3DRS_DESTBLEND
        NativeState = COD3_D3D9_RS_DESTBLEND;
        NativeValue = nullD3DBlendFactor(Value);
        break;
    case 0x4Au: // D3DRS_BLENDOP
        NativeState = COD3_D3D9_RS_BLENDOP;
        NativeValue = nullD3DBlendOp(Value);
        break;
    case 0x4Bu: // D3DRS_BLENDCOLOR
        NativeState = D3DRS_BLENDFACTOR;
        break;
    case 0x8Au: // D3DRS_FOGCOLOR
        NativeState = COD3_D3D9_RS_FOGCOLOR;
        break;
    case 0x40u: // D3DRS_ZWRITEENABLE
        NativeState = COD3_D3D9_RS_ZWRITEENABLE;
        break;
    case 0x43u: // D3DRS_COLORWRITEENABLE
        NativeState = COD3_D3D9_RS_COLORWRITEENABLE;
        NativeValue = nullD3DColorWriteMask(Value);
        break;
    case 0x67u: // D3DRS_SPECULARENABLE
        NativeState = COD3_D3D9_RS_SPECULARENABLE;
        break;
    case 0x93u: // D3DRS_CULLMODE
        NativeState = COD3_D3D9_RS_CULLMODE;
        NativeValue = nullD3DCullMode(Value);
        break;
    case 0x8Fu: // D3DRS_ZENABLE
        NativeState = COD3_D3D9_RS_ZENABLE;
        break;
    case 0x95u: // D3DRS_ZBIAS
        NativeState = D3DRS_DEPTHBIAS;
        break;
    case 0x90u: // D3DRS_STENCILENABLE
        NativeState = COD3_D3D9_RS_STENCILENABLE;
        break;
    case 0x46u: // D3DRS_STENCILFUNC
        NativeState = COD3_D3D9_RS_STENCILFUNC;
        NativeValue = nullD3DCompareFunc(Value);
        break;
    case 0x48u: // D3DRS_STENCILMASK
        NativeState = COD3_D3D9_RS_STENCILMASK;
        break;
    case 0x45u: // D3DRS_STENCILPASS
        NativeState = COD3_D3D9_RS_STENCILPASS;
        NativeValue = nullD3DStencilOp(Value);
        break;
    case 0x98u: // D3DRS_MULTISAMPLEANTIALIAS
        NativeState = COD3_D3D9_RS_MULTISAMPLEANTIALIAS;
        break;
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
    if (VertexFormat != NULL) {
        memcpy(&gD3D9SelectedVertexFormat, VertexFormat, sizeof(gD3D9SelectedVertexFormat));
        gD3D9SelectedVertexFormatValid = true;
        nullD3DBuildVertexDeclaration(&gD3D9SelectedVertexFormat);
    }
    for (unsigned int i = 0; i < 4; ++i) {
        if (i >= StreamCount || StreamInputs == NULL) {
            gD3D9VertexBuffers[i] = NULL;
            gD3D9VertexInfos[i] = NULL;
            gD3D9VertexData[i] = NULL;
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
        gD3D9VertexData[i] = StreamInputs[i].VertexBuffer == NULL ||
                                     StreamInputs[i].VertexBuffer->Data == 0
                                 ? NULL
                                 : (const unsigned char*)(uintptr_t)
                                       StreamInputs[i].VertexBuffer->Data;
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
    // D3D9 retains a reference to the bound native texture independently of
    // the Xbox wrapper.  Drop every matching stage before releasing the
    // wrapper/native resource so a later push-buffer draw cannot reuse it.
    for (unsigned int Stage = 0; Stage < 4; ++Stage) {
        if (gNullBoundTextures[Stage] == (D3DBaseTexture*)Resource) {
            gNullBoundTextures[Stage] = NULL;
            gNullTextureWidths[Stage] = 0;
            gNullTextureHeights[Stage] = 0;
            gNullPalettes[Stage] = NULL;
            if (gD3D9Device != NULL)
                gD3D9Device->SetTexture(Stage, NULL);
        }
    }
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
        if (Info->Kind == NULL_D3D_BUFFER) {
            for (unsigned int i = 0; i < sizeof(gNullBuffers) / sizeof(gNullBuffers[0]); ++i) {
                if (gNullBuffers[i] == (nullD3DBuffer*)Resource) {
                    gNullBuffers[i] = NULL;
                    break;
                }
            }
        }
        free(Info->Bits);
        free(Resource);
    }
    return 0;
}
int __cdecl __fpclass(double a0) { (void)a0; return 0; }
}
