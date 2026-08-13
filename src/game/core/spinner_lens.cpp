// ============================================================================
// spinner_lens.cpp - spinner + lens flare draw (core.o)
// ============================================================================

#include "game/core/core_systems.h"
#include "game/core/core_globals.h"

#include <intrin.h>
#include <math.h>
#include <string.h>

struct nglQuad;
struct nglTexture;
extern void nglInitQuad(nglQuad* quad);
extern void nglSetQuadRect(nglQuad* quad, float x1, float y1, float x2,
                           float y2);
extern void nglSetQuadColor(nglQuad* quad, unsigned int c);
extern void nglSetQuadZ(nglQuad* quad, float z);
extern void nglSetQuadBlend(nglQuad* quad, unsigned int blend);
extern void nglSetQuadTex(nglQuad* quad, nglTexture* tex);
extern void nglListAddQuad(nglQuad* quad);
struct nglFont;
extern void nglListAddString(nglFont* font, const char* text, float x,
                             float y, float z, unsigned int color,
                             float scaleX, float scaleY);
extern void nglGetStringDimensions(nglFont* font, unsigned int* width,
                                   unsigned int* height, float scaleX,
                                   float scaleY, const char* fmt, ...);
extern void nglSetClearFlags(unsigned int clearFlags);
extern void nglSetZWriteEnable(bool enable);
extern void nglWaitForRendering();
extern void nglPresent();
extern int nglGetScreenWidth();
extern int nglGetScreenHeight();
extern nglTexture* nglGetTexture(const tlFixedString& fileName);
extern void* FEManager_GetFont(void* mgr, int f);
extern unsigned int AeHash(const char* str);
extern const float VectorNormalize(float* const v);
struct nglScene;
extern math::Position3* nglProjectPoint(math::Position3* result,
                                        const math::Position3* in,
                                        nglScene* scene);
extern const math::Mat43* nglGetMatrix_ViewToWorld(nglScene* scene);
extern nglScene* nglBuildScene;
void* gpBrocAPI;  // ?gpBrocAPI (scr.o artifact PAXA)
extern unsigned int BrocAPI_GetEnt(void* api, void* nameStr, unsigned int hash,
                                   void* a3, int a4, int a5);
extern void* dword_F00ED4;
extern void* dword_F00ED8;
extern void* dword_F00EDC;
extern void* dword_F00EE0;
float l;
float t;
float r_0;
float b_0;
struct FEManager; extern FEManager g_femanager;

class EntityHandleDb {
public:
    struct DbElement {
        Entity* mObject;  // +0x00
        int     mKey;     // +0x04
    };
    unsigned char _pad[0xA8];
    DbElement     mElements[0x540];
    static EntityHandleDb sInst;
};

enum {
    kLanguageEnglish = 0,
    kLanguageGerman = 1,
    kLanguageFrench = 2,
    kLanguageSpanish = 3,
    kLanguageItalian = 4,
};

// ea: 0x004BD900
void SpinnerDrawLoading()
{
    const char* v0 = "Loading ...";
    switch (gLanguage)
    {
    case kLanguageGerman: v0 = "Laden ..."; break;
    case kLanguageFrench: v0 = "Chargement"; break;
    case kLanguageSpanish: v0 = "Cargando ..."; break;
    case kLanguageItalian: v0 = "Caricamento ..."; break;
    default: break;
    }
    unsigned char loadImage[0x60];
    nglInitQuad((nglQuad*)loadImage);
    nglSetQuadRect((nglQuad*)loadImage, 0.0f, 0.0f, 640.0f, 480.0f);
    nglSetQuadColor((nglQuad*)loadImage, 0x96000000);
    nglSetQuadZ((nglQuad*)loadImage, 300.0f);
    nglSetQuadBlend((nglQuad*)loadImage, 0x64CF8600);
    void* Font = FEManager_GetFont(&g_femanager, 0);
    if (Font != nullptr)
    {
        unsigned int x, y;
        nglGetStringDimensions((nglFont*)Font, &x, &y, 0.5f, 0.5f, v0);
        x = 535 - x;
        for (int i = 2; i != 0; --i)
        {
            nglSetClearFlags(3u);
            nglListAddQuad((nglQuad*)loadImage);
            nglListAddString((nglFont*)Font, v0, (float)x, 425.0f, 0.0f,
                             0xFFB18E5D, 0.5f, 0.5f);
        }
    }
}

