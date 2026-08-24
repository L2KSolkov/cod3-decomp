// ============================================================================
// g_dobj.cpp - DObj/XAnim script wrappers + anim-tree helpers (g.o)
// Source: g_syscalls.cpp / g_utils.cpp / g_client.cpp families
// ============================================================================

#include "game/logic/g_local.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "core/tlFixedString.h"
#include "core/PoolAllocator.h"

extern void DObjSkelMatrixMultiply43(const DObjSkelMat* in1,
                                       const float (*const in2)[3],
                                     float (*const out)[3]);
extern void DObjUpdateChildren(DObj* obj, int boneIndex);  // render.o 0xABB990

// ea: 0x4A6AF0
Entity* GetPlayer(int idx)
{
    return EntityManager::sInst->GetPlayer(idx);
}

// ea: 0x0044A190
int G_FindInvalidatedNode(Entity* pEnt, const PathNodes::PathNode* pNode)
{
    if (pEnt == nullptr || pEnt->client == nullptr || pNode == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_client.cpp";
        AeAssert::gCurrentLine = 1493;
        AeAssert::gCurrentExpr = "pEnt && pEnt->client && pNode";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (pEnt == nullptr)
        return -1;
    Client* client = pEnt->client;
    if (client == nullptr)
        return -1;
    if (pNode == nullptr)
        return -1;
    int mInvalidatedNodeNum = client->mInvalidatedNodeNum;
    int result = 0;
    if (mInvalidatedNodeNum <= 0)
        return -1;
    for (PathNodes::NodeHandle* i = client->mInvalidatedNode; i->mValue != pNode->mHandle.mValue; ++i)
    {
        if (++result >= mInvalidatedNodeNum)
            return -1;
    }
    return result;
}

// ea: 0x004690B0
void UpdateAnims(int msec)
{
    cdl_proftimer_dobj_anim.start();
    float deltaT = msec * 0.001f;
    TaskFunctor1_Anim ftorA;
    ftorA.__vftable = nullptr;
    ftorA.fn = nullptr;
    ftorA.deltaT = deltaT;
    TaskHandler_Update(AnimationUpdateTask_sHandler(), deltaT, &ftorA);
    TaskFunctor1_XAnim ftorX1;
    ftorX1.__vftable = nullptr;
    ftorX1.fn = (void*)0x1;  // UpdateServerTime slot
    ftorX1.deltaT = deltaT;
    TaskHandler_Update(XAnimUpdateTask_sHandler(), deltaT, &ftorX1);
    TaskFunctor1_XAnim ftorX2;
    ftorX2.__vftable = nullptr;
    ftorX2.fn = (void*)0x2;  // CalcAnim1 slot
    ftorX2.deltaT = deltaT;
    TaskHandler_Update(XAnimUpdateTask_sHandler(), deltaT, &ftorX2);
    for (int i = 0; i < 16; ++i)
    {
        Entity* v2 = EntityManager::sInst->mPlayers[i];
        if (v2 != nullptr && v2->client != nullptr)
        {
            tagInfo_t* tagInfo = v2->tagInfo;
            if (tagInfo != nullptr)
            {
                Entity* v4 = tagInfo->parent;
                if (v4->scr_vehicle != nullptr)
                    VEH_UpdateControllers(v4, 0);
            }
        }
    }
    AnimQueue_ExecuteMatrixQueue();
    TaskFunctor1_Anim ftorA2;
    ftorA2.__vftable = nullptr;
    ftorA2.fn = (void*)0x3;  // ApplyPose slot
    ftorA2.deltaT = deltaT;
    TaskHandler_Update(AnimationUpdateTask_sHandler(), deltaT, &ftorA2);
    TaskFunctor1_XAnim ftorX3;
    ftorX3.__vftable = nullptr;
    ftorX3.fn = (void*)0x4;  // CalcAnim2 slot
    ftorX3.deltaT = deltaT;
    TaskHandler_Update(XAnimUpdateTask_sHandler(), deltaT, &ftorX3);
    int mSize = dobjects.mSize;
    for (int v6 = 0; v6 < dobjects.mSize; ++v6)
    {
        if (v6 < 0 || v6 >= mSize)
        {
            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
            AeAssert::gCurrentFile = "../ae\\core/ae_vector.h";
            AeAssert::gCurrentLine = 167;
            AeAssert::gCurrentExpr = "iIndex >= 0 && iIndex < mSize";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("out of bounds"))
                __debugbreak();
        }
        unsigned int v7 = dobjects.mElements[v6].mHandle.mVal & 0xFFF;
        if (v7 < 0x540
            && dobjects.mElements[v6].mHandle.mVal >> 12
                   == EntityHandleDb::sInst.mElements[v7].mKey)
        {
            Entity* mObject = EntityHandleDb::sInst.mElements[v7].mObject;
            if (mObject != nullptr)
            {
                DObjUpdateLod(mObject);
                if ((mObject->flags & 0x10000) == 0
                    || (mObject->mFlags & 0x10) != 0)
                    CG_DoControllers(mObject);
            }
        }
        mSize = dobjects.mSize;
    }
    AnimQueue_ClearMatrixQueue();
    cdl_proftimer_dobj_anim.stop();
}

// ea: 0x0044A230
void G_UpdateInvalidatedNode(Entity* /*pEnt*/)
{
    ;
}

// ea: 0x00450D40
scr_anim_s g_XAnimGetRoot(XAnimTree* tree)
{
    scr_anim_s anim;
    anim.mHandle = (uint16_t)Scr_GetAnimsIndex(XAnimGetAnims(tree));
    return anim;
}

// ea: 0x00450D70
void XAnimClearTreeGoalWeights(XAnimTree* tree, scr_anim_s anim, float blendtime)
{
    AnimTree* Anims = Scr_GetAnims(((anim.mHandle >> 16) & 0xFFFF));
    if (Anims != XAnimGetAnims(tree))
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_syscalls.cpp";
        AeAssert::gCurrentLine = 271;
        AeAssert::gCurrentExpr = "Scr_GetAnims(anim.tree) == XAnimGetAnims(tree)";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    XAnimClearTreeGoalWeights(tree, anim.mHandle, blendtime);
}

// ea: 0x00450DF0
void XAnimClearGoalWeight(XAnimTree* tree, scr_anim_s anim, float blendtime)
{
    AnimTree* Anims = Scr_GetAnims(((anim.mHandle >> 16) & 0xFFFF));
    if (Anims != XAnimGetAnims(tree))
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_syscalls.cpp";
        AeAssert::gCurrentLine = 276;
        AeAssert::gCurrentExpr = "Scr_GetAnims(anim.tree) == XAnimGetAnims(tree)";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    XAnimClearGoalWeight(tree, anim.mHandle, blendtime);
}

// ea: 0x00450E70
void XAnimClearTreeGoalWeightsStrict(XAnimTree* tree, scr_anim_s anim, float blendtime)
{
    AnimTree* Anims = Scr_GetAnims(((anim.mHandle >> 16) & 0xFFFF));
    if (Anims != XAnimGetAnims(tree))
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_syscalls.cpp";
        AeAssert::gCurrentLine = 281;
        AeAssert::gCurrentExpr = "Scr_GetAnims(anim.tree) == XAnimGetAnims(tree)";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    XAnimClearTreeGoalWeightsStrict(tree, anim.mHandle, blendtime);
}

// ea: 0x00450EF0
void XAnimSetAnimRate(XAnimTree* tree, scr_anim_s anim, float rate)
{
    AnimTree* Anims = Scr_GetAnims(((anim.mHandle >> 16) & 0xFFFF));
    if (Anims != XAnimGetAnims(tree))
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_syscalls.cpp";
        AeAssert::gCurrentLine = 298;
        AeAssert::gCurrentExpr = "Scr_GetAnims(anim.tree) == XAnimGetAnims(tree)";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    XAnimSetAnimRate(tree, anim.mHandle, rate);
}

// ea: 0x00450F70
void XAnimSetTime(XAnimTree* tree, scr_anim_s anim, float time)
{
    AnimTree* Anims = Scr_GetAnims(((anim.mHandle >> 16) & 0xFFFF));
    if (Anims != XAnimGetAnims(tree))
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_syscalls.cpp";
        AeAssert::gCurrentLine = 303;
        AeAssert::gCurrentExpr = "Scr_GetAnims(anim.tree) == XAnimGetAnims(tree)";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    XAnimSetTime(tree, anim.mHandle, time);
}

