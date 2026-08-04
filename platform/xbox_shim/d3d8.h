// ============================================================================
// d3d8.h — Xbox D3D8 API shim declarations (d3d8d library, NEVER reconstruct).
// Source: Xbox XDK Dec 2003 d3d8.h
//
// Opaque types + the handful of D3D8 entry points the ported game/engine
// objects call. Implementations land in the future D3D11 backend
// (Phase 6); until then the linker satisfies them via /FORCE:UNRESOLVED.
// ============================================================================
#pragma once

#ifdef __cplusplus

// ---- Opaque D3D8 objects ------------------------------------------------
// D3DBaseTexture is the common base; the engine stores these in nglTexture.
struct D3DBaseTexture {};
struct D3DTexture : D3DBaseTexture {};
struct D3DCubeTexture : D3DBaseTexture {};
struct D3DSurface {};
struct D3DResource {};

// ---- Cube-map face selector (D3DCUBEMAP_FACES) ---------------------------
enum _D3DCUBEMAP_FACES {
    D3DCUBEMAP_FACE_POSITIVE_X = 0,
    D3DCUBEMAP_FACE_NEGATIVE_X = 1,
    D3DCUBEMAP_FACE_POSITIVE_Y = 2,
    D3DCUBEMAP_FACE_NEGATIVE_Y = 3,
    D3DCUBEMAP_FACE_POSITIVE_Z = 4,
    D3DCUBEMAP_FACE_NEGATIVE_Z = 5,
};

// ---- D3D8 entry points (stdcall, @N-decorated like the XDK exports) ------
extern "C" {
void         __stdcall D3DDevice_SetRenderTarget(D3DSurface* pRenderTarget, D3DSurface* pZBuffer);
D3DSurface*  __stdcall D3DCubeTexture_GetCubeMapSurface2(D3DBaseTexture* pTexture,
                                                        _D3DCUBEMAP_FACES FaceType,
                                                        unsigned int Level);
D3DSurface*  __stdcall D3DTexture_GetSurfaceLevel2(D3DBaseTexture* pTexture, unsigned int Level);
unsigned int __stdcall D3DResource_Release(D3DResource* pResource);
}

#endif // __cplusplus
