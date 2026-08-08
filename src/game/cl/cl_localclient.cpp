// ============================================================================
// cl_localclient.cpp - local-client helpers + netchan + misc (cl.o)
// 24 functions, verified against IDA (release map offsets + 0x40C000 = VA).
// ============================================================================

#include "cl_input.h"
#include "cl_console.h"

#include <string.h>

// ============================================================================
// Externs
// ============================================================================
extern void Com_Printf(const char* fmt, ...);
extern void Q_strncpyz(char* dest, const char* src, int destsize);
extern void MSG_WriteByte(struct msg_t* msg, int c);
extern void Netchan_Transmit(void* chan, int length, const unsigned char* data);
extern int Netchan_Process(void* chan, struct msg_t* msg);
extern void _Z_FreeInternal(void* ptr);
extern struct cvar_t* cl_shownet;
extern int unk_F6A290;   // dev/retail flag (2 = dev)
extern int unk_F6A28C;   // primary controller port
extern int dword_F6A28C; // active port

// msg_t (message buffer, from server_types)
struct msg_t {
    int overflowed;
    unsigned char* data;
    int maxsize;
    int cursize;
    int readcount;
};

// Debug-data globals (cl.o; shared with cl_debug.cpp)
extern int dword_F171C0;
extern int dword_F171C4;
extern int dword_F171C8;
extern int dword_F171CC;
extern int dword_F171D0;
extern int dword_F171D4;
extern int ptr;
extern int dword_F171DC;
extern int dword_F171E0;
extern int dword_F171E4;

// ============================================================================
// LocalClient statics
// ============================================================================
static int lFirstLocalClientIndex;
static int lLastLocalClientIndex;
static int lNumLocalClients;

// ea: 0x52EF50
void LocalClient_InitializeClientControllers()
{
    unk_F6A28C = 0;
}

// ea: 0x52EF60
int LocalClient_LastLocalClientIndex()
{
    return lLastLocalClientIndex;
}

// ea: 0x52EF70
void LocalClient_SetLastLocalClientIndex(int index)
{
    lLastLocalClientIndex = index;
}

// ea: 0x52EF80
int LocalClient_FirstLocalClientIndex()
{
    return lFirstLocalClientIndex;
}

// ea: 0x52EF90
void LocalClient_SetFirstLocalClientIndex(int index)
{
    lFirstLocalClientIndex = index;
}

// ea: 0x52EFA0
int LocalClient_NumLocalClients()
{
    return lNumLocalClients;
}

// ea: 0x52EFB0
void LocalClient_SetNumLocalClients(int num)
{
    lNumLocalClients = num;
}

// ea: 0x52EFC0
int LocalClient_GetNumLocalClientsByState(int state)
{
    return unk_F6A290 == state;
}

// ea: 0x52EFE0
int LocalClient_ClientToPort(int client)
{
    if (client != 0)
        return 0;
    return unk_F6A28C;
}

// ea: 0x52F010
int LocalClient_PortToClient()
{
    return 0;
}

// ea: 0x52F020
int LocalClient_PortIsState(int port, int state)
{
    (void)port;
    return unk_F6A290 == state;
}

// ea: 0x52F040
int LocalClient_PortToValidClient(int port)
{
    int result = 0;
    int* v2 = &unk_F6A28C;
    while (v2[1] == 0 || *v2 != port)
    {
        v2 += 802;
        ++result;
        if (v2 >= &unk_F6A28C + 802 * 4)
            return 0;
    }
    return result;
}

// ea: 0x52F080
void LocalClient_SetClientPort(int client, int port)
{
    if (client == 0)
        unk_F6A28C = port;
}

// ea: 0x52F0A0
void LocalClient_UpdatePlayerPorts(int fixedPort)
{
    int availCont[1];
    availCont[0] = 1;
    if (dword_F6A28C >= 0)
        availCont[dword_F6A28C] = -1;
    if (fixedPort < 0 || (availCont[fixedPort] = -1, fixedPort != 0))
    {
        if (unk_F6A290 == 0)
        {
            int v1 = 0;
            while (availCont[v1] != 1)
            {
                if (++v1 >= 1)
                    return;
            }
            dword_F6A28C = v1;
            availCont[v1] = -1;
        }
    }
}