// ea: 0x00450FF0
int XAnimHasTime(scr_anim_s anim)
{
    AnimTree* Anims = Scr_GetAnims(((anim.mHandle >> 16) & 0xFFFF));
    return XAnimHasTime(Anims, anim.mHandle);
}

// ea: 0x00451020
int XAnimIsPrimitive(scr_anim_s anim)
{
    AnimTree* Anims = Scr_GetAnims(((anim.mHandle >> 16) & 0xFFFF));
    return XAnimIsPrimitive(Anims, anim.mHandle);
}

// ea: 0x00451050
float XAnimGetLength(scr_anim_s anim)
{
    AnimTree* Anims = Scr_GetAnims(((anim.mHandle >> 16) & 0xFFFF));
    return XAnimGetLength(Anims, anim.mHandle);
}

// ea: 0x00451080
void XAnimCalcAbsDelta(XAnimTree* tree, scr_anim_s anim, float* rot, float* trans)
{
    AnimTree* Anims = Scr_GetAnims(((anim.mHandle >> 16) & 0xFFFF));
    if (Anims != XAnimGetAnims(tree))
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_syscalls.cpp";
        AeAssert::gCurrentLine = 346;
        AeAssert::gCurrentExpr = "Scr_GetAnims(anim.tree) == XAnimGetAnims(tree)";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    XAnimCalcAbsDelta(tree, anim.mHandle, rot, trans);
}

// ea: 0x00451100
void XAnimGetRelDelta(scr_anim_s anim, float* rot, float* trans, float time1, float time2)
{
    AnimTree* Anims = Scr_GetAnims(((anim.mHandle >> 16) & 0xFFFF));
    XAnimGetRelDelta(Anims, anim.mHandle, rot, trans, time1, time2);
}

// ea: 0x00451140
void XAnimGetAbsDelta(scr_anim_s anim, float* rot, float* trans, float time)
{
    AnimTree* Anims = Scr_GetAnims(((anim.mHandle >> 16) & 0xFFFF));
    XAnimGetAbsDelta(Anims, anim.mHandle, rot, trans, time);
}

// ea: 0x00451170
int XAnimIsLooped(scr_anim_s anim)
{
    AnimTree* Anims = Scr_GetAnims(((anim.mHandle >> 16) & 0xFFFF));
    return XAnimIsLooped(Anims, anim.mHandle);
}

// ea: 0x004511A0
bool XAnimNotetrackExists(scr_anim_s anim, const unsigned int& name)
{
    AnimTree* Anims = Scr_GetAnims(((anim.mHandle >> 16) & 0xFFFF));
    return XAnimNotetrackExists(Anims, anim.mHandle, name);
}


// ea: 0x004511D0
float XAnimGetTime(XAnimTree* tree, scr_anim_s anim)
{
    AnimTree* Anims = Scr_GetAnims(((anim.mHandle >> 16) & 0xFFFF));
    if (Anims != XAnimGetAnims(tree))
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_syscalls.cpp";
        AeAssert::gCurrentLine = 367;
        AeAssert::gCurrentExpr = "Scr_GetAnims(anim.tree) == XAnimGetAnims(tree)";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    return XAnimGetTime(tree, anim.mHandle);
}

// ea: 0x00451250
float XAnimGetWeight(XAnimTree* tree, scr_anim_s anim)
{
    AnimTree* Anims = Scr_GetAnims(((anim.mHandle >> 16) & 0xFFFF));
    if (Anims != XAnimGetAnims(tree))
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_syscalls.cpp";
        AeAssert::gCurrentLine = 372;
        AeAssert::gCurrentExpr = "Scr_GetAnims(anim.tree) == XAnimGetAnims(tree)";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    return XAnimGetWeight(tree, anim.mHandle);
}

// ea: 0x004512D0
int XAnimHasFinished(XAnimTree* tree, scr_anim_s anim)
{
    AnimTree* Anims = Scr_GetAnims(((anim.mHandle >> 16) & 0xFFFF));
    if (Anims != XAnimGetAnims(tree))
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_syscalls.cpp";
        AeAssert::gCurrentLine = 389;
        AeAssert::gCurrentExpr = "Scr_GetAnims(anim.tree) == XAnimGetAnims(tree)";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    return XAnimHasFinished(tree, anim.mHandle);
}

// ea: 0x00451350
int XAnimGetNumChildren(scr_anim_s anim)
{
    AnimTree* Anims = Scr_GetAnims(((anim.mHandle >> 16) & 0xFFFF));
    return XAnimGetNumChildren(Anims, anim.mHandle);
}

// ea: 0x00451380
scr_anim_s XAnimGetChildAt(scr_anim_s anim, int index)
{
    AnimTree* Anims = Scr_GetAnims(((anim.mHandle >> 16) & 0xFFFF));
    anim.mHandle = (anim.mHandle & 0xFFFF0000u) | XAnimGetChildAt(Anims, anim.mHandle, index);
    return anim;
}

// ea: 0x004513B0
const char* XAnimGetAnimName(scr_anim_s anim)
{
    AnimTree* Anims = Scr_GetAnims(((anim.mHandle >> 16) & 0xFFFF));
    return XAnimGetAnimName(Anims, anim.mHandle);
}

// ea: 0x00453B80
XAnimTree* G_GetEntAnimTree(Entity* ent)
{
    if (ent->s.eType == 11)
        return G_GetActorAnimTree(ent->actor);
    if (ent->s.eType == 13)
        return G_GetActorCorpseAnimTree(ent);
    return ent->pAnimTree;
}

// ea: 0x00453BC0
void G_SetModelIndex(Entity* ent, int iflIndex)
{
    ent->GetRenderEntity().iflIndex = (uint8_t)iflIndex;
}

// ea: 0x00453C30
bool G_DObjUpdateServerTime(Entity* ent, int msec, bool bNotify)
{
    return SV_DObjUpdateServerTime(ent, msec * 0.001f, bNotify);
}

// ea: 0x00453DC0
void G_DObjCalcPose(Entity* ent)
{
    int partBits[4];
    if (!SV_DObjCreateSkelForBones(ent))
    {
        memset(partBits, 255, sizeof(partBits));
        if (ent != nullptr)
        {
            SV_DObjCalcAnim(ent, -1);
            uint8_t controller = ent->controller;
            if (controller != 0)
            {
                if (controller >= 4u)
                {
                    AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
                    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_utils.cpp";
                    AeAssert::gCurrentLine = 1629;
                    AeAssert::gCurrentExpr = "ent->controller > 0 && ent->controller < CONTROLLER_MAX";
                    if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
                        __debugbreak();
                }
                controllertable[ent->controller](ent, partBits);
            }
            SV_DObjCalcSkel(ent, partBits);
        }
    }
}

// ea: 0x00453E80
void G_DObjCalcBone(Entity* ent, int boneIndex)
{
    int partBits[4];
    if (!SV_DObjCreateSkelForBone(ent, boneIndex))
    {
        SV_DObjGetHierarchyBits(ent, boneIndex, partBits);
        SV_DObjCalcAnim(ent, -1);
        uint8_t controller = ent->controller;
        if (controller != 0)
        {
            if (controller >= 4u)
            {
                AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
                AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_utils.cpp";
                AeAssert::gCurrentLine = 1650;
                AeAssert::gCurrentExpr = "ent->controller > 0 && ent->controller < CONTROLLER_MAX";
                if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
                    __debugbreak();
            }
            controllertable[ent->controller](ent, partBits);
        }
        SV_DObjCalcSkel(ent, partBits);
    }
}

// ea: 0x00453F40
const DObjSkelMat* G_DObjGetLocalBoneIndexMatrix(Entity* ent, int boneIndex)
{
    G_DObjCalcBone(ent, boneIndex);
    return &SV_DObjGetMatrixArray(ent)[boneIndex];
}

// ea: 0x00453F70
bool G_DObjGetWorldBoneIndexMatrix(Entity* ent, int boneIndex, DObjSkelMat* tagMat)
{
    G_DObjCalcBone(ent, boneIndex);
    const DObjSkelMat* v3 = &SV_DObjGetMatrixArray(ent)[boneIndex];
    if (v3 == nullptr)
        return 0;
    float ent_axis[12];
    AnglesToAxis(ent->r.currentAngles, (float(*)[3])ent_axis);
    ent_axis[9] = ent->r.currentOrigin.v.m128_f32[0];
    ent_axis[10] = ent->r.currentOrigin.v.m128_f32[1];
    ent_axis[11] = ent->r.currentOrigin.v.m128_f32[2];
    DObjSkel2MatrixMultiply43(v3, (const float(*)[3])ent_axis, tagMat);
    return 1;
}

