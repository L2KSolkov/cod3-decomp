// ============================================================================
// tr_dobj2.cpp - render.o DObj/free-list/effect/console helpers
// Types and bodies verified against IDA (codmp_xboxr.xbe).
// ============================================================================

#include "core/math_types.h"
#include "core/tlFixedString.h"
#include "aeps/apsEffect.h"
#include "game/logic/g_local.h"
#include "ngl/ngl_lighting.h"
#include "ngl/ngl_scene.h"
#include "ngl/ngl_dx_core.h"
#include "ngl/ngl_dx_quad.h"
#include "render/cdGlassShader.h"

#include <string.h>
#include <intrin.h>

extern void tlPrintf(const char* fmt, ...);  // core.o

// ============================================================================
// SizedHandle / HandleDb (ae/core/HandleDb.h; render.o COMDAT cluster
// 0x6EA1C0-0x6EF070)
// ============================================================================
template <int INDEX_BITS, int KEY_BITS>
class SizedHandle : public Handle {
public:
    SizedHandle() {}
    SizedHandle(unsigned int index, int key);
};

template <int INDEX_BITS, int KEY_BITS>
SizedHandle<INDEX_BITS, KEY_BITS>::SizedHandle(unsigned int index, int key)
{
    mVal = index | (key << INDEX_BITS);
}

template <typename T, int MAX, typename H>
class HandleDb {
public:
    struct DbElement {
        T* mObject;  // +0x00
        int mKey;    // +0x04
        void SetObject(T* obj);        // ?SetObject@DbElement@?$HandleDb@...@@QAEXPAVDObj@@@Z
        int GetKey() const;            // ?GetKey@DbElement@...@@QBEHXZ
        void Release();                // ?Release@DbElement@...@@QAEXXZ
        T* GetObject() const;          // ?GetObject@DbElement@...@@QBEPAVDObj@@XZ
    };

    unsigned int mFreeIndices[(MAX + 31) / 32];  // +0x00 (BitSet<MAX>)
    DbElement mElements[MAX];                    // +0xA8
    void (__cdecl* mDebugCallback)(int, T*);     // +0x1918

    H AllocateHandle();                          // ?AllocateHandle@?$HandleDb@VDObj@@...@@QAE?AV?$SizedHandle@$0M@$0BE@@@XZ
    void BindObjectToHandle(Handle handle, T* obj);  // ?BindObjectToHandle@...@@QAEXVHandle@@PAVDObj@@@Z
    T* DereferenceHandle(Handle h) const;        // ?DereferenceHandle@...@@QBEPAVDObj@@VHandle@@@Z
    void ReleaseHandle(Handle h);                // ?ReleaseHandle@...@@QAEXVHandle@@@Z
    void Dump();                                 // ?Dump@...@@QAEXXZ
};

template <typename T, int MAX, typename H>
void HandleDb<T, MAX, H>::DbElement::SetObject(T* obj)
{
    mObject = obj;
}

template <typename T, int MAX, typename H>
int HandleDb<T, MAX, H>::DbElement::GetKey() const
{
    return mKey;
}

template <typename T, int MAX, typename H>
void HandleDb<T, MAX, H>::DbElement::Release()
{
    ++mKey;
    mObject = nullptr;
}

template <typename T, int MAX, typename H>
T* HandleDb<T, MAX, H>::DbElement::GetObject() const
{
    return mObject;
}

template <typename T, int MAX, typename H>
void HandleDb<T, MAX, H>::BindObjectToHandle(Handle handle, T* obj)
{
    unsigned int v3 = handle.mVal & 0xFFF;
    if (v3 < (unsigned int)MAX)
    {
        if (mElements[v3].mKey != (int)(handle.mVal >> 12))
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\HandleDb.h";
            AeAssert::gCurrentLine = 123;
            AeAssert::gCurrentExpr = "element.GetKey() == h.GetKey()";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("handle was not allocated for this object"))
                __debugbreak();
        }
        mElements[v3].mObject = obj;
    }
}

