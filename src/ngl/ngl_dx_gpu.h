// ============================================================================
// ngl_dx_gpu.h — GPU texture/format wrapper layer.
// Source: c:\cod\code\tl\ngl\include\dx/ngl_dx_gpu.h
//
// All functions here are inline COMDATs emitted into ngl_xboxr:ngl_gpu_texture.o
// (and other ngl objects). The map-mangled names are listed per function.
// ============================================================================
#ifndef COD3_NGL_NGL_DX_GPU_H
#define COD3_NGL_NGL_DX_GPU_H

#include "d3d8.h"

#include <cstddef>
#include <intrin.h>

// tl_system.o (tl_xboxr, ported)
extern void* tlMemAlloc(unsigned size, unsigned align, unsigned flags);
extern bool  _tlAssert(const char* file, int line, const char* expr, const char* desc);
extern void  tlFatal(const char* fmt, ...);

// ============================================================================
// gpuTextureFormat — Xbox GPU texture format enum (values from IDA)
// ============================================================================
enum gpuTextureFormat {
    GPU_TEXTURE_L8 = 0,
    GPU_TEXTURE_AL8 = 1,
    GPU_TEXTURE_A1R5G5B5 = 2,
    GPU_TEXTURE_X1R5G5B5 = 3,
    GPU_TEXTURE_A4R4G4B4 = 4,
    GPU_TEXTURE_R5G6B5 = 5,
    GPU_TEXTURE_A8R8G8B8 = 6,
    GPU_TEXTURE_X8R8G8B8 = 7,
    GPU_TEXTURE_P8 = 11,
    GPU_TEXTURE_DXT1 = 12,
    GPU_TEXTURE_DXT2 = 14,
    GPU_TEXTURE_DXT3 = 14,
    GPU_TEXTURE_DXT4 = 15,
    GPU_TEXTURE_DXT5 = 15,
    GPU_TEXTURE_LIN_A1R5G5B5 = 16,
    GPU_TEXTURE_LIN_R5G6B5 = 17,
    GPU_TEXTURE_LIN_A8R8G8B8 = 18,
    GPU_TEXTURE_LIN_Q8W8V8U8 = 18,
    GPU_TEXTURE_LIN_L8 = 19,
    GPU_TEXTURE_LIN_R8B8 = 22,
    GPU_TEXTURE_LIN_G8B8 = 23,
    GPU_TEXTURE_LIN_V8U8 = 23,
    GPU_TEXTURE_LIN_AL8 = 27,
    GPU_TEXTURE_LIN_X1R5G5B5 = 28,
    GPU_TEXTURE_LIN_A4R4G4B4 = 29,
    GPU_TEXTURE_LIN_X8R8G8B8 = 30,
    GPU_TEXTURE_LIN_X8L8V8U8 = 30,
    GPU_TEXTURE_LIN_A8 = 31,
    GPU_TEXTURE_LIN_A8L8 = 32,
    GPU_TEXTURE_YUY2 = 36,
    GPU_TEXTURE_UYVY = 37,
    GPU_TEXTURE_R6G5B5 = 39,
    GPU_TEXTURE_L6V5U5 = 39,
    GPU_TEXTURE_G8B8 = 40,
    GPU_TEXTURE_V8U8 = 40,
    GPU_TEXTURE_R8B8 = 41,
    GPU_TEXTURE_D24S8 = 42,
    GPU_TEXTURE_F24S8 = 43,
    GPU_TEXTURE_D16 = 44,
    GPU_TEXTURE_D16_LOCKABLE = 44,
    GPU_TEXTURE_F16 = 45,
    GPU_TEXTURE_LIN_D24S8 = 46,
    GPU_TEXTURE_LIN_F24S8 = 47,
    GPU_TEXTURE_LIN_D16 = 48,
    GPU_TEXTURE_LIN_F16 = 49,
    GPU_TEXTURE_L16 = 50,
    GPU_TEXTURE_V16U16 = 51,
    GPU_TEXTURE_LIN_L16 = 53,
    GPU_TEXTURE_LIN_V16U16 = 54,
    GPU_TEXTURE_LIN_R6G5B5 = 55,
    GPU_TEXTURE_LIN_L6V5U5 = 55,
    GPU_TEXTURE_R5G5B5A1 = 56,
    GPU_TEXTURE_R4G4B4A4 = 57,
    GPU_TEXTURE_A8B8G8R8 = 58,
    GPU_TEXTURE_Q8W8V8U8 = 58,
    GPU_TEXTURE_B8G8R8A8 = 59,
    GPU_TEXTURE_R8G8B8A8 = 60,
    GPU_TEXTURE_LIN_R5G5B5A1 = 61,
    GPU_TEXTURE_LIN_R4G4B4A4 = 62,
    GPU_TEXTURE_LIN_A8B8G8R8 = 63,
    GPU_TEXTURE_LIN_B8G8R8A8 = 64,
    GPU_TEXTURE_LIN_R8G8B8A8 = 65,
};