// ea: 0x52F0F0
int LocalClient_ConfigureLocalClients()
{
    lNumLocalClients = unk_F6A290 == 2;
    memset(kbss, 0, sizeof(kbss));
    memset(kb, 0, sizeof(kb));
    cl_stance_ss[0] = 0;
    cl_altFireButtonDown_ss[0] = 0;
    cl_grenadeButtonDown_ss[0] = 0;
    return 0;
}

// ea: 0x52F140
bool LocalClient_QuitClientOutOfGame(int client)
{
    (void)client;
    extern void MultiplayerMgr_DropHotJoiningPlayers(void* self);
    MultiplayerMgr_DropHotJoiningPlayers((void*)0);
    extern void (*gpBrocAPI_mCallbackQuitGame)();
    void (*mCallbackQuitGame)() = gpBrocAPI_mCallbackQuitGame;
    if (mCallbackQuitGame != nullptr)
        mCallbackQuitGame();
    return 1;
}

// ============================================================================
// Key catcher
// ============================================================================

// ea: 0x52E2B0
int Key_GetCatcher()
{
    return cls.keyCatchers;
}

// ea: 0x52E2C0
void Key_SetCatcher(int catcher)
{
    cls.keyCatchers = catcher;
}

// ============================================================================
// Netchan + misc
// ============================================================================

// ea: 0x52D930
int CL_ShutdownDebugData()
{
    if (ptr != 0)
    {
        _Z_FreeInternal((void*)ptr);
        ptr = 0;
    }
    if (dword_F171DC != 0)
    {
        _Z_FreeInternal((void*)dword_F171DC);
        dword_F171DC = 0;
    }
    if (dword_F171E0 != 0)
    {
        _Z_FreeInternal((void*)dword_F171E0);
        dword_F171E0 = 0;
    }
    if (dword_F171E4 != 0)
    {
        _Z_FreeInternal((void*)dword_F171E4);
        dword_F171E4 = 0;
    }
    if (dword_F171C8 != 0)
    {
        _Z_FreeInternal((void*)dword_F171C8);
        dword_F171C8 = 0;
    }
    if (dword_F171CC != 0)
        _Z_FreeInternal((void*)dword_F171CC);
    memset(&dword_F171C0, 0, 0x2C);
    return 0;
}

// ea: 0x52D9D0
void CL_Netchan_Transmit(void* chan, struct msg_t* msg)
{
    MSG_WriteByte(msg, 3);
    Netchan_Transmit(chan, msg->cursize, msg->data);
}

// ea: 0x52DA00
int CL_Netchan_Process(void* chan, struct msg_t* msg)
{
    return Netchan_Process(chan, msg);
}

// ea: 0x52DA10
void SHOWNET(struct msg_t* msg, char* s)
{
    if (cl_shownet->integer >= 2)
        Com_Printf("%3i %3i:%s\n", msg->readcount - 1, msg->cursize, s);
}

// ea: 0x52DF00
int CL_IsCGameRendering()
{
    return cls.state == 2;  // CA_ACTIVE
}

// ea: 0x52E2D0
int GetConfigString(unsigned int index, char* buf, int size)
{
    if (index >= 0x400)
        return 0;
    if (cls.servername[4 * index + 128] == 0)
    {
        // assert: cls.configstrings[index].IsDefined()
    }
    int v3 = cls.servername[4 * index + 128];
    if (v3 != 0)
    {
        const char* v4 = (const char*)(v3 + 12);
        if (v3 == -12)
        {
            // assert str
        }
        if (*v4 != 0)
        {
            Q_strncpyz(buf, v4, size);
            return 1;
        }
    }
    if (size != 0)
        *buf = 0;
    return 0;
}