// ea: 0x00454000
const DObjSkelMat* G_DObjGetLocalTagMatrix(Entity* ent, unsigned int tag_name_hash)
{
    int BoneIndex = SV_DObjGetBoneIndex(ent, tag_name_hash);
    int v3 = BoneIndex;
    if (BoneIndex < 0)
        return nullptr;
    G_DObjCalcBone(ent, BoneIndex);
    return &SV_DObjGetMatrixArray(ent)[v3];
}

// ea: 0x00454040
int G_DObjGetWorldTagMatrix(Entity* ent, unsigned int tag_name_hash, DObjSkelMat* tagMat)
{
    int BoneIndex = SV_DObjGetBoneIndex(ent, tag_name_hash);
    if (BoneIndex < 0)
        return 0;
    G_DObjGetWorldBoneIndexMatrix(ent, BoneIndex, tagMat);
    return 1;
}

// ea: 0x004543B0
void G_RegisterSoundWait(Entity* /*ent*/, int /*notifyHash*/, unsigned short /*soundName*/, int /*time*/)
{
    ;
}

// ea: 0x004543C0
void RegisterEffectWait(Entity* ent, int notifyHash)
{
    if (ent->snd_wait.notifyHash.mHash != 0)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_utils.cpp";
        AeAssert::gCurrentLine = 2144;
        AeAssert::gCurrentExpr = "ent->snd_wait.notifyHash.GetHash()==0";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Tried to set a notify on an entity that already has one set"))
        {
            __debugbreak();
        }
    }
    EffectEventSys::sInst->SendSoundNotify(ent);
    ent->snd_wait.notifyHash.mHash = notifyHash;
    ent->snd_wait.soundName.mHash = 0;
}

// ea: 0x00454A70
void G_CleanupAnimTrees()
{
    for (int i = 0; i < level.delayFreeAnimTreeCount; ++i)
    {
        XAnimTree* v1 = level.delayFreeAnimTree[i];
        XAnimClearTree(v1);
        Com_XAnimFreeSmallTree(v1);
    }
    int v3 = 0;
    for (level.delayFreeAnimTreeCount = 0; v3 < level.delayClearAnimTreeCount; ++v3)
    {
        XAnimClearTree(level.delayClearAnimTree[v3]);
    }
    level.delayClearAnimTreeCount = 0;
}

// ea: 0x00455DF0
static void G_RmvInvalidatedNodeRemove(Entity* pEnt, int iRmv);
void G_RmvInvalidatedNode(Entity* pEnt, const PathNodes::PathNode* pNode)
{
    if (pEnt == nullptr || pEnt->client == nullptr || pNode == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_client.cpp";
        AeAssert::gCurrentLine = 1475;
        AeAssert::gCurrentExpr = "pEnt && pEnt->client && pNode";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (pEnt != nullptr && pEnt->client != nullptr && pNode != nullptr)
    {
        int InvalidatedNode = G_FindInvalidatedNode(pEnt, pNode);
        if (InvalidatedNode >= 0)
            G_RmvInvalidatedNodeRemove(pEnt, InvalidatedNode);
    }
}

// ea: 0x44A0D0 (static helper; shift-down remove by index)
static void G_RmvInvalidatedNodeRemove(Entity* pEnt, int iRmv)
{
    if (pEnt != nullptr && pEnt->client != nullptr && iRmv >= 0
        && iRmv < pEnt->client->mInvalidatedNodeNum)
    {
        Client* client = pEnt->client;
        for (int i = iRmv + 1; i < client->mInvalidatedNodeNum; ++i)
            client->mInvalidatedNode[i - 1] = client->mInvalidatedNode[i];
        --client->mInvalidatedNodeNum;
    }
}

// ea: 0x00460780
void G_DelayFreeAnimTree(XAnimTree* tree)
{
    if (level.delayFreeAnimTreeCount >= 0x200u)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_utils.cpp";
        AeAssert::gCurrentLine = 2300;
        AeAssert::gCurrentExpr = "0";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("level.delayFreeAnimTree size exceeded - Tell MikeA"))
            __debugbreak();
    }
    level.delayFreeAnimTree[level.delayFreeAnimTreeCount++] = tree;
    G_CleanupAnimTrees();
}

// ea: 0x004607F0
void G_DelayClearAnimTree(XAnimTree* tree)
{
    if (level.delayClearAnimTreeCount >= 0x80u)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_utils.cpp";
        AeAssert::gCurrentLine = 2316;
        AeAssert::gCurrentExpr = "0";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("level.delayClearAnimTree size exceeded"))
            __debugbreak();
    }
    level.delayClearAnimTree[level.delayClearAnimTreeCount++] = tree;
    G_CleanupAnimTrees();
}

// ea: 0x00464C70
void* G_GetModel(const char* modelName, TPakId pakId)
{
    IVPointer<XModel> xm;
    if (modelName != nullptr)
    {
        xm = XModelManager::sInst->GetXModel(pakId, modelName);
        ValidatePakId((TPakId)xm.mPakId);
        return xm.mValue;
    }
    else
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_utils.cpp";
        AeAssert::gCurrentLine = 481;
        AeAssert::gCurrentExpr = "modelName";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("G_SetModel: bad model name"))
            __debugbreak();
        return nullptr;
    }
}

// ============================================================================
// DObj tracking (register/unregister/commit) + entity ref cleanup
// ============================================================================

namespace {
void VectorAssertIndex(int idx, int mSize)
{
    if (idx < 0 || idx >= mSize)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "../ae\\core/ae_vector.h";
        AeAssert::gCurrentLine = 167;
        AeAssert::gCurrentExpr = "iIndex >= 0 && iIndex < mSize";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("out of bounds"))
            __debugbreak();
    }
}

void VectorRemove(ae_vector<DbLinkedHandle<EntityHandleDb, Entity>>& v, int idx)
{
    if (idx + 1 < v.mSize)
    {
        VectorAssertIndex(idx, v.mSize);
        VectorAssertIndex(v.mSize - 1, v.mSize);
        v.mElements[idx] = v.mElements[v.mSize - 1];
    }
    if (v.mSize != 0)
        --v.mSize;
}

void VectorPush(ae_vector<DbLinkedHandle<EntityHandleDb, Entity>>& v,
                const DbLinkedHandle<EntityHandleDb, Entity>& h)
{
    v.mElements[v.mSize++] = h;
}
}  // namespace

// ea: 0x00448B80
void DObjHandleDb_Init(void)
{
    ;
}

// ea: 0x00468E30
void register_dobj(DbLinkedHandle<EntityHandleDb, Entity> handle)
{
    int mSize = del_pending_dobjects.mSize;
    for (int v2 = 0; v2 < mSize; ++v2)
    {
        VectorAssertIndex(v2, del_pending_dobjects.mSize);
        if (del_pending_dobjects.mElements[v2].mHandle.mVal == handle.mHandle.mVal)
        {
            VectorRemove(del_pending_dobjects, v2);
            return;
        }
    }
    VectorPush(add_pending_dobjects, handle);
}

// ea: 0x00468EF0
void unregister_dobj(DbLinkedHandle<EntityHandleDb, Entity> handle)
{
    int mSize = add_pending_dobjects.mSize;
    for (int v2 = 0; v2 < mSize; ++v2)
    {
        VectorAssertIndex(v2, add_pending_dobjects.mSize);
        if (add_pending_dobjects.mElements[v2].mHandle.mVal == handle.mHandle.mVal)
        {
            VectorRemove(add_pending_dobjects, v2);
            return;
        }
    }
    VectorPush(del_pending_dobjects, handle);
}

// ea: 0x00468FB0
void init_dobj_trackers(void)
{
    dobjects.reserve(192);
    del_pending_dobjects.reserve(192);
    add_pending_dobjects.reserve(128);
}

