// ============================================================================
// tr_fx.cpp - render.o blur/gamma/zbuffer/scene-callback helpers (tr_main.cpp)
// Types and bodies verified against IDA (codmp_xboxr.xbe).
// ============================================================================

#include "ngl/ngl_dx_quad.h"
#include "ngl/ngl_scene.h"
#include "render/ShaderCommon.h"
#include "game/nextgen/nextgen.h"

#include <intrin.h>

extern bool _tlAssert(const char* file, int line, const char* expr,
                      const char* desc);  // ?_tlAssert@@YA_NPBDH00@Z

// FULLSCREENBLUR_STATE (IDA enum; codmp_xboxr.xbe.h)
enum FULLSCREENBLUR_STATE {
    FULLSCREENBLUR_OFF = 0x0,
    FULLSCREENBLUR_START = 0x1,
    FULLSCREENBLUR_RUNNING = 0x2,
    FULLSCREENBLUR_FINISHED = 0x3,
    FULLSCREENBLUR_THISFRAMEONLY = 0x4,
};

FULLSCREENBLUR_STATE g_doFullScreenBlur[1];  // ?g_doFullScreenBlur@@3PAW4FULLSCREENBLUR_STATE@@A @ 0xF743FC
float g_fullScreenBlurAmount[1];             // ?g_fullScreenBlurAmount@@3PAMA @ 0xF743F8
extern float g_screendelta;                  // ?g_screendelta@@3MA (core.o)

struct View_Window {
    float XPos;    // +0x00
    float YPos;    // +0x04
    float Width;   // +0x08
    float Height;  // +0x0C
    float FovX;    // +0x10
    float FovY;    // +0x14
    unsigned int Safety;  // +0x18
};
namespace View {
const View_Window* GetCurrentWindow(int clientIndex);  // ?GetCurrentWindow@View@@YAPBUView_Window@@H@Z
}
extern int currCl;  // ?currCl@@3HA
extern nglScene* nglBuildScene;  // ?nglBuildScene@@3PAUnglScene@@A

// ============================================================================
// NGLPreSceneCallBack - ea: 0x006C2120
// ============================================================================
void NGLPreSceneCallBack(void* Data)
{
    (void)Data;
}

// ============================================================================
// HandleFullScreenBlur - ea: 0x006C2130
// ============================================================================
void HandleFullScreenBlur(int viewport)
{
    FULLSCREENBLUR_STATE v1 = g_doFullScreenBlur[viewport];
    if (v1 != FULLSCREENBLUR_OFF)
    {
        if (v1 == FULLSCREENBLUR_START)
        {
            g_fullScreenBlurAmount[viewport] = 0.0f;
            g_doFullScreenBlur[viewport] = FULLSCREENBLUR_RUNNING;
        }
        CG_SceneBlur::Set((int)(g_fullScreenBlurAmount[viewport] * 10.0f), 1.0f);
        if (g_doFullScreenBlur[viewport] == FULLSCREENBLUR_THISFRAMEONLY)
        {
            g_fullScreenBlurAmount[viewport] = 0.0f;
            g_doFullScreenBlur[viewport] = FULLSCREENBLUR_OFF;
        }
        else if (g_doFullScreenBlur[viewport] != FULLSCREENBLUR_FINISHED)
        {
            float v2 = (g_screendelta * 0.5f) + g_fullScreenBlurAmount[viewport];
            g_fullScreenBlurAmount[viewport] = v2;
            if (v2 > 1.0f)
            {
                g_fullScreenBlurAmount[viewport] = 1.0f;
                g_doFullScreenBlur[viewport] = FULLSCREENBLUR_FINISHED;
            }
        }
    }
    else
    {
        CG_SceneBlur::End();
    }
}

// ============================================================================
// R_ApplyGammaCorrection - ea: 0x006C21F0
// ============================================================================
void R_ApplyGammaCorrection(bool b, int h)
{
    (void)b;
    (void)h;
}

// ============================================================================
// XboxNGLPostSceneCallBack - ea: 0x006C2360
// ============================================================================
void XboxNGLPostSceneCallBack(void* Data)
{
    (void)Data;
    ShaderCommon::HeatHazeCallback(nullptr);
}

// ============================================================================
// R_SetWindowQuadRect - ea: 0x006C1E70
// ============================================================================
void R_SetWindowQuadRect(nglQuad& q)
{
    const View_Window* CurrentWindow = View::GetCurrentWindow(currCl);
    float x1 = (CurrentWindow->XPos + 1.0f) * 0.5f;
    float y1 = (CurrentWindow->YPos + 1.0f) * 0.5f;
    float x2 = ((CurrentWindow->Width + CurrentWindow->XPos) + 1.0f) * 0.5f;
    float y2 = ((CurrentWindow->Height + CurrentWindow->YPos) + 1.0f) * 0.5f;
    float v5 = (float)nglGetScreenHeight() * y2;
    float v4 = (float)nglGetScreenWidth() * x2;
    float v3 = (float)nglGetScreenHeight() * y1;
    float v2 = (float)nglGetScreenWidth() * x1;
    nglSetQuadRect(&q, v2, v3, v4, v5);
}

// ============================================================================
// R_GetZBufferValue - ea: 0x006C1F50
// ============================================================================
float R_GetZBufferValue(float x, float y)
{
    if (x < 0.0f || x >= 640.0f || y < 0.0f || y >= 480.0f)
        return 3.4028235e38f;

    nglValidateMatrices(nglBuildScene);
    if (_tlAssert("c:\\cod\\code\\game\\tr_main.cpp", 1999, "false",
                  "Platform does not support Z-buffer reads."))
    {
        __debugbreak();
    }
    nglValidateMatrices(nglBuildScene);

    __m128 zero = _mm_setzero_ps();
    __m128 v2 = _mm_add_ps(
        _mm_add_ps(
            _mm_mul_ps(_mm_shuffle_ps(zero, zero, 0), nglBuildScene->ViewportToWorld.x.v),
            _mm_mul_ps(_mm_shuffle_ps(zero, zero, 85), nglBuildScene->ViewportToWorld.y.v)),
        _mm_add_ps(
            _mm_mul_ps(_mm_shuffle_ps(zero, zero, 170), nglBuildScene->ViewportToWorld.z.v),
            nglBuildScene->ViewportToWorld.w.v));
    __m128 v3 = _mm_add_ps(
        _mm_add_ps(
            _mm_mul_ps(_mm_shuffle_ps(v2, v2, 0), nglBuildScene->WorldToView.x.v),
            _mm_mul_ps(_mm_shuffle_ps(v2, v2, 85), nglBuildScene->WorldToView.y.v)),
        _mm_add_ps(
            _mm_mul_ps(_mm_shuffle_ps(v2, v2, 170), nglBuildScene->WorldToView.z.v),
            _mm_mul_ps(_mm_shuffle_ps(v2, v2, 255), nglBuildScene->WorldToView.w.v)));
    __m128 v4 = _mm_shuffle_ps(v3, _mm_shuffle_ps(v2, v3, 175), 52);
    float v7 = _mm_shuffle_ps(v4, v4, 255).m128_f32[0];
    __m128 v5 = _mm_div_ps(v4, _mm_shuffle_ps(_mm_set1_ps(v7), _mm_set1_ps(v7), 0));
    return _mm_shuffle_ps(v5, v5, 170).m128_f32[0];
}
