// ============================================================================
// tr_dobj2.cpp - render.o DObj/free-list/effect/console helpers
// Types and bodies verified against IDA (codmp_xboxr.xbe).
// ============================================================================

#include "core/math_types.h"
#include "core/tlFixedString.h"
#include "aeps/apsEffect.h"
#include "game/logic/g_local.h"
#include "ngl/ngl_lighting.h"
#include "ngl/ngl_dx_core.h"
#include "ngl/ngl_dx_quad.h"
#include "render/cdGlassShader.h"

#include <string.h>

// ============================================================================
// DObjFree - ea: 0x006C47A0
// ============================================================================
struct DSkel {
    int animPartBits[4];       // +0x00
    int controlPartBits[4];    // +0x10
    int skelPartBits[4];       // +0x20
    uint8_t mat[64];           // +0x30 (DObjSkelMat[1])
};
struct DSkel4 {
    int animPartBits[4];       // +0x00
    int controlPartBits[4];    // +0x10
    int skelPartBits[4];       // +0x20
    uint8_t mat[256];          // +0x30 (DObjSkelMat[4])
};
struct DSkelMax {
    int animPartBits[4];       // +0x00
    int controlPartBits[4];    // +0x10
    int skelPartBits[4];       // +0x20
    uint8_t mat[5632];         // +0x30 (DObjSkelMat[88])
};

void XAnimClearTree(XAnimTree* tree);  // ?XAnimClearTree@@YAXPAVXAnimTree@@@Z
void XAnimFreeTree(XAnimTree* tree);   // ?XAnimFreeTree@@YAXPAVXAnimTree@@@Z

