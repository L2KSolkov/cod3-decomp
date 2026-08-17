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

// ============================================================================
// D3DDevice::SetRenderState - ea: 0x006E5650
// ============================================================================
void D3DDevice_SetRenderState_Deferred(unsigned int State, unsigned int Value);
unsigned int DSI[92] = {
    262752u, 262756u, 262760u, 262764u, 262768u, 262772u, 262776u, 262780u,
    262792u, 262796u, 264800u, 264804u, 264808u, 264812u, 264816u, 264820u,
    264824u, 264828u, 264832u, 264836u, 264840u, 264844u, 264848u, 264852u,
    264856u, 264860u, 264864u, 264868u, 264872u, 264876u, 264880u, 264884u,
    264888u, 264892u, 264896u, 264900u, 264904u, 264908u, 264912u, 264916u,
    264920u, 264924u, 268280u, 269856u, 269860u, 269888u, 269892u, 269896u,
    269900u, 269904u, 269908u, 269912u, 269916u, 269920u, 269712u, 269940u,
    269944u, 262996u, 262972u, 262916u, 262912u, 262976u, 262980u, 262984u,
    263004u, 262928u, 263036u, 263000u, 263028u, 263032u, 263012u, 263016u,
    263020u, 263008u, 262992u, 262988u, 264696u, 263044u, 263048u, 262960u,
    262964u, 262968u, 269688u, 267388u, 269712u, 269712u, 269712u, 269712u,
    269712u, 269712u, 269712u, 269712u
};  // DSI[92], copied from codmp_xboxr.xbe.c
enum { D3DRS_DEFERRED_MAX_LOCAL = 0x50 };  // D3DRS_DEFERRED_MAX (xbox XDK)

long __stdcall D3DDevice::SetRenderState(_D3DRENDERSTATETYPE State,
                                         unsigned long Value)
{
    if (D3DDevice_SetRenderState_ParameterCheck((unsigned int)State, Value) == 0)
    {
        if ((unsigned int)State < D3DRS_SIMPLE_MAX)
        {
            D3DDevice_SetRenderState_Simple(DSI[(unsigned int)State], Value);
            D3D__RenderState[(unsigned int)State] = Value;
            return 0;
        }
        if ((unsigned int)State < D3DRS_DEFERRED_MAX_LOCAL)
        {
            unsigned int flags = D3D__DirtyFlags;
            D3D__RenderState[(unsigned int)State] = Value;
            D3D__DirtyFlags =
                flags | (1u << ((unsigned int)State - D3DRS_SIMPLE_MAX));
            return 0;
        }
        D3DDevice_SetRenderState_Deferred((unsigned int)State, Value);
    }
    return 0;
}
