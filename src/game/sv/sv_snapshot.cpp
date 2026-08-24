// ============================================================================
// sv_snapshot.cpp — client snapshot transmission (sv_snapshot.cpp of sv.o)
// ============================================================================

#include "game/sv/sv_decl.h"
#include "game/sv/sv_stubs.h"

#include <string.h>

// ============================================================================
// Cross-object externs
// ============================================================================
extern void  MSG_Init(msg_t* buf, unsigned char* data, int length);
extern void  MSG_WriteByte(msg_t* msg, int c);
extern void  MSG_WriteLong(msg_t* msg, int c);
extern void  MSG_WriteString(msg_t* msg, const char* s);
extern void  Netchan_Transmit(netchan_t* chan, int length, const unsigned char* data);
extern void  SV_UpdateServerCommandsToClient(client_s* client, msg_t* msg);
static void SV_BuildClientSnapshot(client_s* client);   // ea: 0x521260
static void SV_WriteSnapshotToClient(client_s* client, msg_t* msg);  // ea: 0x51F990
static void SV_AddEntitiesVisibleFromPoint(int leafnum, DbLinkedHandle<EntityHandleDb, Entity> clientHandle);
extern int   com_frameNumber;

int dword_F641E0[4 * 1580];  // sv.o BSS @ 0xF641E0

// collision / math
extern void  AddLeanToPosition(float* const vPosition, float fViewYaw,
                               float fLeanFrac, float fViewRoll,
                               float fLeanDist);
extern int   CM_PointLeafnum(const math::Position3& p);
extern int   CM_LeafArea(int leafnum);
extern int   CM_LeafCluster(int leafnum);
extern unsigned char* CM_ClusterPVS(int cluster);
extern int   CM_AreasConnected(int area1, int area2);
extern const math::Position3 native_to_cdl_pos3(const float* v);

// ============================================================================
// SV_WriteSnapshotToClient — ea: 0x51F990 (static)
// ============================================================================
static void SV_WriteSnapshotToClient(client_s* client, msg_t* msg) {
    MSG_WriteByte(msg, 7);
    int v2 = VM_Call(gvm, 20);
    MSG_WriteLong(msg, v2);
    if (client->state != (clientState_t)1) {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sv_snapshot.cpp";
        AeAssert::gCurrentLine = 49;
        AeAssert::gCurrentExpr = "client->state == CS_ACTIVE";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("%i", client->state))
            __debugbreak();
    }
    MSG_WriteByte(msg, svs.snapFlagServerBit);
}

// ============================================================================
// SV_BuildClientSnapshot — ea: 0x521260 (static)
// ============================================================================
static void SV_BuildClientSnapshot(client_s* client) {
    unsigned int v2 = client->mEntityHandle.mHandle.mVal & 0xFFF;
    if (v2 < 0x540 && (unsigned int)(client->mEntityHandle.mHandle.mVal >> 12) == EntityHandleDb::sInst.mElements[v2].mKey) {
        Entity* mObject = EntityHandleDb::sInst.mElements[v2].mObject;
        if (mObject != NULL) {
            PlayerState* frames = client->frames;
            memcpy(client->frames, mObject->client, sizeof(client->frames));
            unsigned int mVal = client->frames[0].mClient.mHandle.mVal;
            if (mVal == 0) {
                Com_Error((errorParm_t)2, "\x15" "SV_BuildClientSnapshot: bad gEnt->client");
                frames = client->frames;
            }
            float leanf = frames->leanf;
            float v6 = frames->viewangles[1];
            float v9 = frames->origin.v.m128_f32[0];
            float v10 = frames->origin.v.m128_f32[1];
            float v11 = frames->viewHeightCurrent + frames->origin.v.m128_f32[2];
            AddLeanToPosition(&v9, v6, leanf, 16.0f, 20.0f);
            math::Position3 v8;
            v8.v.m128_f32[0] = v9;
            v8.v.m128_f32[1] = v10;
            v8.v.m128_f32[2] = v11;
            v8.v.m128_f32[3] = 0.0f;
            int v7 = CM_PointLeafnum(v8);
            if (*(int*)&client->netchan[8] != 2) {
                AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
                AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sv_snapshot.cpp";
                AeAssert::gCurrentLine = 312;
                AeAssert::gCurrentExpr = "client->netchan.remoteAddress.type == NA_LOOPBACK";
                if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
                    __debugbreak();
            }
            SV_AddEntitiesVisibleFromPoint(v7, *(DbLinkedHandle<EntityHandleDb, Entity>*)((unsigned int)mObject + 564));
        }
    }
}

// ============================================================================
// SV_SendClientSnapshot — ea: 0x5213E0
// ============================================================================
void SV_SendClientSnapshot(client_s* client) {
    unsigned __int8 msg_buf[3072];
    msg_t msg;
    SV_BuildClientSnapshot(client);
    MSG_Init(&msg, msg_buf, 3072);
    MSG_WriteLong(&msg, client->lastClientCommand);
    SV_UpdateServerCommandsToClient(client, &msg);
    SV_WriteSnapshotToClient(client, &msg);
    if (msg.overflowed != 0 || msg.cursize > msg.maxsize)
        Com_Error((errorParm_t)2, "\x15" "msg overflowed");
    sv_snapshotFrameNumber = com_frameNumber;
    MSG_WriteByte(&msg, 8);
    Netchan_Transmit((netchan_t*)client->netchan, msg.cursize, msg.data);
}

