// ============================================================================
// sv_game.cpp — server <-> game VM bridge + DObj wrappers (sv_game.cpp of sv.o)
// ============================================================================

#include "game/sv/sv_decl.h"
#include "game/sv/sv_stubs.h"

#include <stdio.h>
#include <string.h>

// ============================================================================
// Cross-object externs
// ============================================================================
extern void  Q_strncpyz(char* dest, const char* src, int destsize);
extern void  Com_Memset(unsigned int* dest, int val, unsigned int count);
extern const usercmd_s& CL_GetCurUserCmd(int clientNum);
extern void  CL_SetUsercmdButtonsWeapons(int buttons, int weapon);
extern void* mem_heap_malloc_ctx(unsigned int size, int alignment, const char* ctx, const char* file, int line);
extern int   com_skelTimeStamp;
extern cvar_t* com_developer;
extern const char* nullStr;

// DObj API (animation lib)
class DObj;
struct DObjSkelMat;
class XAnimTree;

// Minimal DObj view with tree[] (full class in g_local.h/cg_local.h)
class DObj {
public:
    void* tree[8];  // +0x00 XAnimTree*[8]
    uint8_t _pad[0xC0 - 0x20];
    int      mPakId;         // +0xC0 (TPakId)
    uint8_t _padC4[0xCE - 0xC4];
    unsigned char numModels; // +0xCE
    uint8_t _padCF[0xD0 - 0xCF];
    void*    mEntity;        // +0xD0 Entity*
};

// ea: 0x006BE190 (render.o)
XAnimTree* DObjGetTree(DObj* obj)  // ?DObjGetTree@@YAPAVXAnimTree@@PAVDObj@@@Z
{
    return (XAnimTree*)obj->tree[0];
}

struct XAnimEntry {
    unsigned int hash;
    unsigned short numAnims;
    unsigned short parent;
    void* anim;
    void* notify;
    int lastAttempt;
    unsigned char lastChosenChild;
    unsigned char pad[3];
    struct { unsigned short flags; unsigned short children; } sync;
};
struct XAnimInfo {
    unsigned short notifyChild;
    short notifyIndex;
    unsigned int notifyName;
    unsigned short notifyType;
    unsigned short prev;
    unsigned short next;
    unsigned char s[36];
    void* pEntity;
};
extern XAnimInfo g_info[512];
extern void XAnimUpdateServerNotify(XAnimTree* tree, unsigned int animIndex);
Entity* gGameEnt = nullptr;  // ?gGameEnt@@3PAVEntity@@A (anim.o @ 0xF25A2C)

static XAnimEntry* ServerAnimEntry(XAnimTree* tree, unsigned int index)
{
    auto* anims = *reinterpret_cast<unsigned char**>(
        reinterpret_cast<unsigned char*>(tree) + 8);
    if (anims == nullptr)
        return nullptr;
    const unsigned int count = *reinterpret_cast<unsigned int*>(anims + 4);
    if (index >= count)
        return nullptr;
    return reinterpret_cast<XAnimEntry*>(
        *reinterpret_cast<unsigned char**>(anims + 8)
        + index * sizeof(XAnimEntry));
}

static float ServerAnimRateFrequency(XAnimTree* tree, unsigned int index)
{
    XAnimEntry* entry = ServerAnimEntry(tree, index);
    if (entry == nullptr)
        return 1.0f;
    if (entry->numAnims == 0)
    {
        if (entry->anim == nullptr)
            return 1.0f;
        const float duration = *reinterpret_cast<float*>(
            reinterpret_cast<unsigned char*>(entry->anim) + 0x38);
        return duration != 0.0f ? 1.0f / duration : 0.0f;
    }
    float totalWeight = 0.0f;
    float weightedRate = 0.0f;
    for (unsigned int i = 0; i < entry->numAnims; ++i)
    {
        const unsigned int child = entry->sync.children + i;
        const unsigned short infoIndex =
            *reinterpret_cast<unsigned short*>(
                reinterpret_cast<unsigned char*>(tree) + 0x18 + 2 * child);
        if (infoIndex == 0 || infoIndex >= 512)
            continue;
        const float weight = *reinterpret_cast<float*>(g_info[infoIndex].s + 0x14);
        if (weight <= 0.0f)
            continue;
        totalWeight += weight;
        weightedRate += ServerAnimRateFrequency(tree, child) * weight * weight;
    }
    return totalWeight != 0.0f ? weightedRate / totalWeight : 0.0f;
}