// ea: 0x004BDA30
void SpinnerDraw(float a)
{
    (void)a;
    unsigned char q[0x60];
    nglInitQuad((nglQuad*)q);
    nglSetQuadRect((nglQuad*)q, l, t, r_0, b_0);
    nglSetQuadTex((nglQuad*)q,
                  (nglTexture*)sSpinnerFrames[sLastSpinnerFrame]);
    nglSetQuadBlend((nglQuad*)q, 0x64CF8600);
    nglListAddQuad((nglQuad*)q);
}

static unsigned long long sSpinnerLast1 = 0;
static unsigned int sSpinnerInit1 = 0;
static float sSpinnerDelta1 = 0.0f;

// ea: 0x004C14E0
void SpinnerDrawFrame(bool bEndFrame)
{
    unsigned long long v1;
    if ((sSpinnerInit1 & 1) != 0)
        v1 = sSpinnerLast1;
    else
    {
        sSpinnerInit1 |= 1u;
        v1 = __rdtsc();
    }
    unsigned long long v4 = __rdtsc();
    unsigned long long v3 = v4 - v1;
    sSpinnerDelta1 = (float)(double)v3 / 733333.31f * 0.001f + sSpinnerDelta1;
    unsigned int frames = 0;
    if (sSpinnerDelta1 >= 0.15000001f)
    {
        float v5 = sSpinnerDelta1;
        do
        {
            v5 = v5 - 0.15000001f;
            ++frames;
        } while (v5 >= 0.15000001f);
        sSpinnerDelta1 = v5;
    }
    sLastSpinnerFrame = (int)((frames + sLastSpinnerFrame) & 7);
    sSpinnerLast1 = v4;
    if (bEndFrame)
        nglWaitForRendering();
    int v6 = bEndFrame ? 1 : 2;
    for (int i = v6; i != 0; --i)
    {
        if (bEndFrame)
        {
            nglSetClearFlags(0xF3u);
            nglSetZWriteEnable(false);
        }
        unsigned char quad[0x60];
        nglInitQuad((nglQuad*)quad);
        nglSetQuadRect((nglQuad*)quad, l, t, r_0, b_0);
        nglSetQuadTex((nglQuad*)quad,
                      (nglTexture*)sSpinnerFrames[sLastSpinnerFrame]);
        nglSetQuadBlend((nglQuad*)quad, 0x64CF8600);
        nglListAddQuad((nglQuad*)quad);
        if (bEndFrame)
            nglPresent();
    }
}

static unsigned long long sSpinnerLast2 = 0;
static unsigned int sSpinnerInit2 = 0;
static float sSpinnerDelta2 = 0.0f;