// ============================================================================
// gpuMultiSampleType — GPU multisample mode enum (values from IDA)
// ============================================================================
enum gpuMultiSampleType {
    NGL_GPU_MULTISAMPLE_NONE = 17,
    NGL_GPU_MULTISAMPLE_2X = 4385,
    NGL_GPU_MULTISAMPLE_4X = 4642,
};

// ============================================================================
// D3DResource type selectors (D3DRTYPE_*)
// ============================================================================
enum _D3DRESOURCETYPE {
    D3DRTYPE_TEXTURE = 3,
    D3DRTYPE_VOLUMETEXTURE = 4,
    D3DRTYPE_CUBETEXTURE = 5,
};

// ============================================================================
// D3DDevice — static D3D8 device entry-point wrappers.
// All inline COMDATs (ea: 0x84A790-0x84A880 in ngl_gpu_texture.o).
// ============================================================================
class D3DDevice {
public:
    static unsigned int __stdcall CreateTexture(unsigned int Width, unsigned int Height,
                                                unsigned int Levels, unsigned int Usage,
                                                _D3DFORMAT Format, unsigned int UnusedPool,
                                                D3DTexture** ppTexture) {
        D3DTexture* Texture2 = (D3DTexture*)D3DDevice_CreateTexture2(Width, Height, 1, Levels, Usage, Format,
                                                        D3DRTYPE_TEXTURE);
        *ppTexture = Texture2;
        return Texture2 != NULL ? 0 : 0x8007000E;
    }

    static unsigned int __stdcall CreateVolumeTexture(unsigned int Width, unsigned int Height,
                                                      unsigned int Depth, unsigned int Levels,
                                                      unsigned int Usage, _D3DFORMAT Format,
                                                      unsigned int UnusedPool,
                                                      D3DVolumeTexture** ppVolumeTexture) {
        D3DVolumeTexture* Texture2 = (D3DVolumeTexture*)D3DDevice_CreateTexture2(Width, Height, Depth, Levels, Usage,
                                                              Format, D3DRTYPE_VOLUMETEXTURE);
        *ppVolumeTexture = Texture2;
        return Texture2 != NULL ? 0 : 0x8007000E;
    }

    static unsigned int __stdcall CreateCubeTexture(unsigned int EdgeLength, unsigned int Levels,
                                                    unsigned int Usage, _D3DFORMAT Format,
                                                    unsigned int UnusedPool,
                                                    D3DCubeTexture** ppCubeTexture) {
        D3DCubeTexture* Texture2 = (D3DCubeTexture*)D3DDevice_CreateTexture2(EdgeLength, EdgeLength, 1, Levels,
                                                            Usage, Format, D3DRTYPE_CUBETEXTURE);
        *ppCubeTexture = Texture2;
        return Texture2 != NULL ? 0 : 0x8007000E;
    }

    static unsigned int __stdcall CreateRenderTarget(unsigned int Width, unsigned int Height,
                                                     _D3DFORMAT Format, unsigned int UnusedMultiSample,
                                                     int UnusedLockable, D3DSurface** ppSurface) {
        D3DSurface* Surface2 = (D3DSurface*)D3DDevice_CreateSurface2(Width, Height, 1, Format);
        *ppSurface = Surface2;
        return Surface2 != NULL ? 0 : 0x8007000E;
    }

    static unsigned int __stdcall CreateDepthStencilSurface(unsigned int Width, unsigned int Height,
                                                            _D3DFORMAT Format,
                                                            unsigned int UnusedMultiSample,
                                                            D3DSurface** ppSurface) {
        D3DSurface* Surface2 = (D3DSurface*)D3DDevice_CreateSurface2(Width, Height, 2, Format);
        *ppSurface = Surface2;
        return Surface2 != NULL ? 0 : 0x8007000E;
    }
};

// ============================================================================
// gpuD3DDevice — D3D8 device wrapper (DX9-style member interface).
// All inline COMDATs (ea: 0x84A8E0-0x84A9E0 in ngl_gpu_texture.o).
// ============================================================================
class gpuD3DDevice {
public:
    unsigned int CreateTexture(unsigned int Width, unsigned int Height, unsigned int Levels,
                               unsigned int Usage, _D3DFORMAT Format, unsigned int Pool,
                               D3DTexture** ppTexture, void** pSharedHandle) {
        D3DTexture* Texture2 = (D3DTexture*)D3DDevice_CreateTexture2(Width, Height, 1, Levels, Usage, Format,
                                                        D3DRTYPE_TEXTURE);
        *ppTexture = Texture2;
        return Texture2 != NULL ? 0 : 0x8007000E;
    }