template <typename T, int MAX, typename H>
T* HandleDb<T, MAX, H>::DereferenceHandle(Handle h) const
{
    unsigned int v2 = h.mVal & 0xFFF;
    if (v2 < (unsigned int)MAX
        && h.mVal >> 12 == (unsigned int)mElements[v2].mKey)
    {
        return mElements[v2].mObject;
    }
    return nullptr;
}

template <typename T, int MAX, typename H>
void HandleDb<T, MAX, H>::ReleaseHandle(Handle h)
{
    if (h.mVal != 0)
    {
        unsigned int v3 = h.mVal & 0xFFF;
        if (v3 >= (unsigned int)MAX)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\HandleDb.h";
            AeAssert::gCurrentLine = 175;
            AeAssert::gCurrentExpr = nullptr;
            if (!AeAssert::IsIgnored()
                && AeAssert::Warning("freeing invalid handle"))
                __debugbreak();
        }
        else if (mElements[v3].mKey == (int)(h.mVal >> 12))
        {
            mFreeIndices[v3 >> 5] |= 1u << (v3 & 0x1F);  // BitSet::Add
            mElements[v3].Release();
        }
        else
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\HandleDb.h";
            AeAssert::gCurrentLine = 176;
            AeAssert::gCurrentExpr = nullptr;
            if (!AeAssert::IsIgnored()
                && AeAssert::Warning("freeing invalid handle"))
                __debugbreak();
        }
    }
}

template <typename T, int MAX, typename H>
void HandleDb<T, MAX, H>::Dump()
{
    if (mDebugCallback != nullptr)
    {
        tlPrintf("handle db contents:\n");
        unsigned int allocated[(MAX + 31) / 32];
        for (int i = 0; i < (MAX + 31) / 32; ++i)
            allocated[i] = ~mFreeIndices[i];
        int word_idx = 0;
        unsigned int cur_word = allocated[0];
        int cur_val = -1;
        if (cur_word == 0)
        {
            do
            {
                if (word_idx >= (MAX + 31) / 32 - 1)
                    break;
                ++word_idx;
                cur_word = allocated[word_idx];
            } while (cur_word == 0);
        }
        if (cur_word != 0)
        {
            unsigned long v6;
            _BitScanForward(&v6, cur_word);
            cur_val = (int)v6 + 32 * word_idx;
            cur_word &= ~(1u << v6);
        }
        else
        {
            cur_val = -1;
            word_idx = -1;
        }
        while (!(cur_val == -1 && word_idx == -1))
        {
            mDebugCallback(cur_val, mElements[cur_val].mObject);
            if (cur_word != 0)
            {
                unsigned long v6;
                _BitScanForward(&v6, cur_word);
                cur_val = (int)v6 + 32 * word_idx;
                cur_word &= ~(1u << v6);
            }
            else
            {
                do
                {
                    if (word_idx >= (MAX + 31) / 32 - 1)
                        break;
                    ++word_idx;
                    cur_word = allocated[word_idx];
                } while (cur_word == 0);
                if (cur_word != 0)
                {
                    unsigned long v6;
                    _BitScanForward(&v6, cur_word);
                    cur_val = (int)v6 + 32 * word_idx;
                    cur_word &= ~(1u << v6);
                }
                else
                {
                    cur_val = -1;
                    word_idx = -1;
                }
            }
        }
    }
    AeAssert::gCurrentAuthor = AeAssert::COD3;
    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\HandleDb.h";
    AeAssert::gCurrentLine = 193;
    AeAssert::gCurrentExpr = nullptr;
    if (AeAssert::Error("out of handles! - Tell MikeA (MAX_GENTITIES)"))
        __debugbreak();
}