// ============================================================================
// SV_AddEntitiesVisibleFromPoint — ea: 0x520C20 (static)
// ============================================================================
static void SV_AddEntitiesVisibleFromPoint(int leafnum, DbLinkedHandle<EntityHandleDb, Entity> clientHandle) {
    if (sv.state == SS_DEAD)
        return;
    int v3 = CM_LeafArea(leafnum);
    int clientarea = v3;
    int v4 = CM_LeafCluster(leafnum);
    unsigned __int8* v5 = CM_ClusterPVS(v4);
    AeSizedEntityArray& arr = EntityHandleDb::sInst.mActiveList;
    for (int idx = 0; idx < arr.m_size; ++idx) {
        {
            Entity* v7 = arr.m_elements[idx];
            if (v7 != NULL && v7->r.linked != 0) {
                int svFlags = v7->r.svFlags;
                if ((svFlags & 1) == 0
                    && ((svFlags & 0x800) == 0 || v7->r.mSingleClient.mHandle.mVal == clientHandle.mHandle.mVal)
                    && ((svFlags & 0x2000) == 0 || v7->r.mSingleClient.mHandle.mVal != clientHandle.mHandle.mVal)) {
                    if (v7->mHandle.mHandle.mVal != clientHandle.mHandle.mVal && (svFlags & 0x20) == 0 && v7->r.numClusters != 0) {
                        if (CM_AreasConnected(v3, v7->r.areanum) == 0
                            && CM_AreasConnected(v3, v7->r.areanum2) == 0) {
                            // not visible
                            if (v7->r.eventTime == 0)
                                continue;
                            v7->s.eFlags |= 0x80u;
                            continue;
                        }
                        int v9 = 0;
                        int i = 0;
                        if (v7->r.numClusters > 0) {
                            int* clusternums = v7->r.clusternums;
                            do {
                                v9 = *clusternums;
                                if (((1 << (*clusternums & 7)) & v5[*clusternums >> 3]) != 0)
                                    break;
                                ++clusternums;
                                ++i;
                            } while (i < v7->r.numClusters);
                        }
                        if (i == v7->r.numClusters) {
                            int lastCluster = v7->r.lastCluster;
                            if (lastCluster == 0) {
                                // not visible
                                if (v7->r.eventTime == 0)
                                    continue;
                                v7->s.eFlags |= 0x80u;
                            } else if (v9 <= lastCluster) {
                                do {
                                    if (((1 << (v9 & 7)) & v5[v9 >> 3]) != 0)
                                        break;
                                    ++v9;
                                } while (v9 <= lastCluster);
                                if (v9 == lastCluster) {
                                    // not visible
                                    if (v7->r.eventTime == 0)
                                        continue;
                                    v7->s.eFlags |= 0x80u;
                                }
                            }
                        }
                    }
                    v7->SetInSnapshot();
                }
            }
        }
    }
}

// ============================================================================
// SV_inSnapshot — ea: 0x521000
// ============================================================================
int SV_inSnapshot(const float* const origin, DbLinkedHandle<EntityHandleDb, Entity> entityHandle) {
    unsigned int v3 = entityHandle.mHandle.mVal & 0xFFF;
    Entity* mObject = NULL;
    if (v3 < 0x540 && (unsigned int)(entityHandle.mHandle.mVal >> 12) == EntityHandleDb::sInst.mElements[v3].mKey)
        mObject = EntityHandleDb::sInst.mElements[v3].mObject;
    if (mObject->r.linked == 0)
        return 0;
    int svFlags = mObject->r.svFlags;
    if ((svFlags & 1) != 0)
        return 0;
    if ((svFlags & 0x20) != 0 || mObject->r.eventTime != 0 || mObject->r.numClusters == 0)
        return 1;
    math::Position3 v18;
    v18 = native_to_cdl_pos3(origin);
    const math::Position3* v6 = &v18;
    int v7 = CM_PointLeafnum(*v6);
    int v20 = CM_LeafArea(v7);
    int v8 = CM_LeafCluster(v7);
    unsigned __int8* v9 = CM_ClusterPVS(v8);
    int v10 = v20;
    int areanum = mObject->r.areanum;
    unsigned __int8* v19 = v9;
    if (CM_AreasConnected(v20, areanum) == 0 && CM_AreasConnected(v10, mObject->r.areanum2) == 0)
        return 0;
    int numClusters = mObject->r.numClusters;
    int v12 = 0;
    v20 = 0;
    unsigned __int8* v13;
    if (numClusters <= 0) {
        v13 = v19;
    } else {
        int* clusternums = mObject->r.clusternums;
        do {
            v12 = *clusternums;
            v13 = v19;
            if (((1 << (*clusternums & 7)) & v19[*clusternums >> 3]) != 0)
                break;
            ++clusternums;
        } while (++v20 < mObject->r.numClusters);
    }
    if (v20 != mObject->r.numClusters)
        return 1;
    int lastCluster = mObject->r.lastCluster;
    if (lastCluster == 0)
        return 0;
    if (v12 > lastCluster)
        return 1;
    do {
        if (((1 << (v12 & 7)) & v13[v12 >> 3]) != 0)
            break;
        ++v12;
    } while (v12 <= lastCluster);
    return v12 != lastCluster;
}

// ============================================================================
// SV_SendClientMessages — ea: 0x521480
// ============================================================================
void SV_SendClientMessages() {
    if (svs.clients == NULL) {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sv_snapshot.cpp";
        AeAssert::gCurrentLine = 385;
        AeAssert::gCurrentExpr = "svs.clients";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (svs.clients != NULL) {
        VM_Call(gvm, 22);
        netsrc_t v0 = (netsrc_t)currCl;
        if (svs.clients->state != 0 && dword_F6A290[0] == 2) {
            currCl = NS_CLIENT;
            SV_SendClientSnapshot(svs.clients);
        }
        currCl = (int)v0;
        com_inServerFrame = 0;
    }
}