// ea: 0x00468FE0
void DObjSetNotRenderedFlag(void)
{
    cdl_proftimer_dobj_anim.start();
    int mSize = dobjects.mSize;
    for (int i = 0; i < dobjects.mSize; ++i)
    {
        VectorAssertIndex(i, mSize);
        unsigned int v2 = dobjects.mElements[i].mHandle.mVal & 0xFFF;
        if (v2 < 0x540
            && dobjects.mElements[i].mHandle.mVal >> 12
                   == EntityHandleDb::sInst.mElements[v2].mKey)
        {
            Entity* mObject = EntityHandleDb::sInst.mElements[v2].mObject;
            if (mObject != nullptr)
            {
                DObj* mDObj = mObject->mDObj;
                if (mDObj != nullptr)
                    mDObj->mFlags |= 0xFu;
            }
        }
        mSize = dobjects.mSize;
    }
    cdl_proftimer_dobj_anim.stop();
}

// ea: 0x00477450
void commit_dobjects(void)
{
    for (int i = 0; i < del_pending_dobjects.mSize; ++i)
    {
        unsigned int mVal = del_pending_dobjects.mElements[i].mHandle.mVal;
        int size = dobjects.mSize;
        for (int v1 = 0; v1 < size; ++v1)
        {
            VectorAssertIndex(v1, dobjects.mSize);
            if (dobjects.mElements[v1].mHandle.mVal == mVal)
            {
                if (v1 + 1 < size)
                {
                    VectorAssertIndex(size - 1, dobjects.mSize);
                    dobjects.mElements[v1--] = dobjects.mElements[size - 1];
                }
                --size;
                if (dobjects.mSize != 0)
                    --dobjects.mSize;
            }
        }
    }
    for (int j = 0; j < add_pending_dobjects.mSize; ++j)
    {
        VectorPush(dobjects, add_pending_dobjects.mElements[j]);
    }
    add_pending_dobjects.mSize = 0;
    del_pending_dobjects.mSize = 0;
}

// ea: 0x00472C70
void G_EntDetachAll(Entity* ent)
{
    Broc::string* mAttachModels = (Broc::string*)ent->mAttachModels;
    for (int i = 7; i != 0; --i)
    {
        mAttachModels[0].mBlock = nullptr;
        mAttachModels[1].mBlock = (Broc::string::Block*)-1;
        mAttachModels[2].clear();
        mAttachModels += 3;
    }
    ent->attachIgnoreCollision = 0;
    G_DObjUpdate(ent, false);
}

// ea: 0x004736C0
void G_FreeEntityRefs(Entity* ed)
{
    if (level.turrets != nullptr)
    {
        if (level.turrets->manualTarget == ed)
            level.turrets->manualTarget = nullptr;
        if (level.turrets->target == ed)
            level.turrets->target = nullptr;
        if (level.turrets->detachSentient == ed->sentient)
            level.turrets->detachSentient = nullptr;
    }
    for (int i = 0; i < 32; ++i)
    {
        actor_s* actor = level.actors[i];
        if (actor != nullptr && actor->iSpawnTime >= 0 && actor->pPileUpEnt == ed)
        {
            actor->pPileUpActor = nullptr;
            actor->pPileUpEnt = nullptr;
        }
    }
    for (int i = 0; i < 16; ++i)
    {
        Client* v8 = level.clients != nullptr ? &level.clients[i] : nullptr;
        if (v8 != nullptr && v8->pLookatEnt == ed)
            v8->pLookatEnt = nullptr;
    }
    G_FreeVehicleRefs(ed);
}

// ea: 0x00472AE0
int G_EntDetach(Entity* ent, const char* modelName, const char* tagName)
{
    if (tagName == nullptr || *tagName == 0)
        return 0;
    if (_strnicmp(modelName, "xmodel/", 7) == 0)
        modelName += 7;
    int v3 = 0;
    Broc::string* i = &ent->mAttachModels[0].mTag;
    for (;; i += 3)
    {
        if (i->mBlock != nullptr
            && i->mBlock != (Broc::string::Block*)-12
            && *(const char*)&i->mBlock[1] != 0)
        {
            ValidatePakId((TPakId)(uintptr_t)i[-1].mBlock);
            if (i[-2].mBlock != nullptr)
            {
                const char* v5 = i->mBlock != nullptr ? (const char*)&i->mBlock[1]
                                                      : defaultFileName;
                if (_stricmp(v5, tagName) == 0)
                {
                    ValidatePakId((TPakId)(uintptr_t)i[-1].mBlock);
                    XModel* xm = (XModel*)i[-2].mBlock;
                    if (_stricmp(xm->name.mStr, modelName) == 0)
                        break;
                }
            }
        }
        if (++v3 >= 7)
            return 0;
    }
    Broc::string* v8 = (Broc::string*)&ent->mAttachModels[v3];
    v8[0].mBlock = nullptr;
    v8[1].mBlock = (Broc::string::Block*)-1;
    v8[2].clear();
    if (v3 < 6)
    {
        int tagNamea = v3 + 1;
        do
        {
            Broc::string::Block* mBlock = v8[4].mBlock;
            v8[0].mBlock = v8[3].mBlock;
            v8[1].mBlock = mBlock;
            v8[2] = v8[5];
            if (((1 << tagNamea) & ent->attachIgnoreCollision) != 0)
                ent->attachIgnoreCollision |= (1u << v3);
            else
                ent->attachIgnoreCollision &= ~(1u << v3);
            ++v3;
            v8 += 3;
            ++tagNamea;
        } while ((tagNamea + 1) < 7);
    }
    Broc::string* v13 = (Broc::string*)&ent->mAttachModels[v3];
    v13[0].mBlock = nullptr;
    v13[1].mBlock = (Broc::string::Block*)-1;
    v13[2].clear();
    ent->attachIgnoreCollision &= ~(1u << v3);
    G_DObjUpdate(ent, false);
    return 1;
}

// ============================================================================
// Local-bone / tag setters (helpers live in g_utils.cpp; setter wrappers)
// ============================================================================

// ea: 0x004735E0
int G_DObjSetLocalBoneIndex(Entity* ent, int* const /*partBits*/, int boneIndex,
                            const float* const trans, const float* const angles, bool bRelative)
{
    G_DObjSetLocalTagInternal_0(trans, angles, boneIndex, ent, bRelative);
    return 1;
}

// ea: 0x00473610
int G_DObjSetLocalBoneIndex(Entity* ent, int* const /*partBits*/, int boneIndex,
                            const math::Position3& trans, const math::Mat33& angles,
                            bool bRelative)
{
    float ftrans[3] = { trans.v.m128_f32[0], trans.v.m128_f32[1], trans.v.m128_f32[2] };
    float fangles[3];
    AxisToAngles((const float(*)[3])&angles, fangles);
    G_DObjSetLocalTagInternal_0(ftrans, fangles, boneIndex, ent, bRelative);
    return 1;
}

// ea: 0x00473640
int G_DObjSetLocalTag(Entity* ent, int* const /*partBits*/, unsigned int tag_name_hash,
                      const float* const trans, const float* const angles, bool bRelative)
{
    int BoneIndex = SV_DObjGetBoneIndex(ent, tag_name_hash);
    if (BoneIndex < 0)
        return 0;
    G_DObjSetLocalTagInternal_0(trans, angles, BoneIndex, ent, bRelative);
    return 1;
}

// ea: 0x00473680
int G_DObjSetControlTagAngles(Entity* ent, int* const /*partBits*/, unsigned int tag_name_hash,
                              float* const angles)
{
    int BoneIndex = SV_DObjGetBoneIndex(ent, tag_name_hash);
    if (BoneIndex < 0)
        return 0;
    G_DObjSetLocalTagInternal_0(vec3_origin, angles, BoneIndex, ent, 0);
    return 1;
}