template <typename T, int MAX, typename H>
H HandleDb<T, MAX, H>::AllocateHandle()
{
    int word_idx = 0;
    unsigned int cur_word = mFreeIndices[0];
    int cur_val = -1;
    if (cur_word == 0)
    {
        do
        {
            if (word_idx >= (MAX + 31) / 32 - 1)
                break;
            ++word_idx;
            cur_word = mFreeIndices[word_idx];
        } while (cur_word == 0);
    }
    if (cur_word != 0)
    {
        unsigned long v6;
        _BitScanForward(&v6, cur_word);
        cur_val = (int)v6 + 32 * word_idx;
        cur_word &= ~(1u << v6);
    }
    else
    {
        cur_val = -1;
        word_idx = -1;
    }
    int m_cur_val = cur_val;
    if ((unsigned int)m_cur_val >= (unsigned int)MAX)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\HandleDb.h";
        AeAssert::gCurrentLine = 98;
        AeAssert::gCurrentExpr = "nextIndex >= 0 && nextIndex < _MaxEltements";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("index out of bounds!!! ILLEGAL array access!"))
            __debugbreak();
    }
    mFreeIndices[m_cur_val >> 5] &= ~(1u << (m_cur_val & 0x1F));  // BitSet::Rmv
    if (m_cur_val == -1)
        Dump();
    return H((unsigned int)m_cur_val, mElements[m_cur_val].mKey);
}

template class HandleDb<DObj, 1344, SizedHandle<12, 20>>;

class DObjHandleDb : public HandleDb<DObj, 1344, SizedHandle<12, 20>> {
private:
    static DObjHandleDb sInst;  // ?sInst@DObjHandleDb@@0V1@A @ 0x12C2268 (g.o)
    friend DObjHandleDb* DObjHandleDb_SInst();
public:
    DObjHandleDb();  // ??0DObjHandleDb@@QAE@XZ (g.o 0x4B3D40)
    void Init();  // ?Init@DObjHandleDb@@QAEXXZ (g.o 0x448B80)
};
DObjHandleDb::DObjHandleDb() : HandleDb<DObj, 1344, SizedHandle<12, 20>>()
{
}
void DObjHandleDb::Init()
{
}
DObjHandleDb DObjHandleDb::sInst;
DObjHandleDb* DObjHandleDb_SInst()
{
    return &DObjHandleDb::sInst;
}

// ea: 0x006D9920
DObj::DObj(TPakId pakId)
{
    for (int i = 0; i < 8; ++i)
    {
        models[i].mPakId = (unsigned int)PAK_ID_INVALID;
        models[i].mValue = nullptr;
    }
    mPakId = pakId;
    mPhysData.mPakId = (unsigned int)PAK_ID_INVALID;
    mPhysData.mValue = nullptr;
    mEntity = nullptr;
    mHandle = 0;
    mLODOverride = -1;
    mLODAnim = -1;
    mLOD = 0;
    mFlags = 0;
    Handle handle;
    handle.mVal = DObjHandleDb_SInst()->AllocateHandle().mVal;
    mHandle = handle.mVal;
    DObjHandleDb_SInst()->BindObjectToHandle(handle, this);
    for (int i = 0; i < 8; ++i)
    {
        tree[i] = nullptr;
        mPose[i] = nullptr;
        animPlayers[i] = nullptr;
    }
}

// ============================================================================
// DObjFree - ea: 0x006C47A0
// ============================================================================
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
extern nglScene* nglListBeginScene(nglSceneParamType ParamSource);
void nglSetClearFlags(unsigned int ClearFlags);
void nglSetFBWriteMask(unsigned int WriteMask);
void R_SetWindowQuadRect(nglQuad& q);  // tr_fx.cpp

extern unsigned int dword_CB8600;  // @ 0xCB8600

void R_ShowAlphaChannel()
{
    nglQuad q;
    nglListBeginScene(NGLSCENE_PARENT);
    nglSetClearFlags(0);
    nglSetFBWriteMask(0x10101u);
    nglInitQuad(&q);
    R_SetWindowQuadRect(q);
    nglListAddQuad(&q);
    nglListEndScene();
    nglListBeginScene(NGLSCENE_PARENT);
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
