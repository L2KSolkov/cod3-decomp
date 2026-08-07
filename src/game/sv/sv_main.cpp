// ============================================================================
// sv_main.cpp — server main loop + network channel (sv_main.cpp of sv.o)
// ============================================================================

#include "game/sv/sv_decl.h"
#include "game/sv/sv_stubs.h"

#include <string.h>

// ============================================================================
// Cross-object externs
// ============================================================================
extern void  G_RunFrame(int msec);
extern void  Com_Shutdown(void);
extern void  Com_EventLoop(void);
extern void  Cbuf_AddText(const char* text);
extern void  Cbuf_ExecuteText(int exec_when, const char* text);
extern void  CL_SetFrametime(int frametime, int animFrametime);
extern int   CL_IsCGameRendering(void);
extern void  SV_PreFrame(int msec);
extern int   SV_CheckLoadGame(void);
extern void  SV_SendClientMessages(void);
extern void  MSG_WriteByte(msg_t* msg, int c);
extern void  MSG_WriteLong(msg_t* msg, int c);
extern void  MSG_WriteString(msg_t* msg, const char* s);
extern void  MSG_BeginReading(msg_t* msg);
extern short MSG_ReadShort(msg_t* msg);
extern void  Netchan_Transmit(netchan_t* chan, int length, const unsigned char* data);
extern int   Netchan_Process(netchan_t* chan, msg_t* msg);
extern void  NET_OutOfBandPrint(netsrc_t sock, netadr_t adr, const char* format, ...);
extern void  SV_ExecuteClientMessage(client_s* cl, msg_t* msg);
extern const char* nullStr;
extern int   com_frameNumber;

// cdl profilers
struct cdl_proftimer {
    void start();
    void stop();
};
extern cdl_proftimer cdl_proftimer_vmcalls;
extern cdl_proftimer cdl_proftimer_cl_msgs;