static void ServerAnimCheckNoteTrack(XAnimTree* tree, unsigned int animIndex,
                                     float dtime)
{
    XAnimEntry* entry = ServerAnimEntry(tree, animIndex);
    if (entry == nullptr)
        return;
    const unsigned short infoIndex = *reinterpret_cast<unsigned short*>(
        reinterpret_cast<unsigned char*>(tree) + 0x18 + 2 * animIndex);
    if (infoIndex == 0 || infoIndex >= 512)
        return;
    XAnimInfo& info = g_info[infoIndex];
    const float weight = *reinterpret_cast<float*>(info.s + 0x14);
    if (weight == 0.0f)
        return;
    if (entry->numAnims != 0)
    {
        for (unsigned int i = 0; i < entry->numAnims; ++i)
            ServerAnimCheckNoteTrack(tree, entry->sync.children + i, dtime);
        return;
    }
    const float rate = *reinterpret_cast<float*>(info.s + 0x18)
                     * ServerAnimRateFrequency(tree, animIndex) * dtime;
    if (rate == 0.0f)
        return;
    const float oldTime = *reinterpret_cast<float*>(info.s + 0x04);
    float time = oldTime + rate;
    const bool looping = entry->anim != nullptr
        && ((*reinterpret_cast<unsigned char*>(
                 reinterpret_cast<unsigned char*>(entry->anim) + 0x34) & 1)
            != 0);
    if (looping)
    {
        time -= static_cast<float>(static_cast<int>(time));
        if (time < 0.0f)
            time += 1.0f;
    }
    else if (time > 1.0f)
        time = 1.0f;
    *reinterpret_cast<float*>(info.s + 0x00) = time;
    XAnimUpdateServerNotify(tree, animIndex);
}

void XAnimFindServerNoteTrack(XAnimTree* tree, unsigned int animIndex,
                              float dtime)  // ?XAnimFindServerNoteTrack (anim.o 0x541CD0)
{
    ServerAnimCheckNoteTrack(tree, animIndex, dtime);
}
void XAnimUpdateServerInfoInternal(XAnimTree* tree, unsigned int animIndex,
                                   float dtime, bool bNotify)
{
    XAnimEntry* entry = ServerAnimEntry(tree, animIndex);
    if (entry == nullptr)
        return;
    const unsigned short infoIndex = *reinterpret_cast<unsigned short*>(
        reinterpret_cast<unsigned char*>(tree) + 0x18 + 2 * animIndex);
    if (infoIndex == 0 || infoIndex >= 512)
        return;
    XAnimInfo& info = g_info[infoIndex];
    if (*reinterpret_cast<float*>(info.s + 0x14) == 0.0f)
        return;
    if (entry->numAnims != 0)
    {
        for (unsigned int i = 0; i < entry->numAnims; ++i)
            XAnimUpdateServerInfoInternal(tree, entry->sync.children + i,
                                          dtime, bNotify);
        return;
    }
    const float oldTime = *reinterpret_cast<float*>(info.s + 0x04);
    const float rate = *reinterpret_cast<float*>(info.s + 0x18)
                     * ServerAnimRateFrequency(tree, animIndex) * dtime;
    float time = oldTime + rate;
    const bool looping = entry->anim != nullptr
        && ((*reinterpret_cast<unsigned char*>(
                 reinterpret_cast<unsigned char*>(entry->anim) + 0x34) & 1)
            != 0);
    if (looping)
    {
        if (time < 0.0f)
            time += 1.0f - static_cast<float>(static_cast<int>(-time));
        if (time >= 1.0f)
            time -= static_cast<float>(static_cast<int>(time));
    }
    else
    {
        if (time < 0.0f)
            time = 0.0f;
        if (time > 1.0f)
            time = 1.0f;
    }
    *reinterpret_cast<float*>(info.s + 0x00) = time;
    if (bNotify)
        XAnimUpdateServerNotify(tree, animIndex);
}

