// ============================================================================
// tr_gl.cpp - render.o GL config / overexposure / view-model helpers
// Types and bodies verified against IDA (codmp_xboxr.xbe).
// ============================================================================

#include "game/logic/g_local.h"
#include "ngl/ngl_dx_gpu.h"

#include <intrin.h>
#include <math.h>
#include <string.h>

// glconfig_t (IDA type, 40 members, size 0xA0; ?glConfig@@3Uglconfig_t@@A @ 0xF74300)
struct glconfig_t {
    const char* renderer_string;      // +0x00
    const char* vendor_string;        // +0x04
    const char* version_string;       // +0x08
    const char* extensions_string;    // +0x0C
    const char* wgl_extensions_string;// +0x10
    int maxTextureSize;               // +0x14
    int maxActiveTextures;            // +0x18
    int maxHardwareLights;            // +0x1C
    int colorBits;                    // +0x20
    int depthBits;                    // +0x24
    int stencilBits;                  // +0x28
    int deviceSupportsGamma;          // +0x2C
    int anisotropicAvailable;         // +0x30
    float maxAnisotropy;              // +0x34
    int ARB_texture_env_add;          // +0x38
    int ARB_texture_cube_map;         // +0x3C
    int ARB_texture_env_combine;      // +0x40
    int ARB_texture_env_dot3;         // +0x44
    int ARB_vertex_buffer_object;     // +0x48
    int ARB_vertex_program;           // +0x4C
    int EXT_rescale_normal;           // +0x50
    int NVFogAvailable;               // +0x54
    int NVFogMode;                    // +0x58
    int NV_vertex_array_range;        // +0x5C
    int NV_fence;                     // +0x60
    int NV_register_combiners;        // +0x64
    int NV_texture_shader;            // +0x68
    int ATIMaxTruformTess;            // +0x6C
    int ATINormalMode;                // +0x70
    int ATIPointMode;                 // +0x74
    int ATI_vertex_array_object;      // +0x78
    int ATI_element_array;            // +0x7C
    int ATI_fragment_shader;          // +0x80
    int vidWidth;                     // +0x84
    int vidHeight;                    // +0x88
    float windowAspect;               // +0x8C
    int displayFrequency;             // +0x90
    int isFullscreen;                 // +0x94
    int stereoEnabled;                // +0x98
    int textureFilterAnisotropicAvailable;  // +0x9C
};
static_assert(sizeof(glconfig_t) == 0xA0, "glconfig_t size mismatch");
glconfig_t glConfig;  // ?glConfig@@3Uglconfig_t@@A @ 0xF74300

// BP_OVEREXPOSURE (IDA type, size 0x2C; ?bpOe@@3UBP_OVEREXPOSURE@@A @ 0xDFB0C8)
struct BP_OVEREXPOSURE {
    float oeBloomLevel;     // +0x00
    float oeBloomStart;     // +0x04
    float oeBloomEnd;       // +0x08
    float oeBloomDecay;     // +0x0C
    float oeGammaLevel;     // +0x10
    float oeGammaStart;     // +0x14
    float oeGammaDecay;     // +0x18
    float oeBloomRealStart; // +0x1C
    float oeBloomInTime;    // +0x20
    float oeGammaInTime;    // +0x24
    float oeLeadTime;       // +0x28
};
static_assert(sizeof(BP_OVEREXPOSURE) == 0x2C, "BP_OVEREXPOSURE size mismatch");
BP_OVEREXPOSURE bpOe;  // ?bpOe@@3UBP_OVEREXPOSURE@@A @ 0xDFB0C8

// viewModelInfo_t (IDA type, size 0x70; tr.viewModelInfo @ tr+0x2A0)
struct viewModelInfo_t {
    int mDoingRender;       // +0x00
    int mInWorldScene;      // +0x04
    int mScaleWeaponTrans;  // +0x08
    int mDrawBeforeWorld;   // +0x0C
    float mArmsScale;       // +0x10
    float mWeaponScale;     // +0x14
    math::Mat43 mArmsOffsetMat;    // +0x20 (aligned)
    math::Position3 mWeaponOrigin; // +0x60
};
static_assert(sizeof(viewModelInfo_t) == 0x70, "viewModelInfo_t size mismatch");

// trGlobals_t view (IDA type: world +0x290, viewModelInfo +0x2A0,
// viewModelInfoIndex +0x310, debug +0x314)
struct trGlobals_t {
    int registered;          // +0x00
    int worldMapLoaded;      // +0x04
    int frameCount;          // +0x08
    int viewCount;           // +0x0C
    uint8_t _pad0[0x290 - 0x10];  // viewParms / or / refdef
    void* world;             // +0x290
    uint8_t _pad1[0x2A0 - 0x294];
    viewModelInfo_t viewModelInfo[1];  // +0x2A0
    int viewModelInfoIndex;  // +0x310
};
extern trGlobals_t tr;  // ?tr@@3UtrGlobals_t@@A @ 0xF74DD0

