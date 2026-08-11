// ============================================================================
// sv_client.cpp — server client management (sv_client.cpp of sv.o)
// ============================================================================

#include "game/sv/sv_decl.h"
#include "game/sv/sv_stubs.h"

#include <stdio.h>
#include <string.h>

// ============================================================================
// Cross-object externs
// ============================================================================
extern void  Com_Printf(const char* fmt, ...);
extern void  Com_sprintf(char* dest, int size, const char* fmt, ...);
extern void  Com_DPrintf(const char* fmt, ...);
extern void  Cmd_TokenizeString(const char* text_in);
extern int   NET_IsLocalAddress(netadr_t adr);
extern void  Netchan_Setup(netsrc_t sock, netchan_t* chan, netadr_t adr, int qport);
extern int   MSG_ReadLong(msg_t* msg);
extern unsigned char MSG_ReadByte(msg_t* msg);
extern char* MSG_ReadString(msg_t* msg);
extern void* _Z_MallocInternal(int size);
extern char* ClientConnect(DbLinkedHandle<EntityHandleDb, Entity> entity);
extern void  SV_FreeAcknowledgedReliableCommands(client_s* cl);
extern void  SV_PreFrame(int msec);
extern void  SV_RunFrame(int msec);
extern void  CL_ParseGamestate(Broc::string* configstrings);
extern void  CL_ConnectResponse(netadr_t from);
extern void  SV_DirectConnect(netadr_t from);
extern void  SV_SwapClients(int client1, int client2);

// static helper
static int SV_ClientCommand(client_s* cl, msg_t* msg);   // ea: 0x51E970

// ============================================================================
// SV_PostConnect â€” ea: 0x5257E0
// ============================================================================
void SV_PostConnect() {
    if (svs.clients[currCl].state != CS_FREE) {
        int v1 = 0;
        client_s* clients = svs.clients;
        while (clients->state != CS_FREE) {
            if (clients[1].state == CS_FREE) {
                v1 += 1;
                break;
            }
            if (clients[2].state == CS_FREE) {
                v1 += 2;
                break;
            }
            if (clients[3].state == CS_FREE) {
                v1 += 3;
                break;
            }
            v1 += 4;
            clients += 4;
            if (v1 >= 16)
                break;
        }
        EntityManager::sInst->SwapPlayers(v1, currCl);
        SV_SwapClients(v1, currCl);
        for (int i = 0; i < 16; ++i) {
            MPPlayerManager* PlayerManager = MultiplayerMgr2_sInst()->mPeer->GetPlayerManager();
            MPPlayer* player = PlayerManager->GetPlayer(i);
            if (player != NULL && player->mClientIndex == currCl)
                player->mClientIndex = v1;
        }
    }
    netadr_t v12;
    v12.type = NA_BOT;
    memset(v12.ipx, 0, sizeof(v12.ipx));
    SV_DirectConnect(v12);
    v12.type = NA_BOT;
    memset(v12.ipx, 0, sizeof(v12.ipx));
    CL_ConnectResponse(v12);
    cls.state = CA_ACTIVE;
    int v6 = currCl;
    dword_F6A290[0] = 2;
    dword_F641E0[1580 * v6] = -1;
    if (v6 >= 16) {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\EntityManager.h";
        AeAssert::gCurrentLine = 19;
        AeAssert::gCurrentExpr = "idx<16";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Bounds check"))
            __debugbreak();
        v6 = currCl;
    }
    Entity* playerEnt = EntityManager_GetPlayerEntity(v6);
    DbLinkedHandle<EntityHandleDb, Entity>* v9 = (DbLinkedHandle<EntityHandleDb, Entity>*)((char*)playerEnt + 0x234);
    client_s* v10 = &svs.clients[v6];
    v10->mEntityHandle = v9[141];
    v10->deltaMessage = -1;
    ClientConnect(v9[141]);
}

