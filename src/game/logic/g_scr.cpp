// ============================================================================
// g_scr.cpp - script integration wrappers (g.o: g_scr_main.cpp / g_spawn.cpp)
// ============================================================================

#include "game/logic/g_local.h"

#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "core/PoolAllocator.h"
#include "core/tlFixedString.h"
#include "game/nextgen/nextgen.h"

extern PoolAllocator* gCommonPoolAllocator;  // ?gCommonPoolAllocator@@3PAVPoolAllocator@@A (core.o)
extern bool g_indoor;                              // ?g_indoor@@3_NA (core.o)
extern float CG_GetNorthDirection();               // ?CG_GetNorthDirection@@YAMXZ (cg.o)
extern void G_FlushCorpses();                      // ?G_FlushCorpses@@YAXXZ (mp_actors.o)
extern void FX_SetRainDrops(bool on);              // ?FX_SetRainDrops@@YAX_N@Z (render.o)

namespace ShaderCommon {
extern bool gGlowGodRays;  // ?gGlowGodRays@ShaderCommon@@3_NA
extern int  gGlowPasses;   // ?gGlowPasses@ShaderCommon@@3HA
extern float gGlowIntensity;  // ?gGlowIntensity@ShaderCommon@@3MA
extern float gGlowExpansion;  // ?gGlowExpansion@ShaderCommon@@3MA
}

namespace View {
bool IsSplitScreen();  // ?IsSplitScreen@View@@YA_NXZ (cg.o)
}

namespace BrocHelper {
int m_treeCount;  // ?m_treeCount@BrocHelper@@3HA (scr.o @ 0x1329E78)
void SetLoadedTrees(int num);  // ?SetLoadedTrees@BrocHelper@@YAXH@Z
int  GetLoadedTrees();         // ?GetLoadedTrees@BrocHelper@@YAHXZ
}

// ea: 0x005BE020
void BrocHelper::SetLoadedTrees(int num)
{
    BrocHelper::m_treeCount = num;
}

// ea: 0x005BE030
int BrocHelper::GetLoadedTrees()
{
    return BrocHelper::m_treeCount;
}

extern void __fastcall Sentient_SetGoalRadius(sentient_s* pSelf, float fRadius);  // ?Sentient_SetGoalRadius@@YIXPAUsentient_s@@M@Z
extern void __fastcall Sentient_SetGoalAngleTolerance(sentient_s* pSelf, float fTolerance);  // ?Sentient_SetGoalAngleTolerance@@YIXPAUsentient_s@@M@Z
extern bool g_controllerConnectedErrorShown[4];  // ?g_controllerConnectedErrorShown@@3PA_NA (game2.o)
extern bool gNANO_Animate;                       // ?gNANO_Animate@@3_NA (g.o)

namespace AeAssert {
extern bool gAssertsEnabled;  // ?gAssertsEnabled@AeAssert@@3_NA (core_xboxr)
}

extern void EffectEventPlayQueuedEffect(Handle effect);  // ?EffectEventPlayQueuedEffect@@YAXVHandle@@@Z (game.o)
extern bool EffectEventIsPlaying(Handle effect);         // ?EffectEventIsPlaying@@YA_NVHandle@@@Z (game.o)
extern void EffectEventStopEmitting(Handle effect);      // ?EffectEventStopEmitting@@YAXVHandle@@@Z (game.o)
extern void EffectEventFF(Handle effect, float deltaT);  // ?EffectEventFF@@YAXVHandle@@M@Z (game.o)

class AnimNotifyTask {
public:
    static void RegisterFunc(const char* pKey, void (__cdecl* cbFunc)(Broc::entity));
    // ?RegisterFunc@AnimNotifyTask@@SAXPBDP6AXVentity@Broc@@@Z@Z
};

extern const char* nodeStringTable[0x13];  // ?nodeStringTable@@3PAPBDA (mp_actors.o @ 0xE37A20)
extern float gProjShadowAlpha;             // ?gProjShadowAlpha@@3MA (render.o)

struct IGOCompassWidget {
    void SetHideCompassStar(int active, int index);  // ?SetHideCompassStar@IGOCompassWidget@@QAEXHH@Z
    void SetHideUpdatedText(int active, int index);  // ?SetHideUpdatedText@IGOCompassWidget@@QAEXHH@Z
};

// MusicMgr view (game.o; class lives in g_entity_misc.cpp)
class MusicMgr {
public:
    static MusicMgr* sInst;  // ?sInst@MusicMgr@@2PAV1@A
    void Stop(float fadeOutTime);         // ?Stop@MusicMgr@@QAEXM@Z
    void StopIndoor(float fadeOutTime);   // ?StopIndoor@MusicMgr@@QAEXM@Z
};

// CurveManager view (game.o; class lives in g_cmd.cpp)
typedef float (__cdecl* CurveEvalFunc)(unsigned int, unsigned int, unsigned int,
                                       float, float, unsigned int);
class CurveManager {
public:
    static CurveManager* sInst;  // ?sInst@CurveManager@@2PAV1@A
    void AddKeyFunc(unsigned int type, CurveEvalFunc function);        // ?AddKeyFunc@CurveManager@@QAEXIP6AMIIIMMI@Z@Z
    void AddConditionFunc(unsigned int type, CurveEvalFunc function);  // ?AddConditionFunc@CurveManager@@QAEXIP6AMIIIMMI@Z@Z
};

// ?gpBrocAPI@@3PAUBrocAPI@@A (scr.o data @ 0xF3ABDC, BSS)
BrocAPI* gpBrocAPI = NULL;

