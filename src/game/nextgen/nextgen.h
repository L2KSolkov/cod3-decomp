// ============================================================================
// nextgen.h â€” next-gen rendering shims (CG_MotionBlur, CG_SceneBlur)
// Source: nextgen.o (game object, no lib prefix)
// Verified against IDA (release codmp_xboxr.xbe):
//   CG_MotionBlur::Begin          @0x6F0790
//   CG_MotionBlur::End            @0x6F07D0
//   CG_MotionBlur::Callback       @0x6F07E0
//   CG_MotionBlur::AddPostCallback @0x6F0880
//   CG_SceneBlur::blurCallBack    @0x6F0920
//   CG_SceneBlur::Set             @0x6F0960
//   CG_SceneBlur::End             @0x6F0980
//   CG_SceneBlur::AddPostCallback @0x6F0990
// ============================================================================

#pragma once

struct nglTexture;
struct nglQuad;

// ============================================================================
// CG_MotionBlur â€” screen-space motion-blur overlay
// ============================================================================
class CG_MotionBlur {
public:
    static void Begin(float level, float plateauTime, float fadeTime);
    static void End();
    static void Callback(void* data);
    static void AddPostCallback();
};

// ============================================================================
// CG_SceneBlur â€” full-screen blur pass
// ============================================================================
class CG_SceneBlur {
public:
    static void blurCallBack(void* data);
    static void Set(int passes, float expansion);
    static void End();
    static void AddPostCallback();

    static int   s_passes;    // ?s_passes@CG_SceneBlur@@3HA
    static float s_expansion; // ?s_expansion@CG_SceneBlur@@3MA
};