// ea: 0x00552C20 (anim.o)
bool DObjUpdateServerInfo(DObj* obj, float dtime, bool bNotify,
                          int animindex)  // ?DObjUpdateServerInfo@@YA_NPAVDObj@@M_NH@Z
{
    if (dtime < 0.0f)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\xanim.cpp";
        AeAssert::gCurrentLine = 2934;
        AeAssert::gCurrentExpr = "dtime >= 0";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (obj->tree[0] == nullptr)
        return false;
    gGameEnt = (Entity*)obj->mEntity;
    unsigned char numModels = obj->numModels;
    char v10 = 0;
    for (int v8 = 0; v8 < numModels; ++v8)
    {
        XAnimTree* v9 = (XAnimTree*)obj->tree[v8];
        if (v9 != nullptr)
        {
            if (bNotify)
            {
                XAnimFindServerNoteTrack(v9, animindex, dtime);
                if (dtime != 1.0f)
                {
                    float v4 = dtime * dtime + 0.001f;
                    if (dtime >= v4)
                    {
                        XAnimUpdateServerInfoInternal(
                            (XAnimTree*)obj->tree[v8], animindex, v4, true);
                        v10 = 1;
                    }
                    continue;
                }
                XAnimUpdateServerInfoInternal(
                    (XAnimTree*)obj->tree[v8], animindex, dtime, true);
            }
            else
            {
                XAnimUpdateServerInfoInternal(v9, animindex, dtime, false);
            }
            v10 = 0;
        }
    }
    return v10 != 0;
}
class XAnimTree;
extern void  DObjDumpInfo(DObj* obj);
extern int   DObjSkelExists(DObj* obj, int timeStamp);
extern int   DObjSkelExistsConst(DObj* obj, int timeStamp);
extern unsigned int DObjGetAllocSkelSize(DObj* obj);  // ?DObjGetAllocSkelSize@@YAIPAVDObj@@@Z
extern void  DObjCreateSkel(DObj* obj, char* buf);
extern bool  DObjUpdateServerInfo(DObj* obj, float dtime, bool bNotify,
int animindex);  // ?DObjUpdateServerInfo@@YA_NPAVDObj@@M_NH@Z
extern void  DObjInitServerTime(DObj* d, float dtime);
extern void  DObjGetHierarchyBits(DObj* obj, int boneIndex, int* const partBits);
extern void  DObjCalcAnim(DObj* obj, int iPhase);
extern void  j_nullsub_82(DObj* obj, int* partBits);
extern int   DObjNumBones(DObj* obj);
extern int   DObjGetBoneIndex(const DObj* obj, unsigned int boneNameHash);
extern DObjSkelMat* DObjGetMatrixArray(const DObj* obj, int modelIndex);
extern void  DObjDisplayAnim(DObj* obj);
extern void  DObjDisplayAnim3D(int id, DObj* obj, float* const loc, int line);
extern XAnimTree* DObjGetTree(DObj* obj);

// File / model
struct fileData_s {
    char* name;                // +0x00
    void* data;                // +0x04
    void (*mem_heap_free)(fileData_s*);  // +0x08
};
extern fileData_s* FS_GetDataForFile(const char* path, const char* filename, const char* extension);
extern TPakId CurPakId(void);  // ?CurPakId@@YA?AW4TPakId@@XZ
extern void SV_SendServerCommand(client_s* cl, const char* fmt, ...);

// ============================================================================
// SV_GameSystemCalls — ea: 0x51F190
// ============================================================================
int SV_GameSystemCalls(int* args) {
    Com_Error((errorParm_t)2, "\x15" "Bad game system trap: %i", *args);
    return -1;
}

// ============================================================================
// SV_FX_Save — ea: 0x51F170
// ============================================================================
int SV_FX_Save() {
    return 0;
}

// ============================================================================
// SV_SaveGameMessages — ea: 0x51F180
// ============================================================================
int SV_SaveGameMessages() {
    return 0;
}

// ============================================================================
// SV_GameCommand — ea: 0x51F1E0
// ============================================================================
int SV_GameCommand() {
    if (sv.state == SS_GAME)
        return VM_Call(gvm, 13);
    else
        return 0;
}

// ============================================================================
// SV_ShutdownGameProgs — ea: 0x51F1B0
// ============================================================================
void SV_ShutdownGameProgs() {
    if (gvm != NULL) {
        VM_Call(gvm, 1, 0);
        VM_Free(gvm);
        gvm = NULL;
    }
}

// ============================================================================
// SV_SaveWrite — ea: 0x51EBC0
// ============================================================================
void* SV_SaveWrite(const void* buffer, int len) {
    void* result = mem_heap_malloc_ctx(len, 16, "hunk", "c:\\cod\\code\\game\\sv_game.cpp", 622);
    memcpy(result, buffer, len);
    return result;
}

// ============================================================================
// SV_XModelGet — ea: 0x51EC00
// ============================================================================
IVPointer<XModel> SV_XModelGet(const char* name) {
    XModelManager* v2 = XModelManager::sInst;
    int v3 = (int)CurPakId();
    return v2->GetXModel((TPakId)v3, name);
}