// ea: 0x004C1690
void SpinnerDrawFrameWithLoading(bool bEndFrame)
{
    unsigned long long v1;
    if ((sSpinnerInit2 & 1) != 0)
        v1 = sSpinnerLast2;
    else
    {
        sSpinnerInit2 |= 1u;
        v1 = __rdtsc();
    }
    unsigned long long v4 = __rdtsc();
    sSpinnerDelta2 = (float)(double)(v4 - v1) / 733333.31f * 0.001f
                     + sSpinnerDelta2;
    unsigned int frames = 0;
    if (sSpinnerDelta2 >= 0.15000001f)
    {
        float v5 = sSpinnerDelta2;
        do
        {
            v5 = v5 - 0.15000001f;
            ++frames;
        } while (v5 >= 0.15000001f);
        sSpinnerDelta2 = v5;
    }
    sLastSpinnerFrame = (int)((frames + sLastSpinnerFrame) & 7);
    sSpinnerLast2 = v4;
    if (bEndFrame)
        nglWaitForRendering();
    int v6 = bEndFrame ? 1 : 2;
    for (int i = v6; i != 0; --i)
    {
        if (bEndFrame)
        {
            nglSetClearFlags(0xF3u);
            nglSetZWriteEnable(false);
        }
        SpinnerDrawLoading();
        unsigned char quad[0x60];
        nglInitQuad((nglQuad*)quad);
        nglSetQuadRect((nglQuad*)quad, l, t, r_0, b_0);
        nglSetQuadTex((nglQuad*)quad,
                      (nglTexture*)sSpinnerFrames[sLastSpinnerFrame]);
        nglSetQuadBlend((nglQuad*)quad, 0x64CF8600);
        nglListAddQuad((nglQuad*)quad);
        if (bEndFrame)
            nglPresent();
    }
}

// ea: 0x004CE990
void LensFlareInit()
{
    Broc::string v9("lightsource");
    unsigned int v1 = BrocAPI_GetEnt(gpBrocAPI, &v9, AeHash("targetname"),
                                     nullptr, 0, 1);
    unsigned int result = v1 & 0xFFF;
    Entity* mObject = nullptr;
    if (result < 0x540 && v1 >> 12 == EntityHandleDb::sInst.mElements[result].mKey)
        mObject = EntityHandleDb::sInst.mElements[result].mObject;
    gLensLightSource = mObject;
    if (mObject != nullptr)
    {
        tlFixedString FileName("dynamiclight");
        gLensFlareTextures[0] = nglGetTexture(FileName);
        tlFixedString v8("lensflare1");
        dword_F00ED4 = nglGetTexture(v8);
        tlFixedString v7("lensflare2");
        dword_F00ED8 = nglGetTexture(v7);
        tlFixedString v6("lensflare3");
        dword_F00EDC = nglGetTexture(v6);
        tlFixedString v4("lensflare4");
        dword_F00EE0 = nglGetTexture(v4);
    }
}

