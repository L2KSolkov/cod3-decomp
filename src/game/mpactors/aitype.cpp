// ============================================================================
// aitype.cpp - mp_actors.o AIType / AITypeManager methods (AIType.cpp)
// ============================================================================

#include "game/mpactors/aitype.h"
#include "game/logic/g_local.h"

#include <new>
#include <stdlib.h>
#include <string.h>

extern char* va(const char* fmt, ...);  // core.o
extern void* mem_heap_malloc_ctx(unsigned int size, int alignment,
                                 const char* ctx, const char* file, int line);

// ============================================================================
// AIType
// ============================================================================

// ea: 0x0077BDB0
void AIType::Spawner(Entity* ent)
{
    if (ent == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::JRS;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\AIType.cpp";
        AeAssert::gCurrentLine = 170;
        AeAssert::gCurrentExpr = "ent";
        if (!AeAssert::IsIgnored() && AeAssert::Assert(defaultFileName))
            __debugbreak();
    }
    AITypeTeam mTeam = this->mTeam;
    if (mTeam != AITYPE_TEAM_AXIS)
    {
        int v4 = mTeam - 1;
        if (v4 != 0)
        {
            if (v4 == 1)
                ent->key = 3;
        }
        else
        {
            ent->key = 2;
        }
    }
    else
    {
        ent->key = 1;
    }
}

// ea: 0x00780EF0
void AIType::InitPlayer(Entity* ent, TPakId pakId)
{
    if (ent == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::JRS;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\AIType.cpp";
        AeAssert::gCurrentLine = 12;
        AeAssert::gCurrentExpr = "ent";
        if (!AeAssert::IsIgnored() && AeAssert::Assert(defaultFileName))
            __debugbreak();
    }
    if (this->mCharacters.mSize == 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::JRS;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\AIType.cpp";
        AeAssert::gCurrentLine = 13;
        AeAssert::gCurrentExpr = "mCharacters.size() > 0";
        if (!AeAssert::IsIgnored() && AeAssert::Assert(defaultFileName))
            __debugbreak();
    }
    unsigned int mSize = this->mCharacters.mSize;
    unsigned int v6 = mSize;
    CharacterType* v8 =
        &this->mCharacters.mList[rand() % v6];
    G_SetModel(ent, v8->mBodyName.mStr, pakId, 0);
}

// ea: 0x00785CA0
void AIType::InitEnt(Entity* ent)
{
    if (ent == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::JRS;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\AIType.cpp";
        AeAssert::gCurrentLine = 36;
        AeAssert::gCurrentExpr = "ent";
        if (!AeAssert::IsIgnored() && AeAssert::Assert(defaultFileName))
            __debugbreak();
    }
    if (this->mCharacters.mSize == 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::JRS;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\AIType.cpp";
        AeAssert::gCurrentLine = 37;
        AeAssert::gCurrentExpr = "mCharacters.size() > 0";
        if (!AeAssert::IsIgnored() && AeAssert::Assert(defaultFileName))
            __debugbreak();
    }
    unsigned int mSize = this->mCharacters.mSize;
    int modelIndex = rand() % mSize;
    CharacterType* v6 = &this->mCharacters.mList[modelIndex];
    G_SetModel(ent, v6->mBodyName.mStr, PAK_ID_INVALID, 0);
    CharacterType* v7 = &this->mCharacters.mList[modelIndex];
    char nm[128];
    char oBuff[128];
    char dstBuff[128];
    int oLen = 0;
    AeStringSupport::CStrToAeStr(nm, &oLen, 127,
                                 v7->mPopedHelmetName.mStr);
    nm[127] = (char)oLen;
    dstBuff[0] = 0;
    AeStringSupport::GetFileName(dstBuff, &oLen, nm, oLen, true);
    int v15 = 0;
    AeStringSupport::AeStrCopy(oBuff, &v15, 127, dstBuff, oLen);
    oBuff[127] = (char)v15;
    memcpy(nm, oBuff, sizeof(nm));
    if ((ent->flags & 0x2000000) != 0)
        ent->mTarget = nm;
    CharacterType* v8 = &this->mCharacters.mList[modelIndex];
    ent->mGroupName = v8->mVoiceName.mStr;
    AITypeTeam mTeam = this->mTeam;
    if (mTeam != AITYPE_TEAM_AXIS)
    {
        int v10 = mTeam - 1;
        if (v10 != 0)
        {
            if (v10 == 1)
                ent->team = "neutral";
        }
        else
        {
            ent->team = "allies";
        }
    }
    else
    {
        ent->team = "axis";
    }
    G_DObjUpdate(ent, false);
    g_LinkEntity(ent);
}