// ============================================================================
// SV_DObjDumpInfo — ea: 0x51EC30
// ============================================================================
void SV_DObjDumpInfo(Entity* entity) {
    if (com_developer->integer != 0) {
        if (entity->mDObj != NULL)
            DObjDumpInfo(entity->mDObj);
        else
            Com_Printf("no model.\n");
    }
}

// ============================================================================
// SV_DObjCreateSkelForBone — ea: 0x51EC70
// ============================================================================
bool SV_DObjCreateSkelForBone(Entity* entity, int boneIndex) {
    (void)boneIndex;
    if (entity->mDObj == NULL) {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sv_game.cpp";
        AeAssert::gCurrentLine = 680;
        AeAssert::gCurrentExpr = "entity->GetDObj()";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    DObj* mDObj = entity->mDObj;
    if (DObjSkelExists(mDObj, com_skelTimeStamp) != 0)
        return true;
    unsigned int AllocSkelSize = DObjGetAllocSkelSize(mDObj);
    char* v4 = (char*)mem_heap_malloc_ctx(AllocSkelSize, 16, "hunk", "c:\\cod\\code\\game\\sv_game.cpp", 689);
    DObjCreateSkel(mDObj, v4);
    return false;
}

// ============================================================================
// SV_DObjCreateSkelForBones — ea: 0x51ED10
// ============================================================================
bool SV_DObjCreateSkelForBones(Entity* entity) {
    if (entity != NULL) {
        if (entity->mDObj == NULL) {
            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sv_game.cpp";
            AeAssert::gCurrentLine = 709;
            AeAssert::gCurrentExpr = "entity->GetDObj()";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
                __debugbreak();
        }
        DObj* mDObj = entity->mDObj;
        if (DObjSkelExists(mDObj, com_skelTimeStamp) != 0)
            return true;
        unsigned int AllocSkelSize = DObjGetAllocSkelSize(mDObj);
        char* v4 = (char*)mem_heap_malloc_ctx(AllocSkelSize, 16, "hunk", "c:\\cod\\code\\game\\sv_game.cpp", 718);
        DObjCreateSkel(mDObj, v4);
    }
    return false;
}

// ============================================================================
// SV_DObjUpdateServerTime — ea: 0x51EDC0
// ============================================================================
bool SV_DObjUpdateServerTime(Entity* entity, float dtime, bool bNotify) {
    DObj* mDObj = entity->mDObj;
    return mDObj != NULL && DObjUpdateServerInfo(mDObj, dtime, bNotify, 0);
}

// ============================================================================
// SV_DObjInitServerTime — ea: 0x51EDF0
// ============================================================================
void SV_DObjInitServerTime(Entity* entity, float dtime) {
    DObj* mDObj = entity->mDObj;
    if (mDObj != NULL)
        DObjInitServerTime(mDObj, dtime);
}

// ============================================================================
// SV_DObjGetHierarchyBits — ea: 0x51EE10
// ============================================================================
void SV_DObjGetHierarchyBits(Entity* entity, int boneIndex, int* const partBits) {
    if (entity->mDObj == NULL) {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sv_game.cpp";
        AeAssert::gCurrentLine = 762;
        AeAssert::gCurrentExpr = "entity->GetDObj()";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    DObjGetHierarchyBits(entity->mDObj, boneIndex, partBits);
}

// ============================================================================
// SV_DObjCalcAnim — ea: 0x51EE80
// ============================================================================
void SV_DObjCalcAnim(Entity* entity, int iPhase) {
    DObj* mDObj = entity->mDObj;
    if (mDObj != NULL)
        DObjCalcAnim(mDObj, iPhase);
}

// ============================================================================
// SV_DObjCalcSkel — ea: 0x51EEA0
// ============================================================================
void SV_DObjCalcSkel(Entity* entity, int* const partBits) {
    if (entity->mDObj == NULL) {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sv_game.cpp";
        AeAssert::gCurrentLine = 791;
        AeAssert::gCurrentExpr = "entity->GetDObj()";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (DObjSkelExistsConst(entity->mDObj, com_skelTimeStamp) == 0) {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sv_game.cpp";
        AeAssert::gCurrentLine = 792;
        AeAssert::gCurrentExpr = "DObjSkelExistsConst(entity->GetDObj(), com_skelTimeStamp)";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    j_nullsub_82(entity->mDObj, partBits);
}

// ============================================================================
// SV_DObjNumBones — ea: 0x51EF70
// ============================================================================
int SV_DObjNumBones(Entity* entity) {
    if (entity->mDObj == NULL) {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sv_game.cpp";
        AeAssert::gCurrentLine = 805;
        AeAssert::gCurrentExpr = "entity->GetDObj()";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    return DObjNumBones(entity->mDObj);
}

// ============================================================================
// SV_DObjGetBoneIndex — ea: 0x51EFE0
// ============================================================================
int SV_DObjGetBoneIndex(Entity* entity, unsigned int boneNameHash) {
    DObj* mDObj = entity->mDObj;
    if (mDObj != NULL)
        return DObjGetBoneIndex(mDObj, boneNameHash);
    else
        return -1;
}

// ============================================================================
// SV_DObjGetMatrixArray — ea: 0x51F010
// ============================================================================
DObjSkelMat* SV_DObjGetMatrixArray(Entity* entity) {
    if (entity->mDObj == NULL) {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sv_game.cpp";
        AeAssert::gCurrentLine = 823;
        AeAssert::gCurrentExpr = "entity->GetDObj()";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    return DObjGetMatrixArray(entity->mDObj, 0);
}

// ============================================================================
// SV_DObjDisplayAnim — ea: 0x51F080
// ============================================================================
void SV_DObjDisplayAnim(Entity* entity) {
    if (entity->mDObj != NULL)
        DObjDisplayAnim(entity->mDObj);
}

// ============================================================================
// SV_DObjDisplayAnim3D — ea: 0x51F0A0
// ============================================================================
void SV_DObjDisplayAnim3D(Entity* entity, float* const loc, int line) {
    DObj* mDObj = entity->mDObj;
    if (mDObj != NULL)
        DObjDisplayAnim3D(entity->mHandle.mHandle.mVal, mDObj, loc, line);
}

// ============================================================================
// SV_DObjGetTree — ea: 0x51F0D0
// ============================================================================
XAnimTree* SV_DObjGetTree(Entity* entity) {
    if (entity->mDObj == NULL) {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sv_game.cpp";
        AeAssert::gCurrentLine = 867;
        AeAssert::gCurrentExpr = "entity->GetDObj()";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    return DObjGetTree(entity->mDObj);
}

// ============================================================================
// SV_GetDataForFile — ea: 0x51F140
// ============================================================================
void* SV_GetDataForFile(const char* path, const char* filename, const char* extension) {
    fileData_s* result = FS_GetDataForFile(path, filename, extension);
    if (result != NULL)
        return result->data;
    return result;
}

// ============================================================================
// SV_GetServerinfo — ea: 0x51EB20
// ============================================================================
void SV_GetServerinfo(char* buffer, int bufferSize) {
    if (bufferSize < 1)
        Com_Error((errorParm_t)2, "\x15SV_GetServerinfo: bufferSize == %i", bufferSize);
    const char* v2 = Cvar_InfoString(4);
    Q_strncpyz(buffer, v2, bufferSize);
}

// ============================================================================
// SV_LocateGameData — ea: 0x51EB60
// ============================================================================
void SV_LocateGameData(Entity* entities, int numEntities, int sizeofEntity, PlayerState* clients, int sizeofClient) {
    (void)entities; (void)numEntities; (void)sizeofEntity; (void)clients; (void)sizeofClient;
}

// ============================================================================
// SV_GetUsercmd — ea: 0x51EB70
// ============================================================================
void SV_GetUsercmd(int clientNum, usercmd_s* cmd) {
    if (clientNum >= 0x10)
        Com_Error((errorParm_t)2, "\x15SV_GetUsercmd: bad clientNum:%i", clientNum);
    *cmd = CL_GetCurUserCmd(clientNum);
}

// ============================================================================
// SV_SetUsercmdButtonsWeapons — ea: 0x51EBB0
// ============================================================================
void SV_SetUsercmdButtonsWeapons(int buttons, int weapon) {
    CL_SetUsercmdButtonsWeapons(buttons, weapon);
}

// ============================================================================
// SV_GameSendServerCommand — ea: 0x520F90
// ============================================================================
void SV_GameSendServerCommand(DbLinkedHandle<EntityHandleDb, Entity> entityHandle, const char* text) {
    if (entityHandle.mHandle.mVal != 0) {
        unsigned int v2 = entityHandle.mHandle.mVal & 0xFFF;
        Entity* mObject = NULL;
        if (v2 < 0x540 && (unsigned int)(entityHandle.mHandle.mVal >> 12) == EntityHandleDb::sInst.mElements[v2].mKey)
            mObject = EntityHandleDb::sInst.mElements[v2].mObject;
        if (mObject != NULL && mObject->client != NULL)
            SV_SendServerCommand(svs.clients, "%s", text);
    } else {
        SV_SendServerCommand(NULL, "%s", text);
    }
}