    unsigned int CreateCubeTexture(unsigned int EdgeLength, unsigned int Levels, unsigned int Usage,
                                   _D3DFORMAT Format, unsigned int Pool,
                                   D3DCubeTexture** ppCubeTexture, void** pSharedHandle) {
        D3DCubeTexture* Texture2 = (D3DCubeTexture*)D3DDevice_CreateTexture2(EdgeLength, EdgeLength, 1, Levels,
                                                            Usage, Format, D3DRTYPE_CUBETEXTURE);
        *ppCubeTexture = Texture2;
        return Texture2 != NULL ? 0 : 0x8007000E;
    }

    unsigned int CreateVolumeTexture(unsigned int Width, unsigned int Height, unsigned int Depth,
                                     unsigned int Levels, unsigned int Usage, _D3DFORMAT Format,
                                     unsigned int Pool, D3DVolumeTexture** ppVolumeTexture,
                                     void** pSharedHandle) {
        D3DVolumeTexture* Texture2 = (D3DVolumeTexture*)D3DDevice_CreateTexture2(Width, Height, Depth, Levels, Usage,
                                                              Format, D3DRTYPE_VOLUMETEXTURE);
        *ppVolumeTexture = Texture2;
        return Texture2 != NULL ? 0 : 0x8007000E;
    }

    unsigned int CreateRenderTarget(unsigned int Width, unsigned int Height, _D3DFORMAT Format,
                                    unsigned int MultiSample, unsigned int MultisampleQuality,
                                    int Lockable, D3DSurface** ppSurface, void** pSharedHandle) {
        D3DSurface* Surface2 = (D3DSurface*)D3DDevice_CreateSurface2(Width, Height, 1, Format);
        *ppSurface = Surface2;
        return Surface2 != NULL ? 0 : 0x8007000E;
    }

    unsigned int CreateDepthStencilSurface(unsigned int Width, unsigned int Height, _D3DFORMAT Format,
                                           unsigned int MultiSample, unsigned int MultisampleQuality,
                                           int Discard, D3DSurface** ppSurface,
                                           void** pSharedHandle) {
        D3DSurface* Surface2 = (D3DSurface*)D3DDevice_CreateSurface2(Width, Height, 2, Format);
        *ppSurface = Surface2;
        return Surface2 != NULL ? 0 : 0x8007000E;
    }
};

// ============================================================================
// gpuCreate* — free-function texture creators (inline COMDATs, ngl_gpu_texture.o)
// ============================================================================
inline D3DTexture* gpuCreateTexture(unsigned int width, unsigned int height, unsigned int levels,
                                    gpuTextureFormat format, bool rendertarget,
                                    gpuMultiSampleType multisample) {
    D3DTexture* Texture2 = (D3DTexture*)D3DDevice_CreateTexture2(width, height, 1, levels, 0x200, format,
                                                    D3DRTYPE_TEXTURE);
    if (Texture2 == NULL)
        tlFatal("gpu: Out of memory for texture.");
    return Texture2;
}

inline D3DTexture* gpuCreateCubeTexture(unsigned int size, unsigned int levels,
                                        gpuTextureFormat format, bool rendertarget,
                                        gpuMultiSampleType multisample) {
    D3DTexture* Texture2 = (D3DTexture*)D3DDevice_CreateTexture2(size, size, 1, levels, 0x200, format,
                                                    D3DRTYPE_CUBETEXTURE);
    return Texture2;
}

inline D3DTexture* gpuCreateVolumeTexture(unsigned int width, unsigned int height,
                                          unsigned int depth, unsigned int levels,
                                          gpuTextureFormat format, bool rendertarget,
                                          gpuMultiSampleType multisample) {
    D3DTexture* Texture2 = (D3DTexture*)D3DDevice_CreateTexture2(width, height, depth, levels, 0x200, format,
                                                    D3DRTYPE_VOLUMETEXTURE);
    return Texture2;
}

inline D3DSurface* gpuCreateRenderSurface(unsigned int w, unsigned int h, gpuTextureFormat format,
                                          gpuMultiSampleType multisample) {
    return D3DDevice_CreateSurface2(w, h, 1, format);
}

inline D3DSurface* gpuCreateDepthStencilSurface(unsigned int w, unsigned int h,
                                                gpuTextureFormat format,
                                                gpuMultiSampleType multisample) {
    return D3DDevice_CreateSurface2(w, h, 2, format);
}

// ============================================================================
// XGIsTiledFormat — is the format a swizzled/tiled format? (inline COMDAT)
// ea: 0x84A8B0
// ============================================================================
inline int XGIsTiledFormat(_D3DFORMAT Format) {
    return XGIsSwizzledFormat(Format);
}

#endif // COD3_NGL_NGL_DX_GPU_H