// ea: 0x004728B0
void G_SetModel(Entity* ent, const char* modelName, TPakId pakId, int ngIndex)
{
    static tlFixedString charHash;  // $S148 one-time init
    static unsigned char s_init = 0;
    if (!(s_init & 1))
    {
        s_init |= 1;
        charHash = tlFixedString("cdChar");
    }
    if (modelName != nullptr)
    {
        TPakId mPakId = pakId;
        if (mPakId == PAK_ID_INVALID)
        {
            mPakId = (TPakId)ent->mPakId;
            if (mPakId == PAK_ID_INVALID)
                mPakId = CurPakId();
        }
        IVPointer<XModel> xmp = XModelManager::sInst->GetXModel(mPakId, modelName);
        if (ngIndex == 0)
        {
            ValidatePakId((TPakId)xmp.mPakId);
            if (xmp.mValue == nullptr)
            {
                AeAssert::gCurrentAuthor = (AeAssert::ECoderId)AeAssert::ARO;
                AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_utils.cpp";
                AeAssert::gCurrentLine = 509;
                AeAssert::gCurrentExpr = "xmp";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert("Unable to get entity's model '%s'", modelName))
                {
                    __debugbreak();
                }
            }
        }
        ValidatePakId((TPakId)xmp.mPakId);
        if (xmp.mValue != nullptr)
        {
            ValidatePakId((TPakId)xmp.mPakId);
            if (xmp.mValue->parts != nullptr)
            {
                ent->mModel.mValue = xmp.mValue;
                ent->mModel.mPakId = xmp.mPakId;
                ValidatePakId((TPakId)xmp.mPakId);
                XModelParts* parts = (XModelParts*)xmp.mValue->parts;
                if (parts->mMeshPtrs.mList != nullptr && parts->mMeshPtrs.mList[0] != nullptr)
                {
                    ValidatePakId((TPakId)xmp.mPakId);
                    unsigned int iflMask = xmp.mValue->iflFrames;
                    if (iflMask != 0)
                    {
                        int v6 = rand() % 32 + 1;
                        int v7 = 31;
                        while (v6 != 0)
                        {
                            if (++v7 >= 32)
                                v7 -= 32;
                            if ((iflMask >> v7) & 1)
                                --v6;
                        }
                        if (!((iflMask >> v7) & 1))
                        {
                            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
                            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_utils.cpp";
                            AeAssert::gCurrentLine = 564;
                            AeAssert::gCurrentExpr = "iflFrames.Test(t)";
                            if (!AeAssert::IsIgnored() && AeAssert::Assert("sanity check"))
                                __debugbreak();
                        }
                        ent->GetRenderEntity().iflIndex = (uint8_t)v7;
                    }
                }
            }
        }
    }
    else
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)AeAssert::JRS;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_utils.cpp";
        AeAssert::gCurrentLine = 500;
        AeAssert::gCurrentExpr = "modelName";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("G_SetModel: bad model name"))
            __debugbreak();
    }
}

// ============================================================================
// XModel children propagation (g.o: g_utils.cpp)
// ============================================================================

static XModelParts* FirstValidModelParts(XModel* xm)
{
    int lodIdx = 0;
    while (xm->lod[lodIdx] == nullptr)
        ++lodIdx;
    XModelLod* lod = xm->lod[lodIdx];
    if (lod == nullptr || lod->xmodelParts == nullptr)
        return nullptr;
    return lod->xmodelParts;
}

// Gram-Schmidt orthonormalization of the 3x3 rotation rows.
// Mirrors math::Mat43 orthonormalize (phys_xboxr inline @ runtime 0xC84200).
static void OrthonormalizeDObjSkel(DObjSkelMat* m)
{
    float* x = m->axis[0];
    float* y = m->axis[1];
    float* z = m->axis[2];

    float len = sqrtf(x[0] * x[0] + x[1] * x[1] + x[2] * x[2]);
    float inv = 1.0f / len;
    x[0] *= inv;
    x[1] *= inv;
    x[2] *= inv;

    float dot = y[0] * x[0] + y[1] * x[1] + y[2] * x[2];
    y[0] -= dot * x[0];
    y[1] -= dot * x[1];
    y[2] -= dot * x[2];
    len = sqrtf(y[0] * y[0] + y[1] * y[1] + y[2] * y[2]);
    inv = 1.0f / len;
    y[0] *= inv;
    y[1] *= inv;
    y[2] *= inv;

    z[0] = x[1] * y[2] - x[2] * y[1];
    z[1] = x[2] * y[0] - x[0] * y[2];
    z[2] = x[0] * y[1] - x[1] * y[0];
}

// Build the inverse of an orthonormal 4x3 row-major matrix:
// rotation transposed, translation negated and rotated.
static void InverseDObjSkel(const DObjSkelMat* in, DObjSkelMat* out)
{
    for (int i = 0; i < 3; ++i)
    {
        for (int j = 0; j < 3; ++j)
            out->axis[i][j] = in->axis[j][i];
        out->axis[i][3] = 0.0f;
    }
    for (int j = 0; j < 3; ++j)
        out->origin[j] = -(in->origin[0] * in->axis[j][0]
                           + in->origin[1] * in->axis[j][1]
                           + in->origin[2] * in->axis[j][2]);
    out->origin[3] = 1.0f;
}

// ea: 0x00464CF0
void XModelChildrenToLocal(IVPointer<XModel> model, DObjSkelMat* matrices, int parent)
{
    ValidatePakId((TPakId)model.mPakId);
    XModel* xm = model.mValue;
    if (xm == nullptr)
        return;
    XModelParts* parts = FirstValidModelParts(xm);
    if (parts == nullptr)
        return;
    InplaceVector<XBoneHierarchy>& hierarchy = parts->mHierarchy;
    int mSize = hierarchy.mSize;

    // DFS over descendants of `parent`, recording pre-order into `bones`.
    // `stack` drives the descent; `bones` accumulates the full queue.
    int bones[45];
    int stack[45];
    int count = 0;
    int stackCount = 0;
    int cur = parent;
    for (;;)
    {
        for (int i = cur + 1; i < mSize; ++i)
        {
            int idx = i < hierarchy.mSize ? i : 0;
            if (hierarchy.mList[idx].mParentIndex == cur)
            {
                if (count < 45)
                    bones[count++] = i;
                else
                {
                    AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
                    AeAssert::gCurrentFile = "../ae\\core/ae_array.h";
                    AeAssert::gCurrentLine = 174;
                    AeAssert::gCurrentExpr = "m_size < _CAPACITY";
                    if (!AeAssert::IsIgnored() && AeAssert::Assert("no room left in array"))
                        __debugbreak();
                }
                if (stackCount < 45)
                    stack[stackCount++] = i;
                else
                {
                    AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
                    AeAssert::gCurrentFile = "../ae\\core/ae_array.h";
                    AeAssert::gCurrentLine = 174;
                    AeAssert::gCurrentExpr = "m_size < _CAPACITY";
                    if (!AeAssert::IsIgnored() && AeAssert::Assert("no room left in array"))
                        __debugbreak();
                }
            }
        }
        if (stackCount <= 0)
            break;
        cur = stack[stackCount - 1];
        --stackCount;
    }

    // Convert each descendant to its parent's local frame, deepest first.
    int cachedParent = -1;
    DObjSkelMat invParent;
    for (int n = count - 1; n >= 0; --n)
    {
        int bone = bones[n];
        int idx = bone < hierarchy.mSize ? bone : 0;
        int boneParent = hierarchy.mList[idx].mParentIndex;
        if (boneParent != cachedParent)
        {
            cachedParent = boneParent;
            InverseDObjSkel(&matrices[boneParent], &invParent);
        }
        matrices[bone] =
            DObjSkelMatrixMultiply(&matrices[bone], &invParent);
    }
}

// ea: 0x00465290
void XModelChildrenToModel(IVPointer<XModel> model, DObjSkelMat* matrices, int parent)
{
    ValidatePakId((TPakId)model.mPakId);
    XModel* xm = model.mValue;
    if (xm == nullptr)
        return;
    XModelParts* parts = FirstValidModelParts(xm);
    if (parts == nullptr)
        return;
    InplaceVector<XBoneHierarchy>& hierarchy = parts->mHierarchy;
    int mSize = hierarchy.mSize;

    int stack[45];
    int stackCount = 0;
    int cur = parent;
    for (;;)
    {
        for (int i = cur + 1; i < mSize; ++i)
        {
            int idx = i < hierarchy.mSize ? i : 0;
            if (hierarchy.mList[idx].mParentIndex == cur)
            {
                if (stackCount < 45)
                {
                    stack[stackCount++] = i;
                }
                else
                {
                    AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
                    AeAssert::gCurrentFile = "../ae\\core/ae_array.h";
                    AeAssert::gCurrentLine = 174;
                    AeAssert::gCurrentExpr = "m_size < _CAPACITY";
                    if (!AeAssert::IsIgnored() && AeAssert::Assert("no room left in array"))
                        __debugbreak();
                }
                matrices[i] =
                    DObjSkelMatrixMultiply(&matrices[i], &matrices[cur]);
                OrthonormalizeDObjSkel(&matrices[i]);
            }
        }
        if (stackCount <= 0)
            break;
        cur = stack[stackCount - 1];
        --stackCount;
    }
}

