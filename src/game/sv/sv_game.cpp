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
extern void  Com_Memset(void* dest, int val, unsigned int count);
extern char* CL_GetCurUserCmd(unsigned int clientNum);
extern void  CL_SetUsercmdButtonsWeapons(int buttons, int weapon);
extern void* mem_heap_malloc_ctx(int alignment, unsigned int size, const char* ctx, const char* file, int line);
extern int   com_skelTimeStamp;
extern cvar_t* com_developer;
extern const char* nullStr;

// DObj API (animation lib)
class DObj;
struct DObjSkelMat;
class XAnimTree;
extern void  DObjDumpInfo(DObj* obj);
extern int   DObjSkelExists(DObj* obj, int timeStamp);
extern int   DObjSkelExistsConst(DObj* obj, int timeStamp);
extern int   DObjGetAllocSkelSize(DObj* obj);
extern void  DObjCreateSkel(DObj* obj, char* buf);
extern int   DObjUpdateServerInfo(DObj* obj, float dtime, bool bNotify, int animindex);
extern void  DObjInitServerTime(DObj* d, float dtime);
extern void  DObjGetHierarchyBits(DObj* obj, int boneIndex, int* partBits);
extern void  DObjCalcAnim(DObj* obj, int iPhase);
extern void  j_nullsub_82(DObj* obj, int* partBits);
extern int   DObjNumBones(DObj* obj);
extern int   DObjGetBoneIndex(const DObj* obj, unsigned int boneNameHash);
extern DObjSkelMat* DObjGetMatrixArray(const DObj* obj, int modelIndex);
extern void  DObjDisplayAnim(DObj* obj);
extern void  DObjDisplayAnim3D(int id, DObj* obj, float* loc, int line);
extern XAnimTree* DObjGetTree(DObj* obj);

// File / model
struct fileData_s {
    char* name;                // +0x00
    void* data;                // +0x04
    void (*mem_heap_free)(fileData_s*);  // +0x08
};
extern fileData_s* FS_GetDataForFile(const char* path, const char* filename, const char* extension);
extern int  CurPakId(void);
extern void SV_SendServerCommand(client_s* cl, const char* fmt, ...);

// ============================================================================
// SV_GameSystemCalls — ea: 0x51F190
// ============================================================================
int SV_GameSystemCalls(int* args) {
    Com_Error(2, "\x15" "Bad game system trap: %i", *args);
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
// SV_GameCommand — ea: 0x51F1E0
// ============================================================================
int SV_GameCommand() {
    return 0;
}

// ============================================================================
// SV_SaveWrite — ea: 0x51EBC0
// ============================================================================
void* SV_SaveWrite(const void* buffer, int len) {
    void* result = mem_heap_malloc_ctx(16, len, "hunk", "c:\\cod\\code\\game\\sv_game.cpp", 622);
    memcpy(result, buffer, len);
    return result;
}

// ============================================================================
// SV_XModelGet — ea: 0x51EC00
// ============================================================================
IVPointer<XModel> SV_XModelGet(const char* name) {
    XModelManager* v2 = XModelManager::sInst;
    int v3 = CurPakId();
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
    char* v4 = (char*)mem_heap_malloc_ctx(16, AllocSkelSize, "hunk", "c:\\cod\\code\\game\\sv_game.cpp", 689);
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
        char* v4 = (char*)mem_heap_malloc_ctx(16, AllocSkelSize, "hunk", "c:\\cod\\code\\game\\sv_game.cpp", 718);
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
        Com_Error(2, "\x15SV_GetServerinfo: bufferSize == %i", bufferSize);
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
        Com_Error(2, "\x15SV_GetUsercmd: bad clientNum:%i", clientNum);
    *cmd = *(usercmd_s*)CL_GetCurUserCmd(clientNum);
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
