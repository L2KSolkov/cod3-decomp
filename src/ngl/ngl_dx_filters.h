// ============================================================================
// ngl_dx_filters.h - NGL full-screen filter passes (glow/blur/DOF/edge).
// Source: src/dx/ngl_dx_filters.cpp (ngl_xboxr)
// ============================================================================
#ifndef COD3_NGL_NGL_DX_FILTERS_H
#define COD3_NGL_NGL_DX_FILTERS_H

#include "ngl/nglTexture.h"

namespace nglDxFilters {

// Z-buffer lookup-table mode (values verified against IDA enum).
enum t_ZBufferLUT {
    ZLUT_ZFOG = 0,
    ZLUT_DOF = 1,
    ZLUT_MAX = 2,
};

// Multi-sample filter offset (12 bytes: scale + X/Y offsets).
struct FilterSample {
    float fScale;    // +0x00
    float fOffsetX;  // +0x04
    float fOffsetY;  // +0x08
};
static_assert(sizeof(FilterSample) == 0xC, "FilterSample size mismatch");

// ngl_dx_filters.o (data, defined in ngl_dx_filters.cpp)
extern t_ZBufferLUT CurZBufferLUT;
extern nglTexture ZBufferTex;
extern bool FiltersTexAllocated;
extern nglTexture* ZBufferLUT;
extern nglTexture* WorkTex[2];
extern nglTexture*& SrcTex;
extern FilterSample BoxFilter[4];
extern FilterSample YFilter[4];
extern FilterSample XFilter[4];
extern int NPasses;
extern float Inc;
extern float PrevVals[3];
extern float PrevVals_0[6];
extern int NPasses_0;
extern float Inc_0;
extern D3DBaseTexture D3DZTex;

// ngl_dx_filters.o (functions, defined in ngl_dx_filters.cpp)
void CopyImage(nglTexture* SrcTex, nglTexture* DstTex);
void FilterCopy(nglTexture* SrcTex, nglTexture* DstTex, unsigned int dwNumSamples,
                const FilterSample* rSample, unsigned int dwSuperSampleX,
                unsigned int dwSuperSampleY);
void RenderGlow(float GlowIntensity);
void FillDepthPalette();
void FillFogPalette();
void RenderBlur(nglTexture* SrcTex, nglTexture* DstTex);
void AllocFiltersTex();
void FreeFiltersTex();
void nglUpdateFilterTextures();
void nglDestroyFilterTextures();

} // namespace nglDxFilters

// Free functions (ngl_dx_filters.o).
void nglSetFiltersTexSizes(unsigned int FilterTexWidth, unsigned int FilterTexHeight);
void nglCreateFilterTextures(unsigned int FilterTexWidth, unsigned int FilterTexHeight);
void nglEdgeDetectionCallBack(void* Data);
void nglFogCallBack(void* Data);
void nglGlowCallBack(void* Data);
void nglPostProcessFiltersTex();
void nglBlurCallBack(void* Data);
void nglDepthOfFieldCallBack(void* Data);

#endif // COD3_NGL_NGL_DX_FILTERS_H
