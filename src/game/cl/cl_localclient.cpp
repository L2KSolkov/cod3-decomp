// ============================================================================
// cl_localclient.cpp - local-client helpers + netchan + misc (cl.o)
// 24 functions, verified against IDA (release map offsets + 0x40C000 = VA).
// ============================================================================

#include "cl_input.h"
#include "cl_console.h"

#include <string.h>

extern int dword_F6A290[4 * 802];  // Xbox dev/retail flag array @ 0xF6A290

struct netchan_t;

// ============================================================================
// Externs
// ============================================================================
extern void Com_Printf(const char* fmt, ...);
extern void Q_strncpyz(char* dest, const char* src, int destsize);
extern void MSG_WriteByte(struct msg_t* msg, int c);
extern void Netchan_Transmit(netchan_t* chan, int length,
                             const unsigned char* data);
extern int Netchan_Process(netchan_t* chan, struct msg_t* msg);
extern void _Z_FreeInternal(void* ptr);
extern struct cvar_t* cl_shownet;

// Minimal view (mp.o); sInst symbol ?sInst@MultiplayerMgr@@2PAV1@A
struct MultiplayerMgr {
    static MultiplayerMgr* sInst;
    void DropHotJoiningPlayers();
};
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

// LocalClient (cl.o; C++ statics matching the binary mangling)
class LocalClient {
public:
    static int  LastLocalClientIndex();          // ?LastLocalClientIndex@LocalClient@@YAHXZ (0x52EF60)
    static int  NumLocalClients();               // ?NumLocalClients@LocalClient@@YAHXZ (0x52EFA0)
    static void SetNumLocalClients(int num);     // ?SetNumLocalClients@LocalClient@@YAXH@Z (0x52EFB0)
    static int  GetNumLocalClientsByState(int state);  // ?GetNumLocalClientsByState@LocalClient@@YAHW4ELocalPlayerStates@@@Z (0x52EFC0)
    static int  PortToClient(int port);          // ?PortToClient@LocalClient@@YAHH@Z (0x52F010)
    static bool PortIsState(int port, int state);// ?PortIsState@LocalClient@@YA_NHH@Z (0x52F020)
    static int  PortToValidClient(int port);     // ?PortToValidClient@LocalClient@@YAHH@Z (0x52F040)
    static void SetClientPort(int client, int port);  // ?SetClientPort@LocalClient@@YAXHH@Z (0x52F080)
    static void UpdatePlayerPorts(int fixedPort);      // ?UpdatePlayerPorts@LocalClient@@YAXH@Z (0x52F0A0)
    static void ConfigureLocalClients();               // ?ConfigureLocalClients@LocalClient@@YAXXZ (0x52F0F0)
    static bool QuitClientOutOfGame(int client);       // ?QuitClientOutOfGame@LocalClient@@YA_NH@Z (0x52F140)
};

// ea: 0x52EF50
void LocalClient_InitializeClientControllers()
{
    unk_F6A28C = 0;
}

// ea: 0x52EF60
int LocalClient::LastLocalClientIndex()
{
    return lLastLocalClientIndex;
}
int LocalClient_LastLocalClientIndex()
{
    return LocalClient::LastLocalClientIndex();
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
int LocalClient::NumLocalClients()
{
    return lNumLocalClients;
}
int LocalClient_NumLocalClients()
{
    return LocalClient::NumLocalClients();
}

// ea: 0x52EFB0
void LocalClient::SetNumLocalClients(int num)
{
    lNumLocalClients = num;
}
void LocalClient_SetNumLocalClients(int num)
{
    LocalClient::SetNumLocalClients(num);
}

// ea: 0x52EFC0
int LocalClient::GetNumLocalClientsByState(int state)
{
    return dword_F6A290[0] == state;
}
int LocalClient_GetNumLocalClientsByState(int state)
{
    return LocalClient::GetNumLocalClientsByState(state);
}

// ea: 0x52EFE0
int LocalClient_ClientToPort(int client)
{
    if (client != 0)
        return 0;
    return unk_F6A28C;
}

// ea: 0x52F010
int LocalClient::PortToClient(int port)
{
    (void)port;
    return 0;
}
int LocalClient_PortToClient()
{
    return LocalClient::PortToClient(0);
}

// ea: 0x52F020
bool LocalClient::PortIsState(int port, int state)
{
    (void)port;
    return dword_F6A290[0] == state;
}
int LocalClient_PortIsState(int port, int state)
{
    return LocalClient::PortIsState(port, state);
}

// ea: 0x52F040
int LocalClient::PortToValidClient(int port)
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
int LocalClient_PortToValidClient(int port)
{
    return LocalClient::PortToValidClient(port);
}

// ea: 0x52F080
void LocalClient::SetClientPort(int client, int port)
{
    if (client == 0)
        unk_F6A28C = port;
}
void LocalClient_SetClientPort(int client, int port)
{
    LocalClient::SetClientPort(client, port);
}

// ea: 0x52F0A0
void LocalClient::UpdatePlayerPorts(int fixedPort)
{
    int availCont[1];
    availCont[0] = 1;
    if (dword_F6A28C >= 0)
        availCont[dword_F6A28C] = -1;
    if (fixedPort < 0 || (availCont[fixedPort] = -1, fixedPort != 0))
    {
        if (dword_F6A290[0] == 0)
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
void LocalClient_UpdatePlayerPorts(int fixedPort)
{
    LocalClient::UpdatePlayerPorts(fixedPort);
}

// ea: 0x52F0F0
void LocalClient::ConfigureLocalClients()
{
    lNumLocalClients = dword_F6A290[0] == 2;
    memset(kbss, 0, sizeof(kbss));
    memset(kb, 0, sizeof(kb));
    cl_stance_ss[0] = 0;
    cl_altFireButtonDown_ss[0] = 0;
    cl_grenadeButtonDown_ss[0] = 0;
}
int LocalClient_ConfigureLocalClients()
{
    LocalClient::ConfigureLocalClients();
    return 0;
}

// ea: 0x52F140
bool LocalClient::QuitClientOutOfGame(int client)
{
    (void)client;
    MultiplayerMgr::sInst->DropHotJoiningPlayers();
    extern void (*gpBrocAPI_mCallbackQuitGame)();
    void (*mCallbackQuitGame)() = gpBrocAPI_mCallbackQuitGame;
    if (mCallbackQuitGame != nullptr)
        mCallbackQuitGame();
    return 1;
}
bool LocalClient_QuitClientOutOfGame(int client)
{
    return LocalClient::QuitClientOutOfGame(client);
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
void CL_Netchan_Transmit(netchan_t* chan, struct msg_t* msg)
{
    MSG_WriteByte(msg, 3);
    Netchan_Transmit(chan, msg->cursize, msg->data);
}

// ea: 0x52DA00
int CL_Netchan_Process(netchan_t* chan, struct msg_t* msg)
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