// ============================================================================
// SV_SendClientGameState — ea: 0x51E910
// ============================================================================
void SV_SendClientGameState(client_s* client) {
    client->gamestateMessageNum = *(int*)&client->netchan[36];
    CL_ParseGamestate(sv.configstrings);
}

// ============================================================================
// SV_ExecuteClientCommand — ea: 0x51E930
// ============================================================================
void SV_ExecuteClientCommand(client_s* cl, const char* s) {
    if (sv.state == SS_GAME) {
        Cmd_TokenizeString(s);
        VM_Call(gvm, 5, &cl->mEntityHandle);
    }
}

// ============================================================================
// SV_ClientCommand — ea: 0x51E970 (static)
// ============================================================================
static int SV_ClientCommand(client_s* cl, msg_t* msg) {
    int Long = MSG_ReadLong(msg);
    const char* String = MSG_ReadString(msg);
    if (cl->lastClientCommand < Long) {
        if (sv_showCommands->integer != 0)
            Com_Printf("clientCommand: %i : %s\n", Long, String);
        if (Long > cl->lastClientCommand + 1) {
            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sv_client.cpp";
            AeAssert::gCurrentLine = 198;
            AeAssert::gCurrentExpr = "seq <= cl->lastClientCommand + 1";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
                __debugbreak();
        }
        if (sv.state == SS_GAME) {
            Cmd_TokenizeString(String);
            VM_Call(gvm, 5, &cl->mEntityHandle);
        }
        cl->lastClientCommand = Long;
        Com_sprintf(cl->lastClientCommandString, 256, "%s", String);
    }
    return 1;
}