// scene frame counters (tr_scene.cpp)
extern int r_firstSceneDlight;  // ?r_firstSceneDlight@@3HA @ 0xF74274
extern int r_numdlights;        // ?r_numdlights@@3HA @ 0xF741E4
extern int r_firstSceneCorona;  // ?r_firstSceneCorona@@3HA @ 0xF742D0
extern int r_numcoronas;        // ?r_numcoronas@@3HA @ 0xF741BC
extern int r_firstScenePoly;    // ?r_firstScenePoly@@3HA @ 0xF742B0
extern int r_numpolys;          // ?r_numpolys@@3HA @ 0xF742FC
extern void R_Init();           // render.o 0x6D27F0 (not yet ported)

// ea: 0x006D2970
void RE_BeginRegistration(glconfig_t* glconfigOut)
{
    R_Init();
    memcpy(glconfigOut, &glConfig, sizeof(glconfig_t));
    r_firstSceneDlight = r_numdlights;
    r_firstSceneCorona = r_numcoronas;
    r_firstScenePoly = r_numpolys;
    tr.registered = 1;
}

// nglListAddMesh_GetScaledMatrix static data (render.o @ 0xF782E0 / 0xF7832C)
static math::Mat43 ScaledLocalToWorld;
static unsigned int s_scaledInit;  // $S28_3

// ea: 0x006C0540
void R_SetNVFogMode()
{
}

// ea: 0x006C0550
void GL_SetDefaultState()
{
}

// ea: 0x006C0560
void R_VboRefresh_f()
{
}

// ea: 0x006C0570
void HackUpGLConfig()
{
    glConfig.colorBits = 24;
    glConfig.depthBits = 24;
    glConfig.maxAnisotropy = 1.0f;
    glConfig.renderer_string = "porting to NGL";
    glConfig.vendor_string = "Michael Uhlik";
    glConfig.version_string = "whatever";
    glConfig.extensions_string = "drink the blood of the bourgeousie";
    glConfig.wgl_extensions_string = "blah";
    glConfig.maxTextureSize = 512;
    glConfig.maxActiveTextures = 2;
    glConfig.maxHardwareLights = 8;
    glConfig.stencilBits = 8;
    glConfig.deviceSupportsGamma = 0;
    glConfig.anisotropicAvailable = 0;
    glConfig.ARB_texture_env_add = 0;
    glConfig.ARB_texture_cube_map = 0;
    glConfig.ARB_texture_env_combine = 0;
    glConfig.ARB_texture_env_dot3 = 0;
    glConfig.ARB_vertex_buffer_object = 0;
    glConfig.ARB_vertex_program = 0;
    glConfig.EXT_rescale_normal = 0;
    glConfig.NVFogAvailable = 0;
    glConfig.NVFogMode = 0;
    glConfig.NV_vertex_array_range = 0;
    glConfig.NV_fence = 0;
    glConfig.NV_register_combiners = 0;
    glConfig.NV_texture_shader = 0;
    glConfig.ATIMaxTruformTess = 0;
    glConfig.ATINormalMode = 0;
    glConfig.ATIPointMode = 0;
    glConfig.ATI_vertex_array_object = 0;
    glConfig.ATI_element_array = 0;
    glConfig.ATI_fragment_shader = 0;
    glConfig.vidWidth = 640;
    glConfig.vidHeight = 480;
    glConfig.windowAspect = 1.3333334f;
    glConfig.displayFrequency = 60;
    glConfig.isFullscreen = 0;
    glConfig.stereoEnabled = 0;
    glConfig.textureFilterAnisotropicAvailable = 0;
}

// ea: 0x006C06A0
void RE_EndRegistration()
{
}

// ea: 0x006C06B0
void nglStartOverExposure(float oeBloomStart, float oeBloomEnd,
                          float oeBloomDecay, float oeGammaStart,
                          float oeGammaDecay)
{
    bpOe.oeBloomStart = oeBloomStart;
    bpOe.oeBloomLevel = oeBloomStart;
    bpOe.oeGammaStart = oeGammaStart;
    bpOe.oeGammaLevel = oeGammaStart;
    bpOe.oeBloomDecay = (oeBloomStart - oeBloomEnd) / oeBloomDecay;
    bpOe.oeGammaDecay = (oeGammaStart - 1.0f) / oeGammaDecay;
    bpOe.oeLeadTime = 0.0f;
}