// ============================================================================
// Local tag setters (g.o: g_utils.cpp file-local helpers)
// ============================================================================

static void Mat43PackedToDObjSkel(const math::Mat43::Packed* in, DObjSkelMat* out)
{
    // Mat43::Packed: three 16-byte rows (x/y/z)
    memcpy(out->axis[0], &in->x, 16);
    memcpy(out->axis[1], &in->y, 16);
    memcpy(out->axis[2], &in->z, 16);
    out->axis[0][3] = 0.0f;
    out->axis[1][3] = 0.0f;
    out->axis[2][3] = 0.0f;
    out->origin[0] = 0.0f;
    out->origin[1] = 0.0f;
    out->origin[2] = 0.0f;
    out->origin[3] = 1.0f;
}

// Apply a local 4x3 tag transform to `bone` of `ent`.
//   a5 != 0: matrices[bone] = local * matrices[bone] (relative to current pose)
//   a5 == 0: matrices[bone] = local * bindPose[bone] (* matrices[parent] when present)
// ea: 0x00472CD0 (file-local)
static void G_DObjSetLocalTagInternal(Entity* ent, int bone, const DObjSkelMat* local,
                                      int a5)
{
    DObj* mDObj = ent->mDObj;
    if (mDObj == nullptr)
        return;
    IVPointer<XModel> model;
    model.mValue = (XModel*)mDObj->models[0].mValue;
    model.mPakId = mDObj->models[0].mPakId;
    ValidatePakId((TPakId)model.mPakId);
    if (model.mValue == nullptr)
        return;
    ValidatePakId((TPakId)model.mPakId);

    int lodIdx = 0;
    while (model.mValue->lod[lodIdx] == nullptr)
        ++lodIdx;
    XModelParts* parts = model.mValue->lod[lodIdx]->xmodelParts;
    if (parts == nullptr)
        return;
    int mParentIndex = -1;
    if (bone < parts->mHierarchy.mSize)
        mParentIndex = parts->mHierarchy.mList[bone].mParentIndex;

    DObjSkelMat* matrices = SV_DObjGetMatrixArray(ent);
    if (matrices == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_utils.cpp";
        AeAssert::gCurrentLine = 1517;
        AeAssert::gCurrentExpr = "matrices";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("no matrices for dobj?"))
            __debugbreak();
        return;
    }
    XModelChildrenToLocal(model, matrices, bone);

    // Compose the new bone matrix.
    if (a5 != 0)
    {
        // Relative to the bone's current matrix.
        matrices[bone] = DObjSkelMatrixMultiply(local, &matrices[bone]);
    }
    else
    {
        // Base is the model's bind-pose transform, composed with the parent's
        // current world matrix when the bone has a parent.
        DObjSkelMat bind;
        math::Mat43::Packed* bindPose = &parts->mTransforms.mList[bone];
        Mat43PackedToDObjSkel(bindPose, &bind);
        DObjSkelMat composed;
        composed = DObjSkelMatrixMultiply(local, &bind);
        if (mParentIndex >= 0)
            matrices[bone] =
                DObjSkelMatrixMultiply(&composed, &matrices[mParentIndex]);
        else
            memcpy(&matrices[bone], &composed, sizeof(composed));
    }

    XModelChildrenToModel(model, matrices, bone);
    if (ent->scr_vehicle != nullptr && mDObj->numModels != 0)
        DObjUpdateChildren(mDObj, bone);
}

// Build a local tag matrix from trans + angles and apply it to bone `bone`.
// ea: 0x004733E0 (file-local)
void G_DObjSetLocalTagInternal_0(const float* trans, const float* angles, int bone,
                                 Entity* ent, int a5)
{
    DObjSkelMat local;
    memset(&local, 0, sizeof(local));
    if (angles != nullptr)
    {
        float axis[3][3];
        AnglesToAxis(angles, axis);
        for (int i = 0; i < 3; ++i)
            for (int j = 0; j < 3; ++j)
                local.axis[i][j] = axis[i][j];
    }
    else
    {
        local.axis[0][0] = 1.0f;
        local.axis[1][1] = 1.0f;
        local.axis[2][2] = 1.0f;
    }
    if (trans != nullptr)
    {
        local.origin[0] = trans[0];
        local.origin[1] = trans[1];
        local.origin[2] = trans[2];
    }
    local.origin[3] = 1.0f;

    G_DObjSetLocalTagInternal(ent, bone, &local, a5);
}

// ============================================================================
// Tag info maintenance (g.o: g_utils.cpp)
// ============================================================================

