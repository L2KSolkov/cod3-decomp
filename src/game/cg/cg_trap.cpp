// ============================================================================
// cg_trap.cpp - client renderer trap wrappers (cg.o)
// ============================================================================

#include "game/cg/cg_local.h"

extern void CG_DrawInformation();
struct trStatistics_t;

// ea: 0x0068C870
void trap_R_SetCullDist(float dist)
{
    re.SetCullDist(dist);
}

// ea: 0x0068C880
void trap_R_SetFog(int fogvar, int var1, int var2, float r, float g, float b,
                   float density)
{
    re.SetFog(fogvar, var1, var2, r, g, b, density);
}

// ea: 0x0068C890
void trap_R_SaveScreen()
{
    re.SaveScreen();
}

// ea: 0x0068C8A0
void trap_R_DrawStretchPicGradient(float x, float y, float w, float h,
                                   float s1, float t1, float s2, float t2,
                                   nglTexture* tex, const float* gradientColor,
                                   int gradientType)
{
    re.DrawStretchPicGradient(x, y, w, h, s1, t1, s2, t2, tex, gradientColor,
                              gradientType);
}

// ea: 0x0068C8B0
void trap_R_DrawStretchPicRotate(float x, float y, float w, float h, float s1,
                                 float t1, float s2, float t2, float fRot,
                                 nglTexture* tex)
{
    re.DrawStretchPicRotate(x, y, w, h, s1, t1, s2, t2, fRot, tex);
}

// ea: 0x0068C8C0
void trap_R_DrawQuadPic(const float (*vVerts)[2], const float (*vST)[2],
                        nglTexture* tex)
{
    re.DrawQuadPic(vVerts, vST, tex);
}

// ea: 0x0068C8D0
void trap_R_TrackStatistics(trStatistics_t* stats)
{
    re.TrackStatistics(stats);
}

// ea: 0x0068C8E0
int trap_R_PickShader(const math::Position3& org, const math::Position3& dir,
                      char* pszName,
                      char* pszSurfaceFlags, char* pszContents, int iMaxChars)
{
    return re.PickShader(org.v.m128_f32, dir.v.m128_f32, pszName,
                         pszSurfaceFlags, pszContents, iMaxChars);
}

// ea: 0x006986C0
int trap_R_RegisterShader(const char* name, int imagetype)
{
    CG_DrawInformation();
    return re.RegisterShader(name, imagetype);
}