// ea: 0x006C2380
float R_GetViewModelArmsScale(int client_index)
{
    return tr.viewModelInfo[client_index].mArmsScale;
}

// ea: 0x006C23A0
float R_GetViewModelWeaponScale(int client_index)
{
    return tr.viewModelInfo[client_index].mWeaponScale;
}

// ea: 0x006C6AA0
void R_SetViewModelScale(int client_index, float armsScale, float weaponScale,
                         int inWorldScene, int scaleWeaponTrans,
                         math::Mat43* armsOffsetMat)
{
    tr.viewModelInfo[client_index].mScaleWeaponTrans = scaleWeaponTrans;
    tr.viewModelInfo[client_index].mArmsScale = armsScale;
    tr.viewModelInfo[client_index].mWeaponScale = weaponScale;
    tr.viewModelInfo[client_index].mInWorldScene = inWorldScene;
    if (armsOffsetMat != nullptr)
    {
        tr.viewModelInfo[client_index].mArmsOffsetMat.x = armsOffsetMat->x;
        tr.viewModelInfo[client_index].mArmsOffsetMat.y = armsOffsetMat->y;
        tr.viewModelInfo[client_index].mArmsOffsetMat.z = armsOffsetMat->z;
        tr.viewModelInfo[client_index].mArmsOffsetMat.w = armsOffsetMat->w;
    }
    else
    {
        tr.viewModelInfo[client_index].mArmsOffsetMat.x.v =
            _mm_setr_ps(1.0f, 0.0f, 0.0f, 0.0f);
        tr.viewModelInfo[client_index].mArmsOffsetMat.y.v =
            _mm_setr_ps(0.0f, 1.0f, 0.0f, 0.0f);
        tr.viewModelInfo[client_index].mArmsOffsetMat.z.v =
            _mm_setr_ps(0.0f, 0.0f, 1.0f, 0.0f);
        tr.viewModelInfo[client_index].mArmsOffsetMat.w.v =
            _mm_setr_ps(0.0f, 0.0f, 0.0f, 1.0f);
    }
    tr.viewModelInfo[client_index].mDrawBeforeWorld = 0;
    if (tr.viewModelInfo[client_index].mArmsScale == 1.0f)
    {
        tr.viewModelInfo[client_index].mArmsOffsetMat.w.v =
            _mm_setzero_ps();
    }
}

// ea: 0x006C0720
void nglStartOverExposureEx(float oeBloomRealStart, float oeBloomStart,
                            float oeBloomEnd, float oeBloomInTime,
                            float oeBloomDecay, float oeGammaStart,
                            float oeGammaDecay)
{
    bpOe.oeBloomInTime = (oeBloomStart - oeBloomRealStart) * (1.0f / oeBloomInTime);
    bpOe.oeBloomStart = oeBloomStart;
    bpOe.oeGammaLevel = 1.0f;
    bpOe.oeBloomRealStart = oeBloomRealStart;
    bpOe.oeGammaInTime = (oeGammaStart - 1.0f) * (1.0f / oeBloomInTime);
    bpOe.oeLeadTime = oeBloomInTime;
    bpOe.oeGammaStart = oeGammaStart;
    bpOe.oeBloomLevel = oeBloomRealStart;
    bpOe.oeGammaDecay = (1.0f / oeBloomDecay) * (oeGammaStart - 1.0f);
    bpOe.oeBloomDecay = (oeBloomStart - oeBloomEnd) * (1.0f / oeBloomDecay);
}

// ea: 0x006C07D0
void nglCycleOverExposure(BP_OVEREXPOSURE* overExposure, int frameTime)
{
    (void)overExposure;
    (void)frameTime;
}

// ============================================================================
// View-model mesh helpers (tr_viewmodel.cpp)
// ============================================================================

// ea: 0x006BE630
math::Mat43* nglListAddMesh_GetScaledMatrix(const math::Mat43& LocalToWorld,
                                            nglMeshParams* MeshParams)
{
    __m128 v2 = MeshParams->Scale.v;
    s_scaledInit |= 1u;
    __m128 v6 = _mm_mul_ps(LocalToWorld.x.v, _mm_shuffle_ps(v2, v2, 0));
    __m128 v3 = _mm_mul_ps(LocalToWorld.y.v, _mm_shuffle_ps(v2, v2, 85));
    __m128 v7 = _mm_mul_ps(LocalToWorld.z.v, _mm_shuffle_ps(v2, v2, 170));
    __m128 v4 = LocalToWorld.w.v;
    ScaledLocalToWorld.x.v = v6;
    ScaledLocalToWorld.y.v = v3;
    ScaledLocalToWorld.z.v = v7;
    ScaledLocalToWorld.w.v = v4;
    return &ScaledLocalToWorld;
}