// ?currentVM@@3PAUvm_s@@A (scr.o data @ 0xF3AC04)
vm_s* currentVM = NULL;

// ea: 0x0077D020 (mp_actors.o)
void __fastcall Sentient_GetOrigin(sentient_s* pSelf, float* const vOriginOut)
{
    if (pSelf == NULL)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sentient.cpp";
        AeAssert::gCurrentLine = 246;
        AeAssert::gCurrentExpr = "pSelf";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (pSelf->pEnt == NULL)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sentient.cpp";
        AeAssert::gCurrentLine = 247;
        AeAssert::gCurrentExpr = "pSelf->pEnt";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (pSelf->pEnt->actor == NULL && pSelf->pEnt->client == NULL)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sentient.cpp";
        AeAssert::gCurrentLine = 248;
        AeAssert::gCurrentExpr = "pSelf->pEnt->actor || pSelf->pEnt->client";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (vOriginOut == NULL)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sentient.cpp";
        AeAssert::gCurrentLine = 249;
        AeAssert::gCurrentExpr = "vOriginOut";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    vOriginOut[0] = pSelf->pEnt->r.currentOrigin.v.m128_f32[0];
    vOriginOut[1] = pSelf->pEnt->r.currentOrigin.v.m128_f32[1];
    vOriginOut[2] = pSelf->pEnt->r.currentOrigin.v.m128_f32[2];
}

// ea: 0x005C1DE0
int VM_Call(vm_s* vm, int callnum, ...)
{
    if (vm == NULL)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\vm.cpp";
        AeAssert::gCurrentLine = 320;
        AeAssert::gCurrentExpr = "vm";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    vm_s* oldVM = currentVM;
    int (*entryPoint)(int, ...) = vm->entryPoint;
    currentVM = vm;
    if (entryPoint == NULL)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\vm.cpp";
        AeAssert::gCurrentLine = 326;
        AeAssert::gCurrentExpr = "vm->entryPoint";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    int args[16];
    int* p_callnum = &callnum;
    unsigned int v4 = 0;
    do
    {
        args[v4++] = p_callnum[1];
        ++p_callnum;
    } while (v4 < 0x10);
    int result = entryPoint(callnum, args[0], args[1], args[2], args[3],
                            args[4], args[5], args[6], args[7], args[8],
                            args[9], args[10], args[11], args[12], args[13],
                            args[14], args[15]);
    currentVM = oldVM;
    return result;
}

// ea: 0x005C1D30
void VM_Free(vm_s* vm)
{
    if (vm->dllHandle == NULL)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\vm.cpp";
        AeAssert::gCurrentLine = 255;
        AeAssert::gCurrentExpr = "vm->dllHandle";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    memset(vm, 0, 0x8C);
    currentVM = NULL;
}

// ea: 0x005BE2B0 (scr.o)
void UpdateEntityHash(Entity* ent)
{
    ent->mClassNameHash = HashString(ent->mClassName);
    ent->mGroupNameHash = HashString::CalcHash(
        ent->mGroupName.mBlock != nullptr ? (const char*)(ent->mGroupName.mBlock + 1)
                                          : (const char*)&"");
    ent->targetnameHash = HashString::CalcHash(
        ent->targetname.mBlock != nullptr ? (const char*)(ent->targetname.mBlock + 1)
                                          : (const char*)&"");
    ent->mTargetHash = HashString::CalcHash(
        ent->mTarget.mBlock != nullptr ? (const char*)(ent->mTarget.mBlock + 1)
                                       : (const char*)&"");
    ent->mScriptNoteworthyHash = HashString::CalcHash(
        ent->mScriptNoteworthy.mBlock != nullptr
            ? (const char*)(ent->mScriptNoteworthy.mBlock + 1)
            : (const char*)&"");
    ent->mAnimNameHash = HashString::CalcHash(
        ent->mAnimName.mBlock != nullptr ? (const char*)(ent->mAnimName.mBlock + 1)
                                         : (const char*)&"");
}

// ============================================================================
// AnimBankManager / AnimBank
// ============================================================================
struct AnimBank {
    InplaceVector<XAnimEntry> anims;  // +0x00
};
class AnimBankManager {
public:
    static AnimBankManager* sInst;  // 0xF25A34
    AnimBank* GetBank(TPakId pakId);
};
AnimBankManager* AnimBankManager::sInst;  // ?sInst@AnimBankManager@@2PAV1@A (anim.o @ 0x1314F34)

// ============================================================================
// saveField_t - script save fields (8 bytes) - verified against IDA
// ============================================================================
enum saveFieldtype_t {
    SVF_NONE = 0,
    SVF_BROCSTR = 0x10,
};
struct saveField_t {
    int               ofs;   // +0x00
    saveFieldtype_t   type;  // +0x04
};
static_assert(sizeof(saveField_t) == 0x8, "saveField_t size mismatch");

saveField_t sentientFields[128];  // ?sentientFields@@3PAUsaveField_t@@A (mp_actors.o)
saveField_t actorFields[128];     // ?actorFields@@3PAUsaveField_t@@A (mp_actors.o)
extern int g_xanim_num;                 // 0xF3A778
unsigned char sConstsLoaded;     // 0xEF357B
const char* gHashStringTblTxt[173];  // ?gHashStringTblTxt (scr.o @ 0xDD6488)
extern void Scr_FreePrecachedAnimTrees();
extern void GScr_LoadScriptsAndAnimsForEntities();
extern void Scr_PrecacheAnimTrees(void* (*Alloc)(int), bool restart);
extern void* Hunk_AllocXAnimCreate(int size);
extern AnimTree* Scr_GetAnimTreeByName(const char* treename);
#ifndef PAK_ID_MIN
#define PAK_ID_MIN ((TPakId)0)
#endif