void DObjFree(DObj* obj, int bClearTree)
{
    cFreeList<DSkel>* gDSkelFreeList = (cFreeList<DSkel>*)(void*)&::gDSkelFreeList;
    cFreeList<DSkel4>* gDSkel4FreeList = (cFreeList<DSkel4>*)(void*)&::gDSkel4FreeList;
    cFreeList<DSkelMax>* gDSkelMaxFreeList = (cFreeList<DSkelMax>*)(void*)&::gDSkelMaxFreeList;

    if (obj->skel == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\DObj.cpp";
        AeAssert::gCurrentLine = 1017;
        AeAssert::gCurrentExpr = "obj->skel";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    unsigned char numBones = obj->numBones;
    if (numBones == 1)
    {
        DSkel* skel = (DSkel*)obj->skel;
        if (skel != nullptr)
        {
            ++gDSkelFreeList->mFree;
            --gDSkelFreeList->mUsed;
            skel->animPartBits[0] = gDSkelFreeList->mpFree->animPartBits[0];
            gDSkelFreeList->mpFree = skel;
        }
    }
    else if (numBones <= 4u)
    {
        DSkel4* skel = (DSkel4*)obj->skel;
        if (skel != nullptr)
        {
            --gDSkel4FreeList->mUsed;
            ++gDSkel4FreeList->mFree;
            skel->animPartBits[0] = gDSkel4FreeList->mpFree->animPartBits[0];
            gDSkel4FreeList->mpFree = skel;
        }
    }
    else if (obj->skel != nullptr)
    {
        DSkelMax* skel = (DSkelMax*)obj->skel;
        ++gDSkelMaxFreeList->mFree;
        --gDSkelMaxFreeList->mUsed;
        skel->animPartBits[0] = gDSkelMaxFreeList->mpFree->animPartBits[0];
        gDSkelMaxFreeList->mpFree = skel;
    }

    int i = 0;
    if (obj->numModels != 0)
    {
        do
        {
            XAnimTree* v7 = (XAnimTree*)obj->tree[i];
            if (v7 != nullptr)
            {
                if (*(void**)((char*)v7 + 8) == nullptr)  // tree->anims
                {
                    AeAssert::gCurrentAuthor = AeAssert::COD3;
                    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\DObj.cpp";
                    AeAssert::gCurrentLine = 1043;
                    AeAssert::gCurrentExpr = "tree->anims";
                    if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
                        __debugbreak();
                }
                if (bClearTree != 0)
                {
                    XAnimClearTree(v7);
                    XAnimFreeTree(v7);
                }
                obj->tree[i] = nullptr;
            }
            ++i;
        } while (i < obj->numModels);
    }
    if (obj->duplicateParts != 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\DObj.cpp";
        AeAssert::gCurrentLine = 1059;
        AeAssert::gCurrentExpr = "0";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Is this used? (CD)"))
            __debugbreak();
        obj->duplicateParts = 0;
    }
}

// ============================================================================
// XModelPartsManager::AssignHashName - ea: 0x006C4700
// ============================================================================
class XModelPartsManager {
public:
    void AssignHashName(XModelParts* xmp);  // ?AssignHashName@XModelPartsManager@@QAEXPAVXModelParts@@@Z
};

void XModelPartsManager::AssignHashName(XModelParts* xmp)
{
    if (xmp != nullptr)
    {
        if (xmp->mHierarchy.mList[0].mNameHash == 0)
        {
            for (unsigned int i = 0; i < xmp->mHierarchy.mSize; ++i)
            {
                xmp->mHierarchy.mList[i].mNameHash =
                    (unsigned int)BrocSys::RegisterHashString(
                        xmp->mHierarchy.mList[i].mName.mStr);
            }
        }
    }
}

// ============================================================================
// UpdateNanoDynamicSystem / FX_ParticleEffectSampleLightGrid
// ============================================================================
void UpdateNanoDynamicSystem(apsEffect* effect)
{
    (void)effect;
}

nglLightContext* FX_ParticleEffectSampleLightGrid(apsEffect* effect)
{
    (void)effect;
    nglLightContext* LightContext = nglCreateLightContext();
    nglSetAmbientLight(0.40000001f, 0.40000001f, 0.40000001f);
    return LightContext;
}

// ============================================================================
// RE_Shutdown - ea: 0x006C6480
// ============================================================================
struct refimport_t {
    void (*Printf)(int, const char*, ...);       // +0x00
    uint8_t _pad[0x30 - 0x04];
    void (*Cmd_RemoveCommand)(const char*);      // +0x30
};
extern refimport_t ri;  // ?ri@@3Urefimport_t@@A @ 0xF741E8
extern nglTexture* gProjShadowTex;  // cdGlassShader.cpp
void nglWaitForRendering();
void nglDestroyTexture(nglTexture* Tex);

// trGlobals_t view (registered +0x00, debug +0x314)
struct trGlobals_t {
    int registered;             // +0x00
    uint8_t _pad[0x314 - 0x04];
    uint8_t debug[0x80];        // +0x314 (trDebug_t)
};
extern trGlobals_t tr;          // ?tr@@3UtrGlobals_t@@A @ 0xF74DD0

void RE_Shutdown(int destroyWindow)
{
    ri.Printf(0, "RE_Shutdown( %i )\n", destroyWindow);
    ri.Cmd_RemoveCommand("modellist");
    ri.Cmd_RemoveCommand("screenshotHigh");
    ri.Cmd_RemoveCommand("screenshot");
    ri.Cmd_RemoveCommand("imagelist");
    ri.Cmd_RemoveCommand("shaderlist");
    ri.Cmd_RemoveCommand("skinlist");
    ri.Cmd_RemoveCommand("gfxinfo");
    ri.Cmd_RemoveCommand("modelist");
    ri.Cmd_RemoveCommand("shaderstate");
    ri.Cmd_RemoveCommand("taginfo");
    ri.Cmd_RemoveCommand("cropimages");
    ri.Cmd_RemoveCommand("r_meminfo");
    ri.Cmd_RemoveCommand("r_vc_stats");
    ri.Cmd_RemoveCommand("r_smc_stats");
    ri.Cmd_RemoveCommand("r_smc_flush");
    ri.Cmd_RemoveCommand("r_loadsun");
    ri.Cmd_RemoveCommand("r_savesun");
    ri.Cmd_RemoveCommand("r_sunhelp");
    ri.Cmd_RemoveCommand("r_vbo_refresh");
    if (gProjShadowTex != nullptr)
    {
        nglWaitForRendering();
        nglDestroyTexture(gProjShadowTex);
        gProjShadowTex = nullptr;
    }
    memset(&tr.debug, 0, sizeof(tr.debug));
    tr.registered = 0;
}

// ============================================================================
// R_ShowAlphaChannel - ea: 0x006C67F0
// ============================================================================
struct nglScene;
extern nglScene* nglListBeginScene(int ParamSource);
void nglSetClearFlags(unsigned int ClearFlags);
void nglSetFBWriteMask(unsigned int WriteMask);
void R_SetWindowQuadRect(nglQuad& q);  // tr_fx.cpp

extern unsigned int dword_CB8600;  // @ 0xCB8600

void R_ShowAlphaChannel()
{
    nglQuad q;
    nglListBeginScene(1);  // NGLSCENE_PARENT
    nglSetClearFlags(0);
    nglSetFBWriteMask(0x10101u);
    nglInitQuad(&q);
    R_SetWindowQuadRect(q);
    nglListAddQuad(&q);
    nglListEndScene();
    nglListBeginScene(1);  // NGLSCENE_PARENT
    nglSetClearFlags(0);
    nglSetFBWriteMask(0x10101u);
    nglInitQuad(&q);
    R_SetWindowQuadRect(q);
    float y2 = (float)nglGetScreenHeight();
    float x2 = (float)nglGetScreenWidth();
    nglSetQuadRect(&q, 0.0f, 0.0f, x2, y2);
    nglSetQuadBlend(&q, dword_CB8600);
    nglListAddQuad(&q);
    nglListEndScene();
}
