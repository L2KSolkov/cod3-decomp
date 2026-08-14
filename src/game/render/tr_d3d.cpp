// ============================================================================
// tr_d3d.cpp - render.o Xbox NGL mid-scene callback (tr_main.cpp)
// Types and bodies verified against IDA (codmp_xboxr.xbe).
// ============================================================================

#include "ngl/ngl_dx_quad.h"
#include "ngl/ngl_dx_gpu.h"
#include "d3d8.h"

// Xbox D3D render-state caches (xbox shim data)
extern unsigned int dword_40354;   // PS_MAX method
extern unsigned int dword_40358;   // COLORWRITEENABLE method
extern unsigned int dword_4035C;   // ZWRITEENABLE method
extern unsigned int dword_BC2D10;  // ZWRITEENABLE cache
extern unsigned int dword_BC2D1C;  // COLORWRITEENABLE cache
extern unsigned int dword_BC2CF4;  // PS_MAX cache ("Value")

extern nglScene* nglBuildScene;    // ?nglBuildScene@@3PAUnglScene@@A
void nglRenderQuad(nglQuad* Quad); // ?nglRenderQuad@@YAXPAUnglQuad@@@Z (ngl_dx_quad.o)
void R_SetWindowQuadRect(nglQuad& q);  // tr_fx.cpp

// ============================================================================
// XboxNGLMidSceneCallBack - ea: 0x006C2200
// ============================================================================
void XboxNGLMidSceneCallBack(void* Data)
{
    (void)Data;
    unsigned char v0 = (unsigned char)dword_BC2D10;
    unsigned int v1 = dword_BC2CF4;

    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_PS_MAX, 0x207u) == 0)
    {
        D3DDevice_SetRenderState_Simple(dword_40354, 0x207u);
        dword_BC2CF4 = 519;
    }
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_ZWRITEENABLE, 0) == 0)
    {
        D3DDevice_SetRenderState_Simple(dword_4035C, 0);
        dword_BC2D10 = 0;
    }
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_COLORWRITEENABLE, 0x1000000u) == 0)
    {
        D3DDevice_SetRenderState_Simple(dword_40358, 0x1000000u);
        dword_BC2D1C = 0x1000000u;
    }

    nglQuad q;
    nglInitQuad(&q);
    R_SetWindowQuadRect(q);
    nglSetQuadZ(&q, -1.0f);
    nglSetQuadColor(&q, 0);
    nglSetQuadBlend(&q, 0);
    nglRenderQuad(&q);

    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_ZWRITEENABLE, v0) == 0)
    {
        D3DDevice_SetRenderState_Simple(dword_4035C, v0);
        dword_BC2D10 = v0;
    }
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_PS_MAX, v1) == 0)
    {
        D3DDevice_SetRenderState_Simple(dword_40354, v1);
        dword_BC2CF4 = v1;
    }

    nglBuildScene->FBWriteMask = 0x10101u;
    unsigned int FBWriteMask = nglBuildScene->FBWriteMask;
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_COLORWRITEENABLE, FBWriteMask) == 0)
    {
        D3DDevice_SetRenderState_Simple(dword_40358, FBWriteMask);
        dword_BC2D1C = FBWriteMask;
    }
}