// ea: 0x00785E90
void AIType::InitActor(actor_s* actor)
{
    if (actor == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::JRS;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\AIType.cpp";
        AeAssert::gCurrentLine = 93;
        AeAssert::gCurrentExpr = "actor";
        if (!AeAssert::IsIgnored() && AeAssert::Assert(defaultFileName))
            __debugbreak();
    }
    sentient_s* pSent = actor->pSentient;
    if (pSent == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::JRS;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\AIType.cpp";
        AeAssert::gCurrentLine = 94;
        AeAssert::gCurrentExpr = "actor->pSentient";
        if (!AeAssert::IsIgnored() && AeAssert::Assert(defaultFileName))
            __debugbreak();
    }
    Entity* pEnt = actor->pEnt;
    if (pEnt == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::JRS;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\AIType.cpp";
        AeAssert::gCurrentLine = 95;
        AeAssert::gCurrentExpr = "actor->pEnt";
        if (!AeAssert::IsIgnored() && AeAssert::Assert(defaultFileName))
            __debugbreak();
    }
    AITypeTeam mTeam = this->mTeam;
    if (mTeam != AITYPE_TEAM_AXIS)
    {
        int v7 = mTeam - 1;
        if (v7 != 0)
        {
            if (v7 == 1)
                pSent->eTeam = TEAM_NEUTRAL;
        }
        else
        {
            pSent->eTeam = TEAM_ALLIES;
        }
    }
    else
    {
        pSent->eTeam = TEAM_AXIS;
    }
    actor->accuracyVsPlayer = this->mAccuracyVsPlayer;
    actor->accuracyVsAI = this->mAccuracyVsAI;
    pEnt->health = this->mHealth;
    pSent->fScariness = this->mScariness;
    actor->fBravery = this->mBravery;
    actor->iGrenadeAmmo = this->mGrenadeAmmo;
    if (this->mWeapon.mStr != nullptr)
        actor->mWeaponName =
            BrocSys::RegisterHashString(this->mWeapon.mStr);
    if (this->mSecondaryWeapon.mStr != nullptr)
        actor->mSecondaryWeaponName =
            BrocSys::RegisterHashString(this->mSecondaryWeapon.mStr);
    if (this->mGrenadeWeapon.mStr != nullptr)
        actor->iGrenadeWeaponIndex =
            BG_GetWeaponIndexForName(this->mGrenadeWeapon.mStr);
    if (this->mCharacters.mSize == 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::JRS;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\AIType.cpp";
        AeAssert::gCurrentLine = 134;
        AeAssert::gCurrentExpr = "mCharacters.size() > 0";
        if (!AeAssert::IsIgnored() && AeAssert::Assert(defaultFileName))
            __debugbreak();
    }
    unsigned int v10 = this->mCharacters.mSize;
    unsigned int v11 = rand() % v10;
    CharacterType* v12 = &this->mCharacters.mList[v11];
    G_SetModel(pEnt, v12->mBodyName.mStr, PAK_ID_INVALID, 0);
    CharacterType* v13 = &this->mCharacters.mList[v11];
    actor->mVoiceName = v13->mVoiceName.mStr;
    CharacterType* v14 = &this->mCharacters.mList[v11];
    char nm[128];
    char oBuff[128];
    char dstBuff[128];
    int oLen = 0;
    AeStringSupport::CStrToAeStr(nm, &oLen, 127,
                                 v14->mPopedHelmetName.mStr);
    dstBuff[0] = 0;
    AeStringSupport::GetFileName(dstBuff, &oLen, nm, oLen, true);
    int v19 = 0;
    AeStringSupport::AeStrCopy(oBuff, &v19, 127, dstBuff, oLen);
    actor->mPopedHelmetName = oBuff;
    G_DObjUpdate(pEnt, false);
    g_LinkEntity(pEnt);
}

// ============================================================================
// AITypeManager
// ============================================================================

// ea: 0x005E9F80
AITypeManager* AITypeManager::Inst()
{
    return AITypeManager::sInst;
}

// ea: 0x00783A70
AITypeManager::AITypeManager()
{
}

// ea: 0x004DCEC0
void AITypeManager::CreateInst()
{
    if (sInst != nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\AITypeManager.h";
        AeAssert::gCurrentLine = 32;
        AeAssert::gCurrentExpr = "sInst==0";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("singleton already created!"))
            __debugbreak();
    }

    AITypeManager* result = static_cast<AITypeManager*>(
        mem_heap_malloc_ctx(0x190u, 4, "core",
                            "c:\\cod\\code\\game\\AITypeManager.h", 32));
    if (result != nullptr)
    {
        result = new (result) AITypeManager();
        sInst = result;
    }
    else
    {
        sInst = nullptr;
    }
}

// ea: 0x004DCFC0
void AITypeManager::DeleteInst()
{
    if (sInst == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\AITypeManager.h";
        AeAssert::gCurrentLine = 32;
        AeAssert::gCurrentExpr = "sInst!=0";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("singleton not created!"))
            __debugbreak();
    }

    if (sInst != nullptr)
        delete sInst;
    sInst = nullptr;
}

// ea: 0x00780FC0
AITypeManager::~AITypeManager()
{
}

// ea: 0x00786170
IVPointer<AIType> AITypeManager::GetAIType(TPakId pak_id, const char* name,
                                            int nameOffset)
{
    IVPointer<AIType> ait;
    ait.mValue = nullptr;
    ait.mPakId = 0;
    (void)pak_id;
    (void)name;
    (void)nameOffset;
    return ait;
}