// ============================================================================
// SV_RunFrame — ea: 0x51F820
// ============================================================================
void SV_RunFrame(int msec) {
    if (++com_skelTimeStamp == 0)
        com_skelTimeStamp = 1;
    if (bSV_AllowedAllocSkel != 0) {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sv_main.cpp";
        AeAssert::gCurrentLine = 359;
        AeAssert::gCurrentExpr = "!bSV_AllowedAllocSkel";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    bSV_AllowedAllocSkel = 1;
    G_RunFrame(msec);
    if (bSV_AllowedAllocSkel == 0) {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sv_main.cpp";
        AeAssert::gCurrentLine = 367;
        AeAssert::gCurrentExpr = "bSV_AllowedAllocSkel";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    bSV_AllowedAllocSkel = 0;
}

// ============================================================================
// SV_SmoothFrame — ea: 0x51F8F0
// ============================================================================
void SV_SmoothFrame() {
    AeAssert::gCurrentAuthor = (AeAssert::ECoderId)2;   // SLB
    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sv_main.cpp";
    AeAssert::gCurrentLine = 557;
    AeAssert::gCurrentExpr = NULL;
    if (!AeAssert::IsIgnored()) {
        if (AeAssert::Warning("Unsupported."))
            __debugbreak();
    }
}

// ============================================================================
// SV_Netchan_Transmit — ea: 0x51F940
// ============================================================================
void SV_Netchan_Transmit(client_s* client, msg_t* msg) {
    MSG_WriteByte(msg, 8);
    Netchan_Transmit((netchan_t*)client->netchan, msg->cursize, msg->data);
}

// ============================================================================
// SV_Netchan_Process — ea: 0x51F970
// ============================================================================
int SV_Netchan_Process(client_s* client, msg_t* msg) {
    return Netchan_Process((netchan_t*)client->netchan, msg);
}

// ============================================================================
// SV_UpdateServerCommandsToClient — ea: 0x51FA10
// ============================================================================
void SV_UpdateServerCommandsToClient(client_s* client, msg_t* msg) {
    int v2 = client->reliableAcknowledge + 1;
    if (v2 > client->reliableSequence) {
        client->reliableSent = client->reliableSequence;
    } else {
        int reliableSequence;
        do {
            MSG_WriteByte(msg, 5);
            MSG_WriteLong(msg, v2);
            int v3 = v2 & 0x3F;
            const char* v4;
            if (client->reliableCommands.bufSize != 0) {
                if (client->reliableCommands.commandLengths[v3] != 0)
                    v4 = client->reliableCommands.commands[v3];
                else
                    v4 = nullStr;
            } else {
                v4 = nullStr;
            }
            MSG_WriteString(msg, v4);
            reliableSequence = client->reliableSequence;
            ++v2;
        } while (v2 <= reliableSequence);
        client->reliableSent = reliableSequence;
    }
}

// ============================================================================
// SV_SendMessageToClient — ea: 0x51FAA0
// ============================================================================
void SV_SendMessageToClient(msg_t* msg, client_s* client) {
    sv_snapshotFrameNumber = com_frameNumber;
    MSG_WriteByte(msg, 8);
    Netchan_Transmit((netchan_t*)client->netchan, msg->cursize, msg->data);
}

// ============================================================================
// SV_GetFollowPlayerState — ea: 0x51FAE0
// ============================================================================
int SV_GetFollowPlayerState(int clientNum, PlayerState* ps) {
    return VM_Call(gvm, 7, clientNum, ps);
}

// ============================================================================
// SV_GetCurrentClientInfo — ea: 0x51FB00
// ============================================================================
int SV_GetCurrentClientInfo(int clientNum, PlayerState* ps) {
    return svs.clients[clientNum].state == 1 && VM_Call(gvm, 7, clientNum, ps) != 0;
}

// ============================================================================
// SV_Vid_Restart — ea: 0x51F800
// ============================================================================
void SV_Vid_Restart() {
    Cbuf_ExecuteText(2, "savegame internal\\vid_restart\n");
}

// ============================================================================
// SV_Snd_Restart — ea: 0x51F810
// ============================================================================
void SV_Snd_Restart() {
    Cbuf_ExecuteText(2, "savegame internal\\snd_restart\n");
}

// ============================================================================
// SV_PacketEvent — ea: 0x520790
// ============================================================================
void SV_PacketEvent(netadr_t from, msg_t* msg) {
    if (++com_skelTimeStamp == 0)
        com_skelTimeStamp = 1;
    if (bSV_AllowedAllocSkel != 0) {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sv_main.cpp";
        AeAssert::gCurrentLine = 244;
        AeAssert::gCurrentExpr = "!bSV_AllowedAllocSkel";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    bSV_AllowedAllocSkel = 1;
    MSG_BeginReading(msg);
    MSG_ReadLong(msg);
    int Short = MSG_ReadShort(msg);
    int v3 = 0;
    client_s* clients = svs.clients;
    while (clients->state == 0 || Short != v3) {
        ++v3;
        ++clients;
        if (v3 >= 16) {
            NET_OutOfBandPrint(NS_SERVER, from, "disconnect");
            goto done;
        }
    }
    client_s* v5 = &svs.clients[Short];
    if (Netchan_Process((netchan_t*)v5->netchan, msg) != 0)
        SV_ExecuteClientMessage(v5, msg);
done:
    if (bSV_AllowedAllocSkel == 0) {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sv_main.cpp";
        AeAssert::gCurrentLine = 276;
        AeAssert::gCurrentExpr = "bSV_AllowedAllocSkel";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    bSV_AllowedAllocSkel = 0;
}

// ============================================================================
// SV_Frame — ea: 0x5259C0
// ============================================================================
void SV_Frame(int msec) {
    AeThreadManager::sInst.Execute(msec * 0.001f);
    if (sv_killserver->integer) {
        Com_Shutdown();
        Cvar_Set("sv_killserver", "0");
    } else if (com_sv_running->integer) {
        if (!GamePause::IsGamePaused(currCl) && !CL_IsCGameRendering())
            GamePause::SetGamePaused(currCl, 1);
        svs.clients->serverId = sv.serverId;
        if (SV_CheckLoadGame()) {
            CL_SetFrametime(0, 0);
        } else {
            cdl_proftimer_vmcalls.start();
            if (!com_inServerFrame) {
                SV_PreFrame(msec);
                com_inServerFrame = 1;
            }
            int v1 = VM_Call(gvm, 20);
            com_time = v1;
            cdl_proftimer_vmcalls.stop();
            if (v1 <= 1879048192 && svs.nextSnapshotEntities < 2147483646 - svs.numSnapshotEntities) {
                SV_RunFrame(msec);
                cdl_proftimer_cl_msgs.start();
                SV_SendClientMessages();
                CL_SetFrametime(msec, 0);
                cdl_proftimer_cl_msgs.stop();
            } else {
                Com_Shutdown();
                Cbuf_AddText("vstr nextmap\n");
            }
        }
    } else {
        CL_SetFrametime(msec, 0);
    }
}