// ea: 0x006BE890
void render_view_model_arms(int client_index, nglMeshParams& meshParams,
                            const math::Mat43& matrix,
                            math::Mat43& worldTrasform)
{
    if (tr.viewModelInfo[client_index].mArmsScale != 1.0f)
    {
        meshParams.Flags |= 2u;
        __m128 v12;
        v12.m128_f32[0] = tr.viewModelInfo[client_index].mArmsScale;
        v12.m128_f32[1] = v12.m128_f32[0];
        v12.m128_f32[2] = v12.m128_f32[0];
        v12.m128_f32[3] = 0.0f;
        meshParams.Scale.v = v12;
        const math::Mat43& off = tr.viewModelInfo[client_index].mArmsOffsetMat;
        if (off.w.v.m128_f32[0] != 0.0f || off.w.v.m128_f32[2] != 0.0f)
        {
            __m128 v5 = matrix.z.v;
            __m128 v6 = matrix.y.v;
            __m128 v7 = matrix.x.v;
            __m128 v10 = _mm_add_ps(
                _mm_add_ps(
                    _mm_mul_ps(_mm_shuffle_ps(off.x.v, off.x.v, 0),
                               matrix.x.v),
                    _mm_mul_ps(_mm_shuffle_ps(off.x.v, off.x.v, 85), v6)),
                _mm_mul_ps(_mm_shuffle_ps(off.x.v, off.x.v, 170), v5));
            __m128 v8 = off.w.v;
            __m128 v9 = matrix.w.v;
            __m128 v11 = _mm_add_ps(
                _mm_add_ps(
                    _mm_mul_ps(_mm_shuffle_ps(off.y.v, off.y.v, 0),
                               matrix.x.v),
                    _mm_mul_ps(_mm_shuffle_ps(off.y.v, off.y.v, 85), v6)),
                _mm_mul_ps(_mm_shuffle_ps(off.y.v, off.y.v, 170), v5));
            worldTrasform.x.v = _mm_add_ps(
                _mm_add_ps(
                    _mm_mul_ps(_mm_shuffle_ps(off.z.v, off.z.v, 0),
                               matrix.x.v),
                    _mm_mul_ps(_mm_shuffle_ps(off.z.v, off.z.v, 85), v6)),
                _mm_mul_ps(_mm_shuffle_ps(off.z.v, off.z.v, 170), v5));
            worldTrasform.y.v = v10;
            worldTrasform.z.v = v11;
            worldTrasform.w.v = _mm_add_ps(
                _mm_add_ps(
                    _mm_mul_ps(_mm_shuffle_ps(v8, v8, 0), v7),
                    _mm_mul_ps(_mm_shuffle_ps(v8, v8, 85), v6)),
                _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(v8, v8, 170), v5),
                           v9));
        }
        meshParams.Flags |= 0x40u;
    }
}

// ea: 0x006BEA90
void render_view_model_weapon(int client_index, nglMeshParams& meshParams,
                              math::Mat43& modelTransform)
{
    if (tr.viewModelInfo[client_index].mWeaponScale != 1.0f)
    {
        meshParams.Flags |= 2u;
        __m128 v3;
        v3.m128_f32[0] = tr.viewModelInfo[client_index].mWeaponScale;
        v3.m128_f32[1] = v3.m128_f32[0];
        v3.m128_f32[2] = v3.m128_f32[0];
        v3.m128_f32[3] = 0.0f;
        meshParams.Scale.v = v3;
        if (tr.viewModelInfo[client_index].mScaleWeaponTrans != 0)
        {
            float scale = tr.viewModelInfo[client_index].mWeaponScale;
            const math::Position3& org =
                tr.viewModelInfo[client_index].mWeaponOrigin;
            modelTransform.w.v.m128_f32[0] =
                ((modelTransform.w.v.m128_f32[0] - org.v.m128_f32[0])
                 * scale)
                + org.v.m128_f32[0];
            modelTransform.w.v.m128_f32[1] =
                ((modelTransform.w.v.m128_f32[1] - org.v.m128_f32[1])
                 * scale)
                + org.v.m128_f32[1];
            modelTransform.w.v.m128_f32[2] =
                ((modelTransform.w.v.m128_f32[2] - org.v.m128_f32[2])
                 * scale)
                + org.v.m128_f32[2];
        }
        meshParams.Flags |= 0x40u;
    }
}