// ea: 0x00449350
void GScr_GetStartOrigin()
{
    ;
}

// ea: 0x00449360
void GScr_GetStartAngles()
{
    ;
}

// ea: 0x00449370
void GScr_GetCycleOriginOffset()
{
    ;
}

// ea: 0x0044CAE0
void Scr_FreeFields(const saveField_t* fields, unsigned char* base)
{
    if (fields->type != SVF_NONE)
    {
        const saveFieldtype_t* p_type = &fields->type;
        int v6;
        do
        {
            if (*p_type == SVF_BROCSTR)
            {
                int v3 = *(p_type - 1);
                Broc::string::Block* v4 = *(Broc::string::Block**)&base[v3];
                if (v4 != nullptr)
                {
                    v4->DecrementCount();
                    *(Broc::string::Block**)&base[v3] = nullptr;
                }
            }
            v6 = *(p_type + 2);
            p_type += 2;
        } while (v6 != 0);
    }
}

// ea: 0x0044CB30
void Scr_FreeSentientFields(sentient_s* pSentient)
{
    Scr_FreeFields(sentientFields, (unsigned char*)pSentient);
}

// ea: 0x0044CB50
void Scr_FreeActorFields(actor_s* pActor)
{
    Scr_FreeFields(actorFields, (unsigned char*)pActor);
}

// ea: 0x0044CB70
HashString GScr_AllocHash(const char* s)
{
    HashString result;
    result.mHash = HashString::CalcHash(s);
    BrocSys::RegisterHashString(s);
    return result;
}

// ea: 0x0044CBA0
void GScr_LoadConsts()
{
    if (!sConstsLoaded)
    {
        sConstsLoaded = 1;
        for (unsigned int i = 0; i < 173; ++i)
        {
            const char* v2 = gHashStringTblTxt[i];
            unsigned int v3 = HashString::CalcHash(v2);
            BrocSys::RegisterHashString(v2);
            (&hash_const.active.mHash)[i] = v3;
            (&str_const.active)[i] = v2;
        }
    }
}

// ea: 0x0044CC00
int GScr_LoadScriptAndLabel(const char* /*scriptName*/, const char* /*labelName*/, int /*mode*/)
{
    AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_main.cpp";
    AeAssert::gCurrentLine = 82;
    AeAssert::gCurrentExpr = "0";
    if (!AeAssert::IsIgnored() && AeAssert::Assert("ma dead code"))
        __debugbreak();
    return 0;
}

// ea: 0x0044CD20
void GScr_LoadSingleAnimScript(scr_animscript_t* /*pScript*/, const char* /*szScript*/)
{
    AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_main.cpp";
    AeAssert::gCurrentLine = 196;
    AeAssert::gCurrentExpr = "0";
    if (!AeAssert::IsIgnored() && AeAssert::Assert("Dead Code CD"))
        __debugbreak();
}

// ea: 0x0044CD70
void GScr_FixupBroAnimScriptHooks()
{
    ;
}

// ea: 0x0044CD90
void GScr_AddFieldsForVehicleNode()
{
    ;
}

// ea: 0x0044CDD0
void GScr_FreeScripts()
{
    Scr_FreePrecachedAnimTrees();
}

// ea: 0x00450290
void GScr_AddFieldsForEntity()
{
    AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_spawn.cpp";
    AeAssert::gCurrentLine = 617;
    AeAssert::gCurrentExpr = "0";
    if (!AeAssert::IsIgnored())
    {
        if (AeAssert::Assert("ma dead code"))
            __debugbreak();
    }
}

// ea: 0x004502E0
void GScr_AddFieldsForRadiant()
{
    AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_spawn.cpp";
    AeAssert::gCurrentLine = 661;
    AeAssert::gCurrentExpr = "0";
    if (!AeAssert::IsIgnored())
    {
        if (AeAssert::Assert("ma dead code"))
            __debugbreak();
    }
}

// ea: 0x00450330
void Scr_SetGenericField(unsigned char* /*base*/, fieldtype_t /*type*/, int /*ofs*/)
{
    AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_spawn.cpp";
    AeAssert::gCurrentLine = 675;
    AeAssert::gCurrentExpr = "0";
    if (!AeAssert::IsIgnored() && AeAssert::Assert("ma dead code"))
        __debugbreak();
}

// ea: 0x00450380
void Scr_SetObjectField(unsigned int /*entnum*/, int /*offset*/, int /*type*/)
{
    AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_spawn.cpp";
    AeAssert::gCurrentLine = 685;
    AeAssert::gCurrentExpr = "0";
    if (!AeAssert::IsIgnored() && AeAssert::Assert("DEAD CODE"))
        __debugbreak();
}

// ea: 0x004503D0
void Scr_GetEntityField(int /*entnum*/, int /*offset*/)
{
    AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_spawn.cpp";
    AeAssert::gCurrentLine = 695;
    AeAssert::gCurrentExpr = "0";
    if (!AeAssert::IsIgnored() && AeAssert::Assert("ma dead code"))
        __debugbreak();
}

// ea: 0x00450420
void Scr_GetObjectField(unsigned int /*entnum*/, int /*offset*/, int /*type*/)
{
    AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_spawn.cpp";
    AeAssert::gCurrentLine = 705;
    AeAssert::gCurrentExpr = "0";
    if (!AeAssert::IsIgnored() && AeAssert::Assert("ma dead code"))
        __debugbreak();
}

