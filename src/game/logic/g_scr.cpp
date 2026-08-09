// ============================================================================
// g_scr.cpp - script integration wrappers (g.o: g_scr_main.cpp / g_spawn.cpp)
// ============================================================================

#include "game/logic/g_local.h"

// ============================================================================
// BrocAPI - Broc exports (mBrocExports at +0xC50)
// ============================================================================
struct BrocExports {
    uint8_t _pad[0xC50];
    void (*mAnimInitialize)();  // +0xC50
};
struct BrocAPI {
    BrocExports mBrocExports;
};
extern BrocAPI* gpBrocAPI;      // 0xF3ABDC

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

extern saveField_t sentientFields[];
extern saveField_t actorFields[];
extern int g_xanim_num;                 // 0xF3A778
extern unsigned char sConstsLoaded;     // 0xEF357B
extern const char* gHashStringTblTxt[173];  // 0xDD6488
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
unsigned char GScr_LoadConsts()
{
    unsigned char result = sConstsLoaded;
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
    return result;
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
bool GScr_AddFieldsForEntity()
{
    bool result;
    AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_spawn.cpp";
    AeAssert::gCurrentLine = 617;
    AeAssert::gCurrentExpr = "0";
    result = AeAssert::IsIgnored();
    if (!result)
    {
        result = AeAssert::Assert("ma dead code");
        if (result)
            __debugbreak();
    }
    return result;
}

// ea: 0x004502E0
bool GScr_AddFieldsForRadiant()
{
    bool result;
    AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_spawn.cpp";
    AeAssert::gCurrentLine = 661;
    AeAssert::gCurrentExpr = "0";
    result = AeAssert::IsIgnored();
    if (!result)
    {
        result = AeAssert::Assert("ma dead code");
        if (result)
            __debugbreak();
    }
    return result;
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
