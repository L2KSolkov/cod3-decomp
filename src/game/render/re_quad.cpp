// ============================================================================
// re_quad.cpp - render.o quad/font/stream helpers (tr_shader.cpp, renderer)
// ============================================================================

#include "game/logic/g_local.h"
#include "ngl/ngl_dx_quad.h"
#include "ngl/nglFont.h"

#include <string.h>
#include <intrin.h>

struct nglScene;
extern nglScene* nglBuildScene;   // ?nglBuildScene@@3PAUnglScene@@A
extern int sCurColor;             // ?sCurColor@@3IA @ 0xF74290
extern float sGlobalFontScale;    // ?sGlobalFontScale@@3MA @ 0xDFA444
extern unsigned char* fdFile;     // ?fdFile@@3PAEA @ 0xF742AC
extern int fdOffset;              // ?fdOffset@@3HA @ 0xF743AC

// refimport_t ri (cl.o) - UI_GetFontInfo only used here
struct refimport_t {
    void* (*UI_GetFontInfo)(int font, float scale);
};
extern refimport_t ri;            // ?ri@@3Urefimport_t@@A @ 0xF741E8

extern void nglInitQuad(nglQuad* Quad);  // ngl.o
extern void nglSetQuadRect(nglQuad* Quad, float x1, float y1, float x2,
                           float y2);  // ngl.o
extern void nglSetQuadUV(nglQuad* Quad, float u1, float v1, float u2,
                         float v2);  // ngl.o
extern void nglSetQuadColor(nglQuad* Quad, unsigned int c);  // ngl.o
extern void nglSetQuadZ(nglQuad* Quad, float z);  // ngl.o
extern void nglListAddQuad(nglQuad* Quad);  // ngl.o

// ea: 0x006BEDB0
void RE_StretchPic(float x, float y, float w, float h, float s1, float t1,
                   float s2, float t2, nglTexture* tex, float z)
{
    if (nglBuildScene != nullptr)
    {
        nglQuad q;
        nglInitQuad(&q);
        nglSetQuadRect(&q, x, y, x + w, y + h);
        nglSetQuadUV(&q, s1, t1, s2, t2);
        nglSetQuadColor(&q, (unsigned int)sCurColor);
        nglSetQuadZ(&q, z);
        q.Tex = tex;
        nglListAddQuad(&q);
    }
}

// ea: 0x006BEE50
void RE_DrawQuadPic(const float (*vVerts)[2], const float (*vST)[2],
                    nglTexture* tex)
{
    if (nglBuildScene != nullptr)
    {
        nglQuad q;
        nglInitQuad(&q);
        nglSetQuadColor(&q, (unsigned int)sCurColor);
        q.Verts[0].X = vVerts[0][0];
        q.Verts[0].Y = vVerts[0][1];
        q.Verts[1].X = vVerts[1][0];
        q.Verts[1].Y = vVerts[1][1];
        q.Verts[3].X = vVerts[3][0];
        q.Verts[3].Y = vVerts[3][1];
        q.Verts[2].X = vVerts[2][0];
        q.Verts[2].Y = vVerts[2][1];
        q.Verts[0].U = vST[0][0];
        q.Verts[0].V = vST[0][1];
        q.Verts[1].U = vST[1][0];
        q.Verts[1].V = vST[1][1];
        q.Verts[3].U = vST[3][0];
        q.Verts[3].V = vST[3][1];
        q.Verts[2].U = vST[2][0];
        q.Verts[2].V = vST[2][1];
        q.Tex = tex;
        nglListAddQuad(&q);
    }
}

// ea: 0x006BFF30
int readInt()
{
    int result = fdFile[fdOffset]
                 + ((fdFile[fdOffset + 1]
                     + ((fdFile[fdOffset + 2] + (fdFile[fdOffset + 3] << 8))
                        << 8))
                    << 8);
    fdOffset += 4;
    return result;
}

// ea: 0x006BFF70
float readFloat()
{
    float me;
    memcpy(&me, &fdFile[fdOffset], 4);
    fdOffset += 4;
    return me;
}

// ea: 0x006BFFB0
void R_InitFreeType()
{
}

// ea: 0x006BFFC0
void R_DoneFreeType()
{
}

// ea: 0x006BFFD0
nglFont* R_GetFontInfo(int font, float scale)
{
    return (nglFont*)ri.UI_GetFontInfo(font, sGlobalFontScale * scale);
}

// ea: 0x006C0000
int RE_Text_Height(int font, float scale)
{
    float scalea = sGlobalFontScale * scale;
    nglFont* v2 = (nglFont*)ri.UI_GetFontInfo(font, scalea);
    unsigned int width, height;
    nglGetStringDimensions(v2, &width, &height, scalea, scalea, "LltygI");
    return (int)height;
}

// ea: 0x006BFB40
void RE_SetCullDist(float dist)
{
    extern float g_dpvs_cullDist;
    g_dpvs_cullDist = 0.0f;
    if (dist > 0.0f)
        g_dpvs_cullDist = dist;
}