// ea: 0x00450470
void Scr_FreeEntity(Entity* /*ent*/)
{
    AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_spawn.cpp";
    AeAssert::gCurrentLine = 715;
    AeAssert::gCurrentExpr = "0";
    if (!AeAssert::IsIgnored() && AeAssert::Assert("DEAD CODE"))
        __debugbreak();
}

// ea: 0x0045BF80
void GScr_LoadScripts(bool restart)
{
    gpBrocAPI->mBrocExports.mAnimInitialize();
    g_xanim_num = AnimBankManager::sInst->GetBank(PAK_ID_MIN)->anims.mSize;
    GScr_LoadScriptsAndAnimsForEntities();
    Scr_PrecacheAnimTrees(Hunk_AllocXAnimCreate, restart);
    AnimTree* AnimTreeByName = Scr_GetAnimTreeByName("generic_human");
    if (AnimTreeByName == nullptr)
        G_Error("Could not find animation tree '%s'", "generic_human");
    g_scr_data.generic_human_tree = AnimTreeByName;
}

// ea: 0x0045BFF0
XAnimTree* GScr_GetEntAnimTree(Entity* ent)
{
    XAnimTree* pAnimTree;
    XAnimTree* ActorAnimTree;
    if (ent->s.eType == 11)
    {
        ActorAnimTree = G_GetActorAnimTree(ent->actor);
        pAnimTree = ActorAnimTree;
    }
    else if (ent->s.eType == 13)
    {
        ActorAnimTree = G_GetActorCorpseAnimTree(ent);
        pAnimTree = ActorAnimTree;
    }
    else
    {
        pAnimTree = ent->pAnimTree;
    }
    if (pAnimTree == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_main.cpp";
        AeAssert::gCurrentLine = 700;
        AeAssert::gCurrentExpr = nullptr;
        if (!AeAssert::IsIgnored())
        {
            const char* classname = ent->mClassName.mBlock != nullptr
                                        ? (const char*)(ent->mClassName.mBlock + 1)
                                        : &defaultFileName[0];
            const char* v5 = ent->s.eType >= 0x12u
                                 ? "WARNING !! Entity Type Unknown WARNING !!!"
                                 : entityTypeNames[ent->s.eType];
            char* v6 = va("entity of type '%s', classname '%s', origin (%f, %f, %f) does not have an animation tree",
                          v5, classname,
                          ent->r.currentOrigin.v.m128_f32[0],
                          ent->r.currentOrigin.v.m128_f32[1],
                          ent->r.currentOrigin.v.m128_f32[2]);
            if (AeAssert::Warning(v6))
                __debugbreak();
        }
    }
    return pAnimTree;
}

// ea: 0x00470600
void Scr_NotifyFromEnt(Entity* ent, HashString hashValue, Entity* fromEnt)
{
    Entity* v3 = fromEnt;
    if (fromEnt != nullptr && ent != nullptr)
    {
        unsigned int v4 = ent->mHandle.mHandle.mVal & 0xFFF;
        Entity* mObject = nullptr;
        if (v4 < 0x540 && ent->mHandle.mHandle.mVal >> 12 == EntityHandleDb::sInst.mElements[v4].mKey)
            mObject = EntityHandleDb::sInst.mElements[v4].mObject;
        if (mObject != ent)
        {
            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_spawn.cpp";
            AeAssert::gCurrentLine = 751;
            AeAssert::gCurrentExpr = "*ent->GetHandle() == ent";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
                __debugbreak();
        }
        unsigned int fromHandle = v3->mHandle.mHandle.mVal;
        ent->Notify(hashValue, &fromHandle);
    }
}