// ea: 0x00460330
void G_UpdateTagInfo(Entity* ent, int bParentHasDObj)
{
    tagInfo_t* tagInfo = ent->tagInfo;
    if (tagInfo == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_utils.cpp";
        AeAssert::gCurrentLine = 1110;
        AeAssert::gCurrentExpr = "tagInfo";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (tagInfo->name.mHash != 0)
    {
        if (bParentHasDObj == 0)
        {
            G_EntUnlink(ent);
            return;
        }
        const char* v3 = BrocSys::ConvertHashToString(tagInfo->name.mHash);
        unsigned int v4 = HashString::CalcHash(v3);
        int16_t BoneIndex = (int16_t)SV_DObjGetBoneIndex(tagInfo->parent, v4);
        tagInfo->index = BoneIndex;
        if (BoneIndex < 0)
            G_EntUnlink(ent);
    }
    else
    {
        tagInfo->index = -1;
    }
}

// ea: 0x004603D0
void G_UpdateTagInfoOfChildren(Entity* parent, int bHasDObj)
{
    Entity* tagChildren = parent->tagChildren;
    if (tagChildren != nullptr)
    {
        Entity* next = nullptr;
        do
        {
            next = tagChildren->tagInfo->next;
            G_UpdateTagInfo(tagChildren, bHasDObj);
            tagChildren = next;
        } while (next != nullptr);
    }
}

// ea: 0x00464C40
void G_UpdateTags(Entity* ent, int bHasDObj)
{
    if (ent->scr_vehicle != nullptr)
        G_UpdateVehicleTags(ent);
    G_UpdateTagInfoOfChildren(ent, bHasDObj);
}

// ea: 0x00482270
void G_CalcTagParentAxis(Entity* ent, float (*parentAxis)[3])
{
    tagInfo_t* tagInfo = ent->tagInfo;
    if (tagInfo == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_utils.cpp";
        AeAssert::gCurrentLine = 1176;
        AeAssert::gCurrentExpr = "tagInfo";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    Entity* parent = tagInfo->parent;
    if (parent == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_utils.cpp";
        AeAssert::gCurrentLine = 1178;
        AeAssert::gCurrentExpr = "parent";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (tagInfo->index < 0)
    {
        AnglesToAxis(parent->r.currentAngles, parentAxis);
        (*parentAxis)[9] = parent->r.currentOrigin.v.m128_f32[0];
        (*parentAxis)[10] = parent->r.currentOrigin.v.m128_f32[1];
        (*parentAxis)[11] = parent->r.currentOrigin.v.m128_f32[2];
        return;
    }
    float tempAxis[4][3];
    AnglesToAxis(parent->r.currentAngles, tempAxis);
    tempAxis[3][0] = parent->r.currentOrigin.v.m128_f32[0];
    tempAxis[3][1] = parent->r.currentOrigin.v.m128_f32[1];
    tempAxis[3][2] = parent->r.currentOrigin.v.m128_f32[2];
    G_DObjCalcBone(parent, tagInfo->index);
    if (parent->scr_vehicle != nullptr)
        VEH_UpdateControllers(parent, 0);
    DObjSkelMat* MatrixArray = SV_DObjGetMatrixArray(parent);
    DObjSkelMatrixMultiply43(&MatrixArray[tagInfo->index], tempAxis, parentAxis);
}

// ea: 0x004823C0
void G_CalcTagParentRelAxis(Entity* ent, float (*parentRelAxis)[3])
{
    tagInfo_t* tagInfo = ent->tagInfo;
    if (tagInfo == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_utils.cpp";
        AeAssert::gCurrentLine = 1219;
        AeAssert::gCurrentExpr = "tagInfo";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    float parentAxis[4][3];
    G_CalcTagParentAxis(ent, parentAxis);
    MatrixMultiply43((const float(*)[3])tagInfo->parentInvAxis, parentAxis, parentRelAxis);
}

// ea: 0x00482440
void G_CalcTagAxis(Entity* ent, int bAnglesOnly)
{
    float parentAxis[4][3];
    float invParentAxis[4][3];
    float axis[4][3];
    G_CalcTagParentAxis(ent, parentAxis);
    AnglesToAxis(ent->r.currentAngles, axis);
    tagInfo_t* tagInfo = ent->tagInfo;
    if (tagInfo == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_utils.cpp";
        AeAssert::gCurrentLine = 1242;
        AeAssert::gCurrentExpr = "tagInfo";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (bAnglesOnly != 0)
    {
        MatrixTranspose(parentAxis, invParentAxis);
        MatrixMultiply(axis, invParentAxis, tagInfo->axis);
    }
    else
    {
        MatrixInverseOrthogonal43(parentAxis, invParentAxis);
        axis[3][0] = ent->r.currentOrigin.v.m128_f32[0];
        axis[3][1] = ent->r.currentOrigin.v.m128_f32[1];
        axis[3][2] = ent->r.currentOrigin.v.m128_f32[2];
        MatrixMultiply43(axis, invParentAxis, tagInfo->axis);
    }
}

// ============================================================================
// G_DObjUpdate - rebuild an entity's DObj from its model + attachments
// ============================================================================

// Minimal cg.o weapon-info view (iWorldSurfIndex at +0x90)
struct CgWeaponSurf {
    uint8_t     _pad[0x90];
    IVPointerRaw iWorldSurfIndex;  // +0x90
};
struct weaponInfo_s;
extern weaponInfo_s cg_weapons[];  // ?cg_weapons@@3PAUweaponInfo_s@@A (cg.o)
extern bool BG_AllowPlayerWeaponAtVehiclePos(int vehType, int vehPos);
extern IVPointer<XModel> SV_XModelGet(const char* name);

// ea: 0x00472370
void G_DObjUpdate(Entity* ent, bool forceWeaponModel)
{
    DObjModel dobjModels[8];
    memset(dobjModels, 0, sizeof(dobjModels));
    for (int k = 0; k < 8; ++k)
        dobjModels[k].boneName = Broc::string();

    XModel* oldModel = nullptr;
    TPakId oldPakId = PAK_ID_INVALID;
    DObj* mDObj = ent->mDObj;
    if (mDObj != nullptr)
    {
        oldModel = (XModel*)mDObj->models[0].mValue;
        oldPakId = (TPakId)mDObj->models[0].mPakId;
    }
    ent->FreeDObj(false);

    XModel* v7 = ent->mModel.mValue;
    TPakId modelPakId = (TPakId)ent->mModel.mPakId;
    ValidatePakId(modelPakId);
    if (v7 == nullptr)
    {
        if (ent->scr_vehicle != nullptr)
            G_UpdateVehicleTags(ent);
        G_UpdateTagInfoOfChildren(ent, 0);
        return;
    }

    int v8 = ent->s.eType - 11;
    XAnimTree* tree;
    if (v8 != 0)
    {
        if (v8 == 2)
            tree = G_GetActorCorpseAnimTree(ent);
        else
            tree = ent->pAnimTree;
    }
    else
    {
        tree = G_GetActorAnimTree(ent->actor);
    }

    ValidatePakId(oldPakId);
    if (oldModel != nullptr)
    {
        ValidatePakId(modelPakId);
        int v11 = 0;
        while (v7->lod[v11] == nullptr)
            ++v11;
        XModelParts* xmodelParts = v7->lod[v11]->xmodelParts;
        ValidatePakId(oldPakId);
        int v14 = 0;
        while (oldModel->lod[v14] == nullptr)
            ++v14;
        void* mAnimDef = xmodelParts->mAnimDef;
        modelPakId = (TPakId)ent->mModel.mPakId;
        if (mAnimDef != oldModel->lod[v14]->xmodelParts->mAnimDef)
            tree = nullptr;
    }

    ValidatePakId(modelPakId);
    dobjModels[0].model.mValue = v7;
    dobjModels[0].model.mPakId = modelPakId;
    int numModels = 1;

    Client* client = ent->client;
    if (client != nullptr
        && ent->health > 0
        && client->mVehicleNoWeaponTime == 0
        && client->ps.weapon != 0
        && ((0x100000 & client->ps.eFlags) == 0
            || BG_AllowPlayerWeaponAtVehiclePos(client->ps.vehType, client->ps.vehPos)))
    {
        weaponFileInfo_t* InfoForWeapon = BG_GetInfoForWeapon(client->ps.weapon);
        if ((InfoForWeapon->weapClass != 0x0D /* WEAPCLASS_AMMO */
             || client->ps.mAmmoDropTime + 2000 <= level.time)
            && ((CgWeaponSurf*)cg_weapons)[client->ps.weapon].iWorldSurfIndex.mValue != nullptr)
        {
            IVPointer<XModel> svx = SV_XModelGet(InfoForWeapon->szWorldModel);
            dobjModels[1].model.mValue = svx.mValue;
            dobjModels[1].model.mPakId = svx.mPakId;
            dobjModels[1].boneName = Broc::string("TAG_WEAPON_RIGHT");
            dobjModels[1].ignoreCollision = 0;
            numModels = 2;
        }
    }
    else if (forceWeaponModel)
    {
        weaponFileInfo_t* v21 = BG_GetInfoForWeapon(ent->s.weapon);
        ValidatePakId((TPakId)((CgWeaponSurf*)cg_weapons)[ent->s.weapon].iWorldSurfIndex.mPakId);
        if (((CgWeaponSurf*)cg_weapons)[ent->s.weapon].iWorldSurfIndex.mValue != nullptr)
        {
            IVPointer<XModel> svx = SV_XModelGet(v21->szWorldModel);
            dobjModels[1].model.mValue = svx.mValue;
            dobjModels[1].model.mPakId = svx.mPakId;
            dobjModels[1].boneName = Broc::string("TAG_WEAPON_RIGHT");
            dobjModels[1].ignoreCollision = 0;
            numModels = 2;
        }
    }

    for (int i = 0; i < 7; ++i)
    {
        AttachModelInfo* mAttachModels = &ent->mAttachModels[i];
        ValidatePakId((TPakId)mAttachModels->mModel.mPakId);
        if (mAttachModels->mModel.mValue != nullptr)
        {
            if (numModels >= 8)
            {
                AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
                AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_utils.cpp";
                AeAssert::gCurrentLine = 456;
                AeAssert::gCurrentExpr = "numModels < DOBJ_MAX_SUBMODELS";
                if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
                    __debugbreak();
            }
            DObjModel* slot = &dobjModels[numModels];
            slot->model.mValue = mAttachModels->mModel.mValue;
            slot->model.mPakId = mAttachModels->mModel.mPakId;
            ValidatePakId((TPakId)slot->model.mPakId);
            if (slot->model.mValue == nullptr)
            {
                AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
                AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_utils.cpp";
                AeAssert::gCurrentLine = 458;
                AeAssert::gCurrentExpr = "dobjModels[numModels].model";
                if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
                    __debugbreak();
            }
            slot->boneName = mAttachModels->mTag;
            slot->ignoreCollision = ((1 << i) & ent->attachIgnoreCollision) != 0;
            ++numModels;
        }
    }

    unsigned short gameId = (unsigned short)ent->mHandle.mHandle.mVal;
    ent->CreateDObj(dobjModels, (unsigned short)numModels, tree, gameId);
    if (ent->scr_vehicle != nullptr)
        G_UpdateVehicleTags(ent);

    Entity* tagChildren = ent->tagChildren;
    while (tagChildren != nullptr)
    {
        tagInfo_t* tagInfo = tagChildren->tagInfo;
        Entity* next = tagInfo->next;
        if (tagInfo == nullptr)
        {
            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_utils.cpp";
            AeAssert::gCurrentLine = 1110;
            AeAssert::gCurrentExpr = "tagInfo";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
                __debugbreak();
        }
        if (tagInfo->name.mHash != 0)
        {
            const char* v30 = BrocSys::ConvertHashToString(tagInfo->name.mHash);
            unsigned int v31 = HashString::CalcHash(v30);
            int16_t BoneIndex = (int16_t)SV_DObjGetBoneIndex(tagInfo->parent, v31);
            tagInfo->index = BoneIndex;
            if (BoneIndex < 0)
                G_EntUnlink(tagChildren);
        }
        else
        {
            tagInfo->index = -1;
        }
        tagChildren = next;
    }
}

// ============================================================================
// Entity tag linking (g.o: g_utils.cpp)
// ============================================================================

PoolAllocator* tagInfo_t::sAllocator = nullptr;  // defined by g_globals/init

// ea: 0x004DD5E0
void tagInfo_t::SetAllocator(PoolAllocator* allocator)
{
    tagInfo_t::sAllocator = allocator;
}

// tagInfo_t memory ops (g.o 0x4A7760-0x4A77A0)
void* tagInfo_t::operator new(size_t size, bool forceHeapAlloc,
                              const char* /*file*/, int /*line*/)
{
    return tagInfo_t::sAllocator->Allocate((unsigned int)size, forceHeapAlloc);
}
void tagInfo_t::operator delete(void* ptr, bool /*forceHeapAlloc*/,
                                const char* /*file*/, int /*line*/)
{
    tagInfo_t::sAllocator->Release(ptr);
}
void tagInfo_t::operator delete(void* ptr)
{
    tagInfo_t::sAllocator->Release(ptr);
}

static tagInfo_t* AllocTagInfo(Entity* parent, Entity* ent, unsigned int tagHash,
                               int index, bool useAngles)
{
    tagInfo_t* v8 = (tagInfo_t*)tagInfo_t::sAllocator->Allocate(0x70, false);
    v8->name.mHash = tagHash;
    v8->parent = parent;
    v8->index = (int16_t)index;
    v8->useAngles = useAngles ? 1 : 0;
    v8->next = parent->tagChildren;
    memset(v8->axis, 0, sizeof(v8->axis));
    parent->tagChildren = ent;
    ent->tagInfo = v8;
    if (ent->scripted != nullptr || v8->useAngles != 0)
    {
        float axis[4][3];
        G_CalcTagParentAxis(ent, axis);
        MatrixInverseOrthogonal43(axis, v8->parentInvAxis);
    }
    else
    {
        memset(v8->parentInvAxis, 0, sizeof(v8->parentInvAxis));
    }
    if (ent->client != nullptr)
    {
        scr_vehicle_t* scr_vehicle = parent->scr_vehicle;
        if (scr_vehicle != nullptr)
            ++scr_vehicle->playersAttached;
    }
    return v8;
}

// ea: 0x0048AB20 (file-local)
static int G_EntLinkToInternal(Entity* ent, Entity* parent, unsigned int tag_name_hash,
                               bool useAngles)
{
    if (parent == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_utils.cpp";
        AeAssert::gCurrentLine = 749;
        AeAssert::gCurrentExpr = "parent";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if ((ent->flags & 0x8000) == 0)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_utils.cpp";
        AeAssert::gCurrentLine = 751;
        AeAssert::gCurrentExpr = "ent->flags & 0x00008000";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    G_EntUnlink(ent);
    if (ent->tagInfo != nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_utils.cpp";
        AeAssert::gCurrentLine = 755;
        AeAssert::gCurrentExpr = "!ent->tagInfo";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (parent->mDObj != nullptr)
    {
        int index = SV_DObjGetBoneIndex(parent, tag_name_hash);
        if (index >= 0)
        {
            for (Entity* i = parent; ; i = i->tagInfo->parent)
            {
                if (i == nullptr)
                {
                    AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
                    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_utils.cpp";
                    AeAssert::gCurrentLine = 767;
                    AeAssert::gCurrentExpr = "checkEnt";
                    if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
                        __debugbreak();
                }
                if (i == ent)
                    break;
                if (i->tagInfo == nullptr)
                {
                    AllocTagInfo(parent, ent, tag_name_hash, index, useAngles);
                    return 1;
                }
            }
        }
    }
    return 0;
}

// ea: 0x0048AD50 (file-local)
static int G_EntLinkToInternal_0(Entity* ent, Entity* parent, const char* tagName,
                                 bool useAngles)
{
    if (parent == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_utils.cpp";
        AeAssert::gCurrentLine = 815;
        AeAssert::gCurrentExpr = "parent";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (tagName == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_utils.cpp";
        AeAssert::gCurrentLine = 816;
        AeAssert::gCurrentExpr = "tagName";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if ((ent->flags & 0x8000) == 0)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_utils.cpp";
        AeAssert::gCurrentLine = 818;
        AeAssert::gCurrentExpr = "ent->flags & 0x00008000";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    G_EntUnlink(ent);
    if (ent->tagInfo != nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_utils.cpp";
        AeAssert::gCurrentLine = 822;
        AeAssert::gCurrentExpr = "!ent->tagInfo";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    int index;
    unsigned int tagHash = 0;
    if (tagName != nullptr && *tagName != 0)
    {
        if (parent->mDObj == nullptr)
            return 0;
        tagHash = HashString::CalcHash(tagName);
        index = SV_DObjGetBoneIndex(parent, tagHash);
        if (index < 0)
            return 0;
    }
    else
    {
        index = -1;
    }
    for (Entity* i = parent; ; i = i->tagInfo->parent)
    {
        if (i == nullptr)
        {
            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_utils.cpp";
            AeAssert::gCurrentLine = 842;
            AeAssert::gCurrentExpr = "checkEnt";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
                __debugbreak();
        }
        if (i == ent)
            break;
        if (i->tagInfo == nullptr)
        {
            AllocTagInfo(parent, ent, tagHash, index, useAngles);
            return 1;
        }
    }
    return 0;
}

// ea: 0x0048AFF0
int G_EntLinkTo(Entity* ent, Entity* parent, const char* tagName)
{
    int result = G_EntLinkToInternal_0(ent, parent, tagName, false);
    if (result != 0)
    {
        G_CalcTagAxis(ent, 0);
        return 1;
    }
    return result;
}

// ea: 0x0048B030
int G_EntLinkTo(Entity* ent, Entity* parent, unsigned int tag_name_hash)
{
    int result = G_EntLinkToInternal(ent, parent, tag_name_hash, false);
    if (result != 0)
    {
        G_CalcTagAxis(ent, 0);
        return 1;
    }
    return result;
}

// ea: 0x0048B070
int G_EntLinkToWithOffsetHash(Entity* ent, Entity* parent, unsigned int tag_name_hash,
                              const float* originOffset, const float* anglesOffset,
                              bool useAngles)
{
    int result = G_EntLinkToInternal(ent, parent, tag_name_hash, useAngles);
    if (result != 0)
    {
        tagInfo_t* tagInfo = ent->tagInfo;
        if (IS_NAN(anglesOffset[0]) || IS_NAN(anglesOffset[1]) || IS_NAN(anglesOffset[2]))
        {
            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_utils.cpp";
            AeAssert::gCurrentLine = 949;
            AeAssert::gCurrentExpr = "!IS_NAN((anglesOffset)[0]) && !IS_NAN((anglesOffset)[1]) && !IS_NAN((anglesOffset)[2])";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid vector"))
                __debugbreak();
        }
        AnglesToAxis(anglesOffset, tagInfo->axis);
        tagInfo->axis[3][0] = originOffset[0];
        tagInfo->axis[3][1] = originOffset[1];
        tagInfo->axis[3][2] = originOffset[2];
        return 1;
    }
    return result;
}

// ea: 0x0048B160
int G_EntLinkToWithOffset(Entity* ent, Entity* parent, const char* tagName,
                          const float* originOffset, const float* anglesOffset,
                          bool useAngles)
{
    int result = G_EntLinkToInternal_0(ent, parent, tagName, useAngles);
    if (result != 0)
    {
        tagInfo_t* tagInfo = ent->tagInfo;
        if (IS_NAN(anglesOffset[0]) || IS_NAN(anglesOffset[1]) || IS_NAN(anglesOffset[2]))
        {
            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_utils.cpp";
            AeAssert::gCurrentLine = 977;
            AeAssert::gCurrentExpr = "!IS_NAN((anglesOffset)[0]) && !IS_NAN((anglesOffset)[1]) && !IS_NAN((anglesOffset)[2])";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid vector"))
                __debugbreak();
        }
        AnglesToAxis(anglesOffset, tagInfo->axis);
        tagInfo->axis[3][0] = originOffset[0];
        tagInfo->axis[3][1] = originOffset[1];
        tagInfo->axis[3][2] = originOffset[2];
        return 1;
    }
    return result;
}
