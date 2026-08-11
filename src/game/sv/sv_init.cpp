// ============================================================================
// sv_init.cpp — server initialization + configstrings + reliable commands
// (sv_init.cpp + sv_main.cpp portions of sv.o)
// ============================================================================

#include "game/sv/sv_decl.h"
#include "game/sv/sv_stubs.h"

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

// ============================================================================
// Cross-object externs
// ============================================================================
extern void  Q_strncpyz(char* dest, const char* src, int destsize);
extern void  Com_Memset(unsigned int* dest, int val, unsigned int count);
extern void  CL_Disconnect();
extern void* _Z_MallocInternal(int size);
extern void  _Z_FreeInternal(void* ptr);
extern void* mem_heap_malloc(int alignment, unsigned int size);
extern void  SV_AddServerCommand(client_s* client, const char* cmd);
extern void  SV_DumpServerCommands(client_s* client);
extern void  SV_AddReliableCommand(client_s* cl, int index, const char* cmd);
extern void  SV_SendServerCommand(client_s* cl, const char* fmt, ...);
extern cvar_t* com_sv_running;
extern const char* nullStr;

// ============================================================================
// SV_GetConfigstring (string& overload) — ea: 0x51F310
// ============================================================================
void SV_GetConfigstring(int index, Broc::string& str) {
    if (index >= 0x400)
        Com_Error(2, "\x15SV_GetConfigstring: bad index %i\n", index);
    str = sv.configstrings[index];
}