// ea: 0x004C1850
void LensFlareDraw()
{
    if (gLensLightSource == nullptr)
        return;
    Entity* light = (Entity*)gLensLightSource;
    float org[4] = {0.0f,
                    light->r.currentOrigin.v.m128_f32[0],
                    light->r.currentOrigin.v.m128_f32[1],
                    light->r.currentOrigin.v.m128_f32[2]};
    float context[10];
    memset(context, 0, sizeof(context));
    context[3] = org[1];
    context[4] = org[2];
    context[5] = org[3];
    nglProjectPoint((math::Position3*)&org[1], (math::Position3*)&context[3],
                    nglBuildScene);
    int v2 = 0;
    context[3] = (float)nglGetScreenWidth() * 0.5f;
    context[4] = (float)nglGetScreenHeight() * 0.5f;
    if (org[1] > (float)(nglGetScreenWidth() + 50) || org[1] < -50.0f)
    {
        if (gLensAlphaAmount == 0)
            return;
        v2 = 1;
    }
    if (org[2] > (float)(nglGetScreenHeight() + 50) || org[2] < -50.0f)
    {
        if (gLensAlphaAmount == 0)
            return;
        v2 = 1;
    }
    const math::Mat43* Matrix_ViewToWorld =
        nglGetMatrix_ViewToWorld(nglBuildScene);
    math::Mat43 cameraMtx;
    memset(&cameraMtx, 0, sizeof(cameraMtx));
    cameraMtx.y.v.m128_f32[1] = Matrix_ViewToWorld->z.v.m128_f32[0];
    cameraMtx.y.v.m128_f32[2] = Matrix_ViewToWorld->z.v.m128_f32[1];
    cameraMtx.y.v.m128_f32[3] = Matrix_ViewToWorld->z.v.m128_f32[2];
    float v5 = Matrix_ViewToWorld->z.v.m128_f32[3];
    cameraMtx.z.v.m128_f32[0] = v5;
    const math::Mat43* next = (const math::Mat43*)((const char*)Matrix_ViewToWorld + 48);
    float v6 = next->x.v.m128_f32[0];
    float v7 = next->x.v.m128_f32[1];
    float v8 = next->x.v.m128_f32[2];
    float v9 = next->x.v.m128_f32[3];
    cameraMtx.w.v.m128_f32[1] = v6;
    cameraMtx.w.v.m128_f32[2] = v7;
    cameraMtx.w.v.m128_f32[3] = v8;
    math::Dir3 v33;
    v33.v.m128_f32[0] = v6;
    v33.v.m128_f32[1] = v7;
    v33.v.m128_f32[2] = v8;
    v33.v.m128_f32[3] = v9;
    math::Position3 start;
    start.v.m128_f32[1] = org[1];
    start.v.m128_f32[2] = org[2];
    start.v.m128_f32[3] = org[3];
    v33.v.m128_f32[0] = org[1] - v33.v.m128_f32[0];
    v33.v.m128_f32[1] = org[2] - v33.v.m128_f32[1];
    v33.v.m128_f32[2] = org[3] - v8;
    VectorNormalize((float*)&v33);
    float v10 = (cameraMtx.y.v.m128_f32[3] * v33.v.m128_f32[2])
                + (cameraMtx.y.v.m128_f32[2] * v33.v.m128_f32[1])
                + (cameraMtx.y.v.m128_f32[1] * v33.v.m128_f32[0]);
    if (v10 < 0.0f)
    {
        if (gLensAlphaAmount == 0)
            return;
        v2 = 1;
    }
    float alphaScale = 0.0f;
    if (v10 > 0.89999998f)
        alphaScale = (v10 - 0.89999998f) / 0.2f;
    if (gLensAlphaAmount == 0)
        ;
    int v12 = v2 != 0 ? gLensAlphaAmount - 20 : gLensAlphaAmount + 40;
    gLensAlphaAmount = v12;
    if (v12 >= 0)
    {
        if (v12 > 124)
            gLensAlphaAmount = 124;
    }
    else
    {
        gLensAlphaAmount = 0;
    }
    float v32 = (float)sqrt((double)((org[1] - context[3]) * (org[1] - context[3])
                                     + (org[2] - context[4]) * (org[2] - context[4])));
    float v13 = v32 / 2.0f;
    v32 = v32 / 2.0f;
    for (int v14 = 0; v14 < 5; ++v14)
    {
        float v15 = ((gLensScaleAmount * alphaScale) * 4.0f) + gLensScaleAmount;
        float x1 = (org[1] - v15) + ((float)v14 * v33.v.m128_f32[0]) * v13;
        float x2 = (v15 + org[1]) + ((float)v14 * v33.v.m128_f32[1]) * v13;
        float y1 = (org[2] - v15) + ((float)v14 * v33.v.m128_f32[1]) * v13;
        float y2 = (v15 + org[2]) + ((float)v14 * v33.v.m128_f32[1]) * v13;
        unsigned char quad[0x60];
        nglInitQuad((nglQuad*)quad);
        nglSetQuadRect((nglQuad*)quad, x1, y1, x2, y2);
        nglSetQuadTex((nglQuad*)quad,
                      (nglTexture*)gLensFlareTextures[v14]);
        nglSetQuadColor((nglQuad*)quad,
                        0xFFFFFF | (gLensAlphaAmount << 24));
        nglSetQuadBlend((nglQuad*)quad, 0x64078600);
        nglListAddQuad((nglQuad*)quad);
        v13 = v32;
    }
}
