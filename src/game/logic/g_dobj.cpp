// ============================================================================
// g_dobj.cpp - DObj/XAnim script wrappers + anim-tree helpers (g.o)
// Source: g_syscalls.cpp / g_utils.cpp / g_client.cpp families
// ============================================================================

#include "game/logic/g_local.h"

#include <stdlib.h>
#include <string.h>

#include "core/tlFixedString.h"

extern void G_RmvInvalidatedNode(Entity* pEnt, int iRmv);
extern void G_DObjSetLocalTagInternal(int bone);  // file-local g_utils.cpp helper

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
    AnglesToAxis(&ent->r.currentAngles, (float(*)[3])ent_axis);
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
            G_RmvInvalidatedNode(pEnt, InvalidatedNode);
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
    // reserve(192) / reserve(192) / reserve(128) - capacity preallocation
    dobjects.mCapacity = 192;
    del_pending_dobjects.mCapacity = 192;
    add_pending_dobjects.mCapacity = 128;
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
        Client* v8 = &level.clients[i];
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
int G_DObjSetLocalBoneIndex(Entity* /*ent*/, int* /*partBits*/, int boneIndex,
                            const float* trans, const float* angles, bool /*bSomething*/)
{
    G_DObjSetLocalTagInternal_0(trans, angles, boneIndex);
    return 1;
}

// ea: 0x00473610
int G_DObjSetLocalBoneIndex(Entity* /*ent*/, int* /*partBits*/, int boneIndex,
                            const math::Position3& /*trans*/, const math::Mat33& /*angles*/,
                            bool /*bSomething*/)
{
    G_DObjSetLocalTagInternal(boneIndex);
    return 1;
}

// ea: 0x00473640
int G_DObjSetLocalTag(Entity* ent, int* /*partBits*/, unsigned int tag_name_hash,
                      const float* trans, const float* angles, bool /*bSomething*/)
{
    int BoneIndex = SV_DObjGetBoneIndex(ent, tag_name_hash);
    if (BoneIndex < 0)
        return 0;
    G_DObjSetLocalTagInternal_0(trans, angles, BoneIndex);
    return 1;
}

// ea: 0x00473680
int G_DObjSetControlTagAngles(Entity* ent, int* /*partBits*/, unsigned int tag_name_hash,
                              float* angles)
{
    int BoneIndex = SV_DObjGetBoneIndex(ent, tag_name_hash);
    if (BoneIndex < 0)
        return 0;
    G_DObjSetLocalTagInternal_0(vec3_origin, angles, BoneIndex);
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