// ea: 0x004706B0
void Scr_Notify(Entity* ent, HashString hashValue, unsigned int paramcount)
{
    if (ent == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_spawn.cpp";
        AeAssert::gCurrentLine = 764;
        AeAssert::gCurrentExpr = "ent";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    unsigned int v2 = ent->mHandle.mHandle.mVal & 0xFFF;
    Entity* mObject = nullptr;
    if (v2 < 0x540 && ent->mHandle.mHandle.mVal >> 12 == EntityHandleDb::sInst.mElements[v2].mKey)
        mObject = EntityHandleDb::sInst.mElements[v2].mObject;
    if (mObject != ent)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_spawn.cpp";
        AeAssert::gCurrentLine = 765;
        AeAssert::gCurrentExpr = "*ent->GetHandle() == ent";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    ent->Notify(hashValue);
}

// ============================================================================
// scr.o batch 1 - MemCount stubs + BrocSys wrappers (smallest first)
// ============================================================================

extern void tlPrint(const char* text);
extern bool gCE;               // ?gCE@@3_NA (pakmanager.cpp)
extern int dword_F6419C[4 * 1580];  // cg_draw.cpp (special recharge block)
extern int dword_F641A0[4 * 1580];
extern int dword_F641A4[4 * 1580];
extern void mem_heap_free(void* ptr);  // ?mem_heap_free@@YAXPAX@Z

namespace MPUIInterface {
void ExitGame();  // ?ExitGame@MPUIInterface@@SAXXZ (mp.o)
bool IsOnlineGame();  // mp.o
bool IsLANGame();     // mp.o
bool IsLocalGame();   // mp.o
}

// ae_heap wrapper view (streamer.o 0x684DD0; definition in pakmanager.cpp)
struct mem_heap;
struct ae_heap_wrapper {
    void* __vftable;   // +0x00
    mem_heap* mHeap;   // +0x04
    bool CheckFree(void* ptr);  // ?CheckFree@ae_heap_wrapper@@UAE_NPAX@Z
};

namespace MemCount {
// IDA types: enum MemCount::eGamePhase : int
enum eGamePhase : int {
    GAME_PHASE_FRONTEND = 0,
    GAME_PHASE_LOADING = 1,
    GAME_PHASE_INGAME = 2,
};
void Init();
void RenderTotals();
void ReportTotals();
bool IsRealLeak(const char* type, int start, int end);
void CheckForMapChangeLeaks();
void CheckForRoundtripLeaks();
void SetGamePhase(eGamePhase phase);
void SetSafeAlloc(bool val);
}  // namespace MemCount

// ea: 0x005BBC50 (retn stub)
void MemCount::Init()
{
}

// ea: 0x005BBC60 (retn stub)
void MemCount::RenderTotals()
{
}

// ea: 0x005BBC70 (retn stub)
void MemCount::ReportTotals()
{
}

// ea: 0x005BBC80 (mov al,1; ret)
bool MemCount::IsRealLeak(const char* /*type*/, int /*start*/, int /*end*/)
{
    return true;
}

// ea: 0x005BBC90 (retn stub)
void MemCount::CheckForMapChangeLeaks()
{
}

// ea: 0x005BBCA0 (retn stub)
void MemCount::CheckForRoundtripLeaks()
{
}

// ea: 0x005BBCB0 (retn stub)
void MemCount::SetGamePhase(MemCount::eGamePhase /*phase*/)
{
}

// ea: 0x005BBCC0 (retn stub)
void MemCount::SetSafeAlloc(bool /*val*/)
{
}

// ea: 0x005BBCD0
void ThreadPrintf(int bitMask, const char* Format, ...)
{
    char Work[512];
    va_list ap;
    va_start(ap, Format);
    if ((bitMask & Cvar_Get("g_scriptdebug", "0", 512)->integer) == bitMask)
    {
        vsprintf(Work, Format, ap);
        tlPrint(Work);
    }
    va_end(ap);
}

// ea: 0x005BC1D0 (empty stub)
void SetDepthOfField(bool, float, float, float, float)
{
}

// ea: 0x005BC1E0 (xor eax,eax; ret)
unsigned int CreateNanoGraph(char* /*id*/, float* const /*param1*/,
                             float* const /*param2*/)
{
    return 0;
}

namespace BrocSys {

bool allowOverLapping;  // ?allowOverLapping@BrocSys@@3_NA (scr.o @ 0x132A0F8)

// ea: 0x005BC840
float atoff(const char* s)
{
    return (float)atof(s);
}

// ea: 0x005BC9B0
float CVarGetFloat(const char* cvarName)
{
    return Cvar_VariableValue(cvarName);
}

// ea: 0x005BECB0
void SceneEffectEnable(unsigned int groupIdHash)
{
    SceneManager::sInst->EnableEffect(groupIdHash);
}

// ea: 0x005BECD0
void SceneEffectDisable(unsigned int groupIdHash)
{
    SceneManager::sInst->DisableEffect(groupIdHash);
}

// ea: 0x005BF420 (disasm: write vec->y to pNode+0x58)
void PathNode_SetAngles(PathNodes::PathNode* pNode, int /*offset*/,
                        Broc::vector* vec)
{
    *(float*)((char*)pNode + 0x58) = vec->y;
}

// ea: 0x005BF4C0 (disasm: nodeStringTable[pNode+0x28] -> Broc::string)
void PathNode_GetType(PathNodes::PathNode* pNode, int /*offset*/,
                      Broc::string* s)
{
    *s = nodeStringTable[*(int*)((char*)pNode + 0x28)];
}

// ea: 0x005BF560
void SentientScr_ConvertSentient(sentient_s* pSelf, int /*offset*/,
                                 Broc::entity* pEnt)
{
    if (pSelf->pEnt != nullptr)
        pEnt->___u0 = pSelf->pEnt->mHandle.mHandle.mVal;
}

// ea: 0x005BF820
void ObjectiveHideStar(unsigned int hideStar, unsigned int objectiveIndex)
{
    ((IGOCompassWidget*)g_femanager.IGO->compassWidget[0])
        ->SetHideCompassStar(hideStar, objectiveIndex);
}

// ea: 0x005BF840
void ObjectiveHideUpdatedText(unsigned int hideText,
                              unsigned int objectiveIndex)
{
    ((IGOCompassWidget*)g_femanager.IGO->compassWidget[0])
        ->SetHideUpdatedText(hideText, objectiveIndex);
}

// ea: 0x005BF890
void SetHUDType(hud_type type, int viewport)
{
    g_femanager.IGO->SetHUDType(type, viewport);
}

// ea: 0x005BF920
void SetTutorialTextAllPlayers(int hash)
{
    if (g_femanager.IGO != nullptr)
        g_femanager.IGO->SetTutorialText(hash, 0);
}

// ea: 0x005BFB40
void SetShadowIntensity(float i)
{
    gProjShadowAlpha = i;
}

// ea: 0x005BDB90 (thunk to CG_MotionBlur::End)
void StopCurGenMotionBlur()
{
    CG_MotionBlur::End();
}

// ea: 0x005BEAE0 (return -1 stub)
int GetNodeInProximity(const Broc::vector&, float, bool, unsigned int)
{
    return -1;
}

// ea: 0x005BEC70
void EffectEventPlayQueued(unsigned int effectId)
{
    EffectEventPlayQueuedEffect(Handle(effectId));
}

// ea: 0x005BEC80
bool EffectEventIsStillPlaying(unsigned int effectId)
{
    return EffectEventIsPlaying(Handle(effectId));
}

// ea: 0x005BEC90
void EffectEventStop(unsigned int effectId)
{
    EffectEventStopEmitting(Handle(effectId));
}

// ea: 0x005BECA0
void EffectEventFastForward(unsigned int effectId, float deltaT)
{
    EffectEventFF(Handle(effectId), deltaT);
}

// ea: 0x005BECF0
void RegisterAnimNotifyFunc(const char* pAnimKey, void (__cdecl* fn)(Broc::entity))
{
    AnimNotifyTask::RegisterFunc(pAnimKey, fn);
}

// ea: 0x005BED80
void DialogPlayAllowOverlapping(bool b)
{
    allowOverLapping = b;
}

// ea: 0x005BEDE0
int IsVehicleNodeDefined(unsigned int handle)
{
    return handle != (unsigned int)-1;
}

// ea: 0x005BEDF0 (empty stub)
void AnimScripted2(unsigned int, unsigned int, const Broc::vector&,
                   const Broc::vector&, unsigned int, const Broc::string&,
                   unsigned int, bool, float, float)
{
}

// ea: 0x005BEEA0 (empty stub)
void StartScriptedAnim(unsigned int, unsigned int, const Broc::vector&,
                       const Broc::vector&, unsigned int, const Broc::string&,
                       unsigned int)
{
}

// ea: 0x005BEF90 (empty stub)
void AddFakeFriendly(unsigned int, bool)
{
}

// ea: 0x005BEFA0 (empty stub)
void RemoveFakeFriendly(unsigned int)
{
}

// ea: 0x005BF0A0
bool IsLocalHost()
{
    return MultiplayerMgr::sInst->IsHost();
}

// ea: 0x005BF730 (empty stub)
void SaveCheckpoint()
{
}

// ea: 0x005BF740
void RestoreLastCheckpoint()
{
    CheckpointMgr::sInst->RestoreLastCheckpoint();
}

// ea: 0x005BF7F0 (empty stub)
void DronesStart(const char*, const char*, int, int, int, float, float,
                 bool, bool)
{
}

// ea: 0x005BF800 (empty stub)
void DronesStop(const char*)
{
}

// ea: 0x005BF810 (empty stub)
void DronesDelete(const char*)
{
}

// ea: 0x005BFB20 (empty stub)
void SetDroneScriptControl(unsigned int, bool)
{
}

// ea: 0x005BFB30 (return 0 stub)
unsigned int GetDrones(const Broc::vector&, float, unsigned int*,
                       unsigned int)
{
    return 0;
}

// ea: 0x005BFB80
void EnableAsserts()
{
    AeAssert::gAssertsEnabled = true;
}

// ea: 0x005BFBE0 (empty stub)
void SetZFog(float, float, float)
{
}

// ea: 0x005C0AF0
void ToggleNano(bool on)
{
    gNANO_Animate = on;
}

// ea: 0x005C0C30
void MPScript_ClearTeamScores()
{
    cgGlobal.teamScores[2] = 0;
    cgGlobal.teamScores[1] = 0;
}

// ea: 0x005C1110
void MPScript_ForceControllerErrorMessageDown()
{
    g_controllerConnectedErrorShown[0] = 0;
}

// ea: 0x005C11C0 (empty stub)
void MPScript_SetCompassVisibilty(unsigned int, bool)
{
}

// ea: 0x005C17E0 (tail jmp to tlPrintf)
void DebugOut(const char* strOut)
{
    tlPrintf(strOut);
}

// ea: 0x005C17F0
void FreezeMovement(bool value)
{
    g_freeze_movement = value;
}

// ea: 0x005C1940 (return true stub)
bool PrintObjectiveUpdate(Broc::string&, int, const char*)
{
    return true;
}

// ea: 0x005C5780 (return 0 stub)
unsigned int CreateNanoForce(const Broc::string&, const Broc::vector&,
                             const Broc::vector&)
{
    return 0;
}

// ea: 0x005BCB70
void MusicStop(float fadeOutTime)
{
    MusicMgr::sInst->Stop(fadeOutTime);
}

// ea: 0x005BCB90
void MusicIndoorStop(float fadeOutTime)
{
    MusicMgr::sInst->StopIndoor(fadeOutTime);
}

// ea: 0x005BCBB0
void SoundFadeIn(unsigned int handle, float time)
{
    SoundDevice::sInst->CrossFade(0, handle, time);
}

// ea: 0x005BCBD0
void SoundFadeOut(unsigned int handle, float time)
{
    SoundDevice::sInst->CrossFade(handle, 0, time);
}

// ea: 0x005BD440
bool HudIsPanelType(game_hudelem_s* hud)
{
    return hud->elem.type >= HE_TYPE_COUNT
           && hud->elem.type <= (HE_TYPE_COUNT | HE_TYPE_TIMER_DOWN);
}

// ea: 0x005BD600
void HudSetIsVisible(bool vla)
{
    g_femanager.mDontDrawHud = !vla;
}

// ea: 0x005BDB00
void GlowSetIntensityAux(float val)
{
    ShaderCommon::gGlowIntensity = val;
}

// ea: 0x005BDB20
void GlowSetExpansionAux(float val)
{
    ShaderCommon::gGlowExpansion = val;
}

// ea: 0x005BE7B0
unsigned int GetPlayer()
{
    Entity* p = EntityManager::sInst->GetPlayer(currCl);
    return p->mHandle.mHandle.mVal;
}

// ea: 0x005BEA60
void Scr_SetByte(Entity* ent, int offset, int* val)
{
    *((unsigned char*)ent + offset) = (unsigned char)*val;
}

// ea: 0x005BEA80
void Scr_GetByte(Entity* ent, int offset, int* val)
{
    *val = *((unsigned char*)ent + offset);
}

// ea: 0x005BEAA0
void Scr_SetWord(Entity* ent, int offset, int* val)
{
    *(int*)((unsigned char*)ent + offset) = *val;
}

// ea: 0x005BEAC0
void Scr_GetWord(Entity* ent, int offset, int* val)
{
    *val = *(int*)((unsigned char*)ent + offset);
}

// ea: 0x005BED40
bool BROC_AddCurveKeyEvaluator(unsigned int type, CurveEvalFunc function)
{
    CurveManager::sInst->AddKeyFunc(type, function);
    return true;
}

// ea: 0x005BED60
bool BROC_AddCurveConditionEvaluator(unsigned int type, CurveEvalFunc function)
{
    CurveManager::sInst->AddConditionFunc(type, function);
    return true;
}

// ea: 0x005BEDC0
int IsPathNodeDefined(unsigned int handle)
{
    return handle != 0 && handle != (unsigned int)-1;
}

// ea: 0x005BF210
void ActorScr_SetGoalRadius(actor_s* a, int /*offset*/, const float* val)
{
    Sentient_SetGoalRadius(a->pSentient, *val);
}

// ea: 0x005BF680
void SentientScr_SetGoalAngleTolerance(sentient_s* pSelf, int /*offset*/,
                                       float* val)
{
    Sentient_SetGoalAngleTolerance(pSelf, *val);
}

// ea: 0x005C10F0
bool MPScript_ControllerErrorMessageUp()
{
    int v0 = 0;
    while (!g_controllerConnectedErrorShown[v0])
    {
        if (++v0 >= 4)
            return false;
    }
    return true;
}

// ea: 0x005BF9B0
void OverrideTriggerLookAtRadius(float radius)
{
    gTriggerLookAtOverride = radius;
}

// ea: 0x005BF9D0
void SaveCheckpoint(const char* checkpointName)
{
    CheckpointMgr::sInst->SaveCheckpoint(checkpointName, true);
}

// ea: 0x005BF9F0
void SetGameUnsignedVar(unsigned int hashVarName, unsigned int iVal)
{
    CheckpointMgr::sInst->SetGameVar(hashVarName, &iVal, 1u);
}

// ea: 0x005BFA10
void SetGameFloatVar(unsigned int hashVarName, float fVal)
{
    CheckpointMgr::sInst->SetGameVar(hashVarName, (unsigned int*)&fVal, 1u);
}

// ea: 0x005C1A40 (empty stub)
void UpdateNPCtoVehicleMovement(unsigned int, unsigned int)
{
}

// ea: 0x005BC580 (thunk to View::IsSplitScreen)
bool IsSplitScreen()
{
    return View::IsSplitScreen();
}

// ea: 0x005BC550 (thunk to MPUIInterface::IsLANGame)
bool IsLanGame()
{
    return MPUIInterface::IsLANGame();
}

// ea: 0x005BC560 (thunk to MPUIInterface::IsOnlineGame)
bool IsOnlineGame()
{
    return MPUIInterface::IsOnlineGame();
}

// ea: 0x005BC570 (thunk to MPUIInterface::IsLocalGame)
bool IsLocalGame()
{
    return MPUIInterface::IsLocalGame();
}

// ea: 0x005BC820
int ModXY(int x, int y)
{
    return x % y;
}

// ea: 0x005BC830
float FModXY(float x, float y)
{
    return fmodf(x, y);
}

// ea: 0x005BC880 (mov eax, AeThreadManager::sInst.mThreadExecuting)
bool ThreadIsThreadExecuting()
{
    return AeThreadManager::sInst.mThreadExecuting != nullptr;
}

// ea: 0x005BC9A0
int CVarGetInt(const char* cvarName)
{
    return Cvar_VariableIntegerValue(cvarName);
}

// ea: 0x005BCC10
void SetIndoor(bool indoor)
{
    g_indoor = indoor;
}

// ea: 0x005BCC80
unsigned int GetTime()
{
    return level.time;
}

// ea: 0x005BCC90
float GetDeltaTime()
{
    return ServerTime::sInst.mTickDelta;
}

// ea: 0x005BCD10 (thunk to CG_GetNorthDirection)
float GetNorthYaw()
{
    return CG_GetNorthDirection();
}

// ea: 0x005BCD20 (empty stub)
void GameSave(const Broc::string&)
{
}

// ea: 0x005BCD30 (empty stub)
void GameLoad(const Broc::string&)
{
}

// ea: 0x005BCDC0
void SetPlayerIgnoreRadiusDamage(bool bVal)
{
    level.bPlayerIgnoreRadiusDamageLatched = bVal;
}

// ea: 0x005BCDD0 (empty stub)
void MissionSuccess(const Broc::string&)
{
}

// ea: 0x005BCE60
void SetRainDrops(bool on)
{
    FX_SetRainDrops(on);
}

// ea: 0x005BCE90
void DrawCompassFriendlies(bool inBool)
{
    level.bDrawCompassFriendlies = inBool;
}

// ea: 0x005BCEC0
void SetMaxVehicles(int vehicles)
{
    vehicle_InitDynamicBuffers(vehicles);
}

// ea: 0x005BCED0 (empty stub)
void ProfBegin()
{
}

// ea: 0x005BCEE0 (empty stub)
void ProfEnd()
{
}

// ea: 0x005BCF10 (thunk to G_FlushCorpses)
void FlushCorpses()
{
    G_FlushCorpses();
}

// ea: 0x005BCF20 (empty stub)
void StartMemCheck()
{
}

// ea: 0x005BCF30 (empty stub)
void EndMemCheck()
{
}

// ea: 0x005BD2B0 (return 0 stub)
int ProfileDeclareID(const char*)
{
    return 0;
}

// ea: 0x005BD2C0 (empty stub)
void ProfileStart(int)
{
}

// ea: 0x005BD2D0 (empty stub)
void ProfileStop(int)
{
}

// ea: 0x005BD2E0 (empty stub)
void ProfileSetVal(int, int)
{
}

// ea: 0x005BD900 (return false stub)
bool SetMissionToTrack(const char*, bool)
{
    return false;
}

// ea: 0x005BD910 (return 0 stub)
int GetMissionStat(int, bool)
{
    return 0;
}

// ea: 0x005BD920 (return 0 stub)
int GetMissionStatAll(int)
{
    return 0;
}

// ea: 0x005BD930 (empty stub)
void SetMissionStat(int, int)
{
}

// ea: 0x005BD940 (empty stub)
void IncMissionStat(int)
{
}

// ea: 0x005BD950 (empty stub)
void DecMissionStat(int)
{
}

// ea: 0x005BD960 (return 0 stub)
float GetAvgMissionStat(int)
{
    return 0.0f;
}

// ea: 0x005BD970 (return 0 stub)
int GetMissionCompletionTime()
{
    return 0;
}

// ea: 0x005BD980 (empty stub)
void UpdateMissionCompletionTime()
{
}

// ea: 0x005BD990 (empty stub)
void SetPlayerWeaponUsed(int)
{
}

// ea: 0x005BD9A0 (return false stub)
bool WasPlayerWeaponUsed(int)
{
    return false;
}

// ea: 0x005BD9B0 (return false stub)
bool WasPlayerWeaponCategoryUsed(int, bool)
{
    return false;
}

// ea: 0x005BDA30 (mov byte ptr [light+0x3D], 1)
void RemoveDynamicLight(unsigned int light)
{
    if (light != 0)
        *((unsigned char*)light + 0x3D) = 1;
}

// ea: 0x005BDAF0 (empty stub)
void EnableNanoForces(bool)
{
}

// ea: 0x005BDB60
void GlowSetGodRaysAux(bool val)
{
    ShaderCommon::gGlowGodRays = val;
}

// ea: 0x005BDB70
void GlowSetPassesAux(int val)
{
    ShaderCommon::gGlowPasses = val;
}

// ea: 0x005BDB80
void CurGenMotionBlur(float fLevel, float fPlateauTime, float fFadeTime)
{
    CG_MotionBlur::Begin(fLevel, fPlateauTime, fFadeTime);
}

// ea: 0x005BC210
const unsigned int ConvertStringToHash(const char* str)
{
    if (str == nullptr)
        return 0;
    tlFixedString tmp(str);
    return tmp.hash;
}

// ea: 0x005BC230
void MemFree(void* p)
{
    if (gBrocPool->InPool(p))
    {
        gBrocPool->Release(p);
    }
    else if (!((ae_heap_wrapper*)gBrocHeap)->CheckFree(p))
    {
        mem_heap_free(p);
    }
}

// ea: 0x005BC280
void* PoolAlloc(unsigned int s)
{
    return gCommonPoolAllocator->Allocate(s, false);
}

// ea: 0x005BC2A0
void PoolFree(void* p)
{
    gCommonPoolAllocator->Release(p);
}

// ea: 0x005BC490
void SetSpecialRecharge(int start, int length, int playerClass, int playerIndex)
{
    int v4 = 1580 * playerIndex;
    dword_F6419C[v4] = playerClass;
    dword_F641A0[v4] = length + start;
    dword_F641A4[v4] = length;
}

// ea: 0x005BC4C0
void AdvanceSpecialRecharge(int time)
{
    int v1 = 1580 * currCl;
    int v2 = dword_F641A0[1580 * currCl] - time;
    dword_F641A0[1580 * currCl] = v2;
    if (cgGlobal.time > v2)
    {
        dword_F641A0[v1] = 0;
        dword_F641A4[v1] = 0;
    }
}

// ea: 0x005BC500
int GetSpecialRechargePlayerClass()
{
    return dword_F6419C[1580 * currCl];
}

// ea: 0x005BC520
bool SpecialEditionSkin()
{
    return gCE;
}

// ea: 0x005BC530
bool IsHost()
{
    return MultiplayerMgr::sInst->IsHost();
}

// ea: 0x005BC540
bool IsRankedGame()
{
    return MultiplayerMgr::sInst->mRankedGame;
}

// ea: 0x005BC590
void SetTeamGame(bool teamGame)
{
    cgGlobal.teamGame = teamGame;
}

// ea: 0x005BC5A0
void SetShowScore(bool showScore)
{
    cgGlobal.showScore = showScore;
}

// ea: 0x005BC5E0
bool GetTeamGame()
{
    return cgGlobal.teamGame;
}

// ea: 0x005BC6D0 (tail jmp to MPUIInterface::ExitGame)
void QuitGame()
{
    MPUIInterface::ExitGame();
}

// ea: 0x005BC700 (tail jmp to Com_Printf)
void Print(const char* txt)
{
    Com_Printf(txt);
}

// ea: 0x005BC710
void PrintLn(const char* txt)
{
    Com_Printf("%s\n", txt);
}

// ea: 0x005BC810
float SquareRootX(float v)
{
    return sqrtf(v);
}

}  // namespace BrocSys
