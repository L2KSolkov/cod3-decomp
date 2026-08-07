// ============================================================================
// nextgen.cpp â€” next-gen rendering shims (CG_MotionBlur, CG_SceneBlur)
// Source: nextgen.o (game object, no lib prefix)
// Verified against IDA (release codmp_xboxr.xbe).
// ============================================================================

#include "game/nextgen/nextgen.h"

#include "game/sv/server_types.h"  // netadr_t, client_s (sv_stubs.h deps)
#include "game/sv/sv_stubs.h"      // ServerTime
#include "ngl/ngl_dx_quad.h"   // nglQuad, nglBuildScene
#include "ngl/nglTexture.h"

// ============================================================================
// Cross-object externs (ngl_xboxr)
// ============================================================================
extern void nglInitQuad(nglQuad* Quad);
extern void nglSetQuadBlend(nglQuad* Quad, unsigned int Blend);
extern void nglSetQuadRect(nglQuad* Quad, float x1, float y1, float x2, float y2);
extern void nglSetQuadUV(nglQuad* Quad, float u1, float v1, float u2, float v2);
extern nglTexture* nglGetFrontBufferTex(void);
extern void nglSetQuadTex(nglQuad* Quad, nglTexture* Tex);
extern void nglRenderQuad(nglQuad* Quad);
extern void nglSetSceneCallBack(nglSceneCallbackType Type, void (*Fn)(void*), void* Data);

// nglDxFilters (ngl_xboxr) â€” minimal view; full definition when ngl_dx_filters.o is ported
struct nglDxFilters {
    static void RenderBlur(nglTexture* SrcTex, nglTexture* DstTex);
};

// ============================================================================
// CG_MotionBlur statics (file-local, .data @0xE01E20..0xE01E30)
// ============================================================================
static float s_curLevel;     // 0xE01E20
static float s_plateauTime;  // 0xE01E24
static float s_fadeTime;     // 0xE01E28
static float s_maxLevel;     // 0xE01E2C
static float s_effectTime;   // 0xE01E30

// ============================================================================
// CG_MotionBlur::Begin â€” ea: 0x6F0790
// ============================================================================
void CG_MotionBlur::Begin(float level, float plateauTime, float fadeTime) {
    s_maxLevel = level;
    s_plateauTime = plateauTime;
    s_fadeTime = fadeTime;
    s_effectTime = 0.0f;
}

// ============================================================================
// CG_MotionBlur::End â€” ea: 0x6F07D0
// ============================================================================
void CG_MotionBlur::End() {
    s_curLevel = 0.0f;
}

// ============================================================================
// CG_MotionBlur::Callback â€” ea: 0x6F07E0
// ============================================================================
void CG_MotionBlur::Callback(void* data) {
    (void)data;
    nglQuad quad;
    nglInitQuad(&quad);

    int v0 = (int)(s_curLevel * 255.0f);
    if (v0 > 255)
        v0 = 255;
    else if (v0 < 0)
        v0 = 0;

    nglSetQuadBlend(&quad, (unsigned int)v0 | 0x87128600);
    nglSetQuadRect(&quad, 0.0f, 0.0f, 640.0f, 480.0f);
    nglSetQuadUV(&quad, 0.0f, 0.0f, 1.0f, 1.0f);
    nglTexture* FrontBufferTex = nglGetFrontBufferTex();
    nglSetQuadTex(&quad, FrontBufferTex);
    nglRenderQuad(&quad);
}

// ============================================================================
// CG_MotionBlur::AddPostCallback â€” ea: 0x6F0880
// ============================================================================
void CG_MotionBlur::AddPostCallback() {
    float v0 = ServerTime::sInst.mTickDelta + s_effectTime;
    s_effectTime = ServerTime::sInst.mTickDelta + s_effectTime;
    if (s_effectTime <= (s_fadeTime + s_plateauTime)) {
        float v1;
        if (v0 <= s_plateauTime)
            v1 = s_maxLevel;
        else
            v1 = (1.0f - ((v0 - s_plateauTime) / s_fadeTime)) * s_maxLevel;
        s_curLevel = v1;
        if (v1 != 0.0f)
            nglSetSceneCallBack(NGLSCENE_POST, CG_MotionBlur::Callback, NULL);
    } else {
        s_curLevel = 0.0f;
    }
}

// ============================================================================
// CG_SceneBlur statics
// ============================================================================
int   CG_SceneBlur::s_passes = 0;     // ?s_passes@CG_SceneBlur@@3HA 0xF7945C
float CG_SceneBlur::s_expansion = 0.0f;  // ?s_expansion@CG_SceneBlur@@3MA 0xE01E34

// ============================================================================
// CG_SceneBlur::blurCallBack â€” ea: 0x6F0920
// ============================================================================
void CG_SceneBlur::blurCallBack(void* data) {
    (void)data;
    for (int i = 0; i < CG_SceneBlur::s_passes; ++i)
        nglDxFilters::RenderBlur(nglBuildScene->RenderTarget, nglBuildScene->RenderTarget);
}

// ============================================================================
// CG_SceneBlur::Set â€” ea: 0x6F0960
// ============================================================================
void CG_SceneBlur::Set(int passes, float expansion) {
    CG_SceneBlur::s_passes = passes;
    CG_SceneBlur::s_expansion = expansion;
}

// ============================================================================
// CG_SceneBlur::End â€” ea: 0x6F0980
// ============================================================================
void CG_SceneBlur::End() {
    CG_SceneBlur::s_passes = 0;
}

// ============================================================================
// CG_SceneBlur::AddPostCallback â€” ea: 0x6F0990
// ============================================================================
void CG_SceneBlur::AddPostCallback() {
    if (CG_SceneBlur::s_passes > 0)
        nglSetSceneCallBack(NGLSCENE_POST, CG_SceneBlur::blurCallBack, NULL);
}