// ============================================================================
// SV_GetConfigstring (char* overload) — ea: 0x520220
// ============================================================================
void SV_GetConfigstring(int index, char* buffer, int bufferSize) {
    if (bufferSize < 1)
        Com_Error(2, "\x15SV_GetConfigstring: bufferSize == %i", bufferSize);
    if (index >= 0x400)
        Com_Error(2, "\x15SV_GetConfigstring: bad index %i\n", index);
    if (sv.configstrings[index].mBlock == NULL) {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sv_init.cpp";
        AeAssert::gCurrentLine = 144;
        AeAssert::gCurrentExpr = "sv.configstrings[index].IsDefined()";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    Broc::string::Block* mBlock = sv.configstrings[index].mBlock;
    const char* v4 = (const char*)&mBlock[1];
    if (mBlock == NULL)
        v4 = defaultFileName;
    Q_strncpyz(buffer, v4, bufferSize);
}

// ============================================================================
// SV_GetConfigstringConst — ea: 0x5202D0
// ============================================================================
const char* SV_GetConfigstringConst(int index) {
    if (index >= 0x400) {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sv_init.cpp";
        AeAssert::gCurrentLine = 165;
        AeAssert::gCurrentExpr = "(unsigned) index < 1024";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    Broc::string::Block* mBlock = sv.configstrings[index].mBlock;
    if (mBlock != NULL)
        return (const char*)(mBlock + 1);
    else
        return defaultFileName;
}

// ============================================================================
// SV_SetConfigstring — ea: 0x5209E0
// ============================================================================
void SV_SetConfigstring(int index, const char* val) {
    char buf[256];
    int v11;
    client_s* client;

    if (index >= 0x400)
        Com_Error(2, "\x15SV_SetConfigstring: bad index %i\n", index);
    const char* v2 = val;
    const char* v3 = defaultFileName;
    if (val == NULL) {
        val = defaultFileName;
        v2 = defaultFileName;
    }
    if (sv.configstrings[index].mBlock == NULL) {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sv_init.cpp";
        AeAssert::gCurrentLine = 63;
        AeAssert::gCurrentExpr = "sv.configstrings[index].IsDefined()";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (sv.configstrings[index].mBlock != NULL)
        v3 = (const char*)&sv.configstrings[index].mBlock[1];
    if (strcmp(v2, v3) != 0) {
        sv.configstrings[index] = v2;
        if (sv.state == SS_GAME) {
            client_s* clients = svs.clients;
            client = svs.clients;
            v11 = 16;
            do {
                if (clients->state == 1) {
                    int v5 = (int)strlen(val);
                    if (v5 < 232) {
                        SV_SendServerCommand(clients, "cs %i %s", index, val);
                    } else {
                        int v6 = 0;
                        for (int i = v5; i > 0; i -= 231) {
                            const char* v8;
                            if (v6 != 0) {
                                v8 = "bcs2";
                                if (i >= 232)
                                    v8 = "bcs1";
                            } else {
                                v8 = "bcs0";
                            }
                            Q_strncpyz(buf, &val[v6], 232);
                            SV_SendServerCommand(client, "%s %i %s", v8, index, buf);
                            v6 += 231;
                        }
                        clients = client;
                    }
                }
                ++clients;
                if (v11 == 1)
                    break;
                client = clients;
                --v11;
            } while (1);
        }
    }
}

// ============================================================================
// SV_InitReliableCommandsForClient — ea: 0x51F350
// ============================================================================
void SV_InitReliableCommandsForClient(client_s* cl, int commands) {
    if (commands == 0) {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sv_init.cpp";
        AeAssert::gCurrentLine = 179;
        AeAssert::gCurrentExpr = "commands";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    cl->reliableCommands.bufSize = 384 * commands;
    cl->reliableCommands.buf = (char*)_Z_MallocInternal(384 * commands);
    cl->reliableCommands.commandLengths = (int*)_Z_MallocInternal(4 * commands);
    cl->reliableCommands.commands = (char**)_Z_MallocInternal(4 * commands);
    cl->reliableCommands.rover = cl->reliableCommands.buf;
}

// ============================================================================
// SV_FreeReliableCommandsForClient — ea: 0x51F3E0
// ============================================================================
void SV_FreeReliableCommandsForClient(client_s* cl) {
    if (cl->reliableCommands.bufSize != 0) {
        _Z_FreeInternal(cl->reliableCommands.buf);
        _Z_FreeInternal(cl->reliableCommands.commandLengths);
        _Z_FreeInternal(cl->reliableCommands.commands);
        Com_Memset((unsigned int*)&cl->reliableCommands, 0, 4u);
    }
}

// ============================================================================
// SV_GetReliableCommand — ea: 0x51F420
// ============================================================================
char* SV_GetReliableCommand(client_s* cl, int index) {
    if (cl->reliableCommands.bufSize != 0 && cl->reliableCommands.commandLengths[index] != 0)
        return cl->reliableCommands.commands[index];
    else
        return (char*)nullStr;
}

// ============================================================================
// SV_FreeAcknowledgedReliableCommands — ea: 0x51F450
// ============================================================================
void SV_FreeAcknowledgedReliableCommands(client_s* cl) {
    if (cl->reliableCommands.bufSize != 0) {
        char v2 = (char)(cl->reliableAcknowledge - 1);
        int realAck = cl->reliableAcknowledge & 0x3F;
        int v3 = v2 & 0x3F;
        for (char* i = cl->reliableCommands.commands[v3]; i != NULL; i = cl->reliableCommands.commands[v3]) {
            memset(i, 0, cl->reliableCommands.commandLengths[v3--]);
            cl->reliableCommands.commands[v3 + 1] = NULL;
            cl->reliableCommands.commandLengths[v3 + 1] = 0;
            if (v3 < 0)
                v3 = 63;
            if (v3 == realAck)
                break;
        }
    }
}

// ============================================================================
// SV_AddReliableCommand — ea: 0x520340
// ============================================================================
void SV_AddReliableCommand(client_s* cl, int index, const char* cmd) {
    int length;
    if (cl->reliableCommands.bufSize == 0) {
        SV_DumpServerCommands(cl);
        Com_Error(2, "Reliable command buffer overflow");
    }
    const char* v4 = &cmd[strlen(cmd) + 1];
    char* buf = cl->reliableCommands.buf;
    int v6 = (int)(v4 - (cmd + 1));
    length = v6;
    if ((int)(cl->reliableCommands.rover - buf) + v6 + 1 >= cl->reliableCommands.bufSize)
        cl->reliableCommands.rover = buf;
    char* rover = cl->reliableCommands.rover;
    int v8 = (int)(v4 - (cmd + 1));
    if (v6 != 0) {
        while (*rover == 0) {
            --v8;
            ++rover;
            if (v8 == 0)
                goto LABEL_28;
        }
        if (v8 != 0) {
            for (int i = 0; i < cl->reliableCommands.bufSize; ++buf) {
                if (*buf == 0 && (i == 0 || *(buf - 1) == 0)) {
                    int v10 = 0;
                    int v11 = cl->reliableCommands.bufSize - 1;
                    if (i < v11) {
                        do {
                            if (v10 >= length + 1)
                                break;
                            if (buf[v10] != 0)
                                break;
                            ++i;
                            ++v10;
                        } while (i < cl->reliableCommands.bufSize - 1);
                        v6 = length;
                    }
                    if (v10 == v6 + 1) {
                        v6 = length;
                        cl->reliableCommands.rover = buf;
                        break;
                    }
                    if (i == v11) {
                        Com_Printf("===== pending server commands =====\n");
                        for (int j = cl->reliableAcknowledge + 1; j <= cl->reliableSequence; ++j)
                            Com_Printf("cmd %5d: %s\n", j, cl->reliableCommands.commands[j & 0x3F]);
                        Com_Error(2, "Reliable command buffer overflow");
                    }
                    v6 = length;
                    buf = &cl->reliableCommands.buf[i];
                }
                ++i;
            }
        }
    }
LABEL_28:
    int v13 = index;
    cl->reliableCommands.commands[index] = cl->reliableCommands.rover;
    int v14 = 0;
    cl->reliableCommands.commandLengths[index] = v6;
    if (v6 > 0) {
        do {
            char* v15 = &cl->reliableCommands.commands[v13][v14];
            char v16 = Q_CleanCharacter(cmd[v14]);
            *v15 = v16;
            if (v16 == 37)
                *v15 = 46;
            ++v14;
            v13 = index;
        } while (v14 < length);
        v6 = length;
    }
    cl->reliableCommands.commands[v13][v6] = 0;
    cl->reliableCommands.rover += v6 + 1;
}

// ============================================================================
// SV_ClearServer — ea: 0x5204E0
// ============================================================================
void SV_ClearServer() {
    if (svs.clients != NULL) {
        unsigned int v0 = 0;
        do {
            client_s* v1 = &svs.clients[v0 / 0x1370];
            reliableCommands_t* p_reliableCommands = &svs.clients[v0 / 0x1370].reliableCommands;
            if (p_reliableCommands->bufSize != 0) {
                _Z_FreeInternal(v1->reliableCommands.buf);
                _Z_FreeInternal(v1->reliableCommands.commandLengths);
                _Z_FreeInternal(v1->reliableCommands.commands);
                Com_Memset((unsigned int*)p_reliableCommands, 0, 4u);
            }
            v0 += 4976;
        } while (v0 < 0x13700);
    }
    Broc::string* configstrings = sv.configstrings;
    do {
        if (configstrings->mBlock != NULL) {
            configstrings->mBlock->DecrementCount();
            configstrings->mBlock = NULL;
        }
        ++configstrings;
    } while (configstrings < &sv.configstrings[1024]);
    Com_Memset((unsigned int*)&sv, 0, 0x1010u);
    com_inServerFrame = 0;
}

// ============================================================================
// SV_Startup — ea: 0x51F4D0
// ============================================================================
void SV_Startup() {
    if (svs.initialized != 0)
        Com_Error(1, "\x15SV_Startup: svs.initialized");
    client_s* clients = (client_s*)mem_heap_malloc(16, 0x13700);
    svs.clients = clients;
    if (clients == NULL) {
        Com_Error(1, "\x15SV_Startup: unable to allocate svs.clients");
        clients = svs.clients;
    }
    Com_Memset((unsigned int*)clients, 0, 0x13700);
    svs.numSnapshotEntities = 1344;
    svs.initialized = 1;
    Cvar_Set("sv_running", "1");
}

// ============================================================================
// SV_SetExpectedHunkUsage — ea: 0x51F550
// ============================================================================
void SV_SetExpectedHunkUsage(char* mapname) {
    int handle;
    const char* buftrav;
    int v1 = FS_FOpenFileByMode("hunkusage.dat", &handle, 0);
    int v2 = v1;
    if (v1 >= 0) {
        unsigned int v3 = (unsigned int)(v1 + 1);
        void* v4 = _Z_MallocInternal(v1 + 1);
        memset(v4, 0, v3);
        char* buf = (char*)v4;
        FS_Read((unsigned char*)v4, (unsigned int)v2, handle);
        FS_FCloseFile(handle);
        buftrav = buf;
        const char* v5 = Com_Parse(&buftrav);
        if (v5 != NULL) {
            while (*v5 != 0) {
                if (Q_strcasecmp(v5, mapname) == 0) {
                    const char* v6 = Com_Parse(&buftrav);
                    if (v6 != NULL && *v6 != 0) {
                        Cvar_Set("com_expectedhunkusage", v6);
                        _Z_FreeInternal(buf);
                        return;
                    }
                }
                v5 = Com_Parse(&buftrav);
                if (v5 == NULL)
                    break;
            }
        }
        _Z_FreeInternal(buf);
    }
    Cvar_Set("com_expectedhunkusage", "-1");
}

// ============================================================================
// SV_Shutdown — ea: 0x520580
// ============================================================================
void SV_Shutdown() {
    if (com_sv_running != NULL && com_sv_running->integer != 0) {
        Com_Printf("----- Server Shutdown -----\n");
        if (gvm != NULL) {
            VM_Call(gvm, 1, 0);
            VM_Free(gvm);
            gvm = NULL;
        }
        SV_ClearServer();
        if (svs.clients != NULL)
            _Z_FreeInternal(svs.clients);
        memset(&svs, 0, sizeof(svs));
        Cvar_Set("sv_running", "0");
        svs.initialized = 0;
        Com_Printf("---------------------------\n");
        CL_Disconnect();
    }
}

// ============================================================================
// SV_AddServerCommand — ea: 0x520630
// ============================================================================
void SV_AddServerCommand(client_s* client, const char* cmd) {
    int LocalClientIndex = LocalClient::FirstLocalClientIndex();
    if (client->mEntityHandle.mHandle.mVal == EntityManager::sInst->GetPlayer(LocalClientIndex)->mHandle.mHandle.mVal) {
        if (client->reliableSequence - client->reliableAcknowledge == 64) {
            SV_DumpServerCommands(client);
            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sv_main.cpp";
            AeAssert::gCurrentLine = 158;
            AeAssert::gCurrentExpr = "0";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("Server command overflow - Tell MikeA"))
                __debugbreak();
        }
        if (client->reliableSequence - client->reliableAcknowledge >= 64) {
            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sv_main.cpp";
            AeAssert::gCurrentLine = 161;
            AeAssert::gCurrentExpr = "client->reliableSequence - client->reliableAcknowledge < 64";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
                __debugbreak();
        }
        char v3 = (char)(char)client->reliableSequence + 1;
        ++client->reliableSequence;
        SV_AddReliableCommand(client, v3 & 0x3F, cmd);
    }
}

// ============================================================================
// SV_SendServerCommand — ea: 0x520720
// ============================================================================
void SV_SendServerCommand(client_s* cl, const char* fmt, ...) {
    unsigned __int8 message[3072];
    va_list ap;
    va_start(ap, fmt);
    vsprintf((char*)message, fmt, ap);
    va_end(ap);
    if (cl != NULL) {
        SV_AddServerCommand(cl, (const char*)message);
    } else {
        client_s* clients = svs.clients;
        for (int i = 16; i != 0; --i) {
            if (clients->state == 1)
                SV_AddServerCommand(clients, (const char*)message);
            ++clients;
        }
    }
}

// ============================================================================
// SV_DumpServerCommands — ea: 0x51F780
// ============================================================================
void SV_DumpServerCommands(client_s* client) {
    Com_Printf("===== pending server commands =====\n");
    for (int i = client->reliableAcknowledge + 1; i <= client->reliableSequence; ++i)
        Com_Printf("cmd %5d: %s\n", i, client->reliableCommands.commands[i & 0x3F]);
}