// ============================================================================
// SV_DirectConnect — ea: 0x51FC80
// ============================================================================
void SV_DirectConnect(netadr_t from) {
    Com_DPrintf("SVC_DirectConnect ()\n");
    if (NET_IsLocalAddress(from) == 0) {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sv_client.cpp";
        AeAssert::gCurrentLine = 24;
        AeAssert::gCurrentExpr = "NET_IsLocalAddress(from)";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    from.port = (unsigned short)currCl;
    client_s* v1 = &svs.clients[currCl];
    memset(v1, 0, sizeof(client_s));
    EntityManager::sInst->CreatePlayers();
    v1->mEntityHandle.mHandle.mVal = EntityManager::sInst->GetPlayer(currCl)->mHandle.mHandle.mVal;
    Netchan_Setup(NS_SERVER, (netchan_t*)v1->netchan, from, 0);
    v1->reliableCommands.bufSize = 24576;
    v1->reliableCommands.buf = (char*)_Z_MallocInternal(24576);
    v1->reliableCommands.commandLengths = (int*)_Z_MallocInternal(256);
    char** v2 = (char**)_Z_MallocInternal(256);
    char* buf = v1->reliableCommands.buf;
    v1->reliableCommands.commands = v2;
    v1->reliableCommands.rover = buf;
    v1->state = (clientState_t)1;
    v1->gamestateMessageNum = -1;
}

// ============================================================================
// SV_ClientEnterWorld — ea: 0x51FDC0 (1-arg)
// ============================================================================
void SV_ClientEnterWorld(client_s* client) {
    client_s* v1 = client;
    Entity* Player = EntityManager::sInst->GetPlayer((int)(client - svs.clients));
    v1->mEntityHandle.mHandle.mVal = Player->mHandle.mHandle.mVal;
    if (Player->mHandle.mHandle.mVal == 0)
        EntityHandleDb::sInst.AssignHandle(*Player);
    v1->deltaMessage = -1;
    ClientConnect(Player->mHandle);
    client = (client_s*)(size_t)Player->mHandle.mHandle.mVal;
    VM_Call(gvm, 3, &client);
}

// ============================================================================
// SV_ExecuteClientMessage — ea: 0x51FE50
// ============================================================================
void SV_ExecuteClientMessage(client_s* cl, msg_t* msg) {
    cl->serverId = MSG_ReadLong(msg);
    int Long = MSG_ReadLong(msg);
    cl->messageAcknowledge = Long;
    if (Long < 0) {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sv_client.cpp";
        AeAssert::gCurrentLine = 232;
        AeAssert::gCurrentExpr = "cl->messageAcknowledge >= 0";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    int v4 = MSG_ReadLong(msg);
    int v5 = cl->reliableSequence - 64;
    cl->reliableAcknowledge = v4;
    if (v4 < v5) {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sv_client.cpp";
        AeAssert::gCurrentLine = 239;
        AeAssert::gCurrentExpr = "cl->reliableAcknowledge >= cl->reliableSequence - 64";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    int serverId = cl->serverId;
    if (serverId == sv.serverId) {
        SV_FreeAcknowledgedReliableCommands(cl);
        int Byte = MSG_ReadByte(msg);
        if (Byte != 3) {
            while (Byte == 2) {
                if (SV_ClientCommand(cl, msg) != 0) {
                    Byte = MSG_ReadByte(msg);
                    if (Byte != 3)
                        continue;
                }
                return;
            }
            if (Byte != 3)
                Com_Printf("WARNING: bad command byte for client %i\n", (int)(cl - svs.clients));
        }
    } else if (serverId == sv.restartedServerId) {
        for (int i = MSG_ReadByte(msg); i != 3; i = MSG_ReadByte(msg)) {
            if (i != 2)
                break;
            int v8 = MSG_ReadLong(msg);
            const char* String = MSG_ReadString(msg);
            const char* s = String;
            if (cl->lastClientCommand < v8) {
                if (sv_showCommands->integer != 0)
                    Com_Printf("Ignored clientCommand: %i : %s\n", v8, String);
                if (v8 > cl->lastClientCommand + 1) {
                    AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
                    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sv_client.cpp";
                    AeAssert::gCurrentLine = 275;
                    AeAssert::gCurrentExpr = "seq <= cl->lastClientCommand + 1";
                    if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
                        __debugbreak();
                }
                cl->lastClientCommand = v8;
                Com_sprintf(cl->lastClientCommandString, 256, "%s", s);
            }
        }
    }
}

// ============================================================================
// SV_ClientEnterWorld — ea: 0x520DE0 (3-arg)
// ============================================================================
void SV_ClientEnterWorld(client_s* client, int restart, int savegame) {
    client_s* v3 = client;
    if (client->state != (clientState_t)1) {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sv_client.cpp";
        AeAssert::gCurrentLine = 91;
        AeAssert::gCurrentExpr = "client->state == CS_ACTIVE";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    Entity* Player = EntityManager::sInst->GetPlayer((int)(v3 - svs.clients));
    v3->mEntityHandle.mHandle.mVal = Player->mHandle.mHandle.mVal;
    ClientConnect(Player->mHandle);
    v3->deltaMessage = -1;
    client = (client_s*)(size_t)Player->mHandle.mHandle.mVal;
    VM_Call(gvm, 3, &client);
    int v5 = savegame;
    if (restart == 0 || savegame == 0) {
        SV_PreFrame(50);
        if (++com_skelTimeStamp == 0)
            com_skelTimeStamp = 1;
        if (bSV_AllowedAllocSkel != 0) {
            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sv_client.cpp";
            AeAssert::gCurrentLine = 120;
            AeAssert::gCurrentExpr = "!bSV_AllowedAllocSkel";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
                __debugbreak();
        }
        bSV_AllowedAllocSkel = 1;
        VM_Call(gvm, 8, v5);
        if (bSV_AllowedAllocSkel == 0) {
            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sv_client.cpp";
            AeAssert::gCurrentLine = 126;
            AeAssert::gCurrentExpr = "bSV_AllowedAllocSkel";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
                __debugbreak();
        }
        bSV_AllowedAllocSkel = 0;
        SV_RunFrame(50);
    }
}
