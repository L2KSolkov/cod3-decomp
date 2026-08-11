// ============================================================================
// g_msg.cpp - game.o MSG_*/Netchan_*/NET_* serialization (net_chan.cpp)
// Verified against IDA (release map offsets + 0x40C000 = VA).
// ============================================================================

#include "game/logic/g_local.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

extern const char* NET_AdrToString(netadr_t a);  // core.o
static const char* netsrcString[2] = { "client", "server" };  // @ 0xDF6BB4

// ============================================================================
// MSG_* - ea: 0x60F260..0x60F650
// ============================================================================
// ea: 0x0060F260
void MSG_Init(msg_t* buf, unsigned char* data, int length)
{
    buf->overflowed = 0;
    buf->data = nullptr;
    buf->maxsize = 0;
    buf->cursize = 0;
    buf->readcount = 0;
    buf->maxsize = length;
    buf->data = data;
    memset(data, 0, length);
}

// ea: 0x0060F2A0
void MSG_BeginReading(msg_t* msg)
{
    msg->readcount = 0;
}

// ea: 0x0060F2B0
void MSG_WriteFlag(msg_t* msg, int value)
{
    msg->data[msg->cursize++] = value;
}

// ea: 0x0060F2D0
int MSG_ReadFlag(msg_t* msg)
{
    int readcount = msg->readcount;
    int result = msg->data[readcount];
    msg->readcount = readcount + 1;
    return result;
}

// ea: 0x0060F2F0
void MSG_WriteChar(msg_t* msg, int c)
{
    msg->data[msg->cursize++] = c;
}

// ea: 0x0060F310
void MSG_WriteByte(msg_t* msg, unsigned char c)
{
    msg->data[msg->cursize++] = c;
}

// ea: 0x0060F330
void MSG_WriteData(msg_t* msg, const void* data, int length)
{
    memcpy(&msg->data[msg->cursize], data, length);
    msg->cursize += (int)length;
}

// ea: 0x0060F370
void MSG_WriteShort(msg_t* msg, int c)
{
    msg->data[msg->cursize] = (unsigned char)c;
    int v3 = msg->cursize + 1;
    msg->cursize = v3;
    msg->data[v3] = (unsigned char)((unsigned short)c >> 8);
    ++msg->cursize;
}

// ea: 0x0060F3A0
void MSG_WriteLong(msg_t* msg, int c)
{
    msg->data[msg->cursize] = (unsigned char)c;
    int v3 = msg->cursize + 1;
    msg->cursize = v3;
    msg->data[v3] = (unsigned char)((unsigned int)c >> 8);
    int v5 = msg->cursize + 1;
    msg->cursize = v5;
    msg->data[v5] = (unsigned char)((unsigned int)c >> 16);
    int v6 = msg->cursize + 1;
    msg->cursize = v6;
    msg->data[v6] = (unsigned char)((unsigned int)c >> 24);
    ++msg->cursize;
}

// ea: 0x0060F3F0
void MSG_WriteFloat(msg_t* msg, float f)
{
    unsigned int bits;
    memcpy(&bits, &f, 4);
    msg->data[msg->cursize] = (unsigned char)bits;
    int v3 = msg->cursize + 1;
    msg->cursize = v3;
    msg->data[v3] = (unsigned char)(bits >> 8);
    int v5 = msg->cursize + 1;
    msg->cursize = v5;
    msg->data[v5] = (unsigned char)(bits >> 16);
    int v6 = msg->cursize + 1;
    msg->cursize = v6;
    msg->data[v6] = (unsigned char)(bits >> 24);
    ++msg->cursize;
}

// ea: 0x0060F450
char MSG_ReadChar(msg_t* msg)
{
    int readcount = msg->readcount;
    unsigned char result = msg->data[readcount];
    int cursize = msg->cursize;
    msg->readcount = ++readcount;
    if (readcount > cursize)
        return (unsigned char)-1;
    return result;
}

// ea: 0x0060F470
unsigned char MSG_ReadByte(msg_t* msg)
{
    int readcount = msg->readcount;
    unsigned char result = msg->data[readcount];
    int cursize = msg->cursize;
    msg->readcount = ++readcount;
    if (readcount > cursize)
        return (unsigned char)-1;
    return result;
}

// ea: 0x0060F490
short MSG_ReadShort(msg_t* msg)
{
    int readcount = msg->readcount;
    unsigned char* data = msg->data;
    unsigned char v4 = data[readcount++];
    msg->readcount = readcount;
    short c = (short)v4;
    c = (short)(c | ((short)data[readcount] << 8));
    int cursize = msg->cursize;
    msg->readcount = ++readcount;
    if (readcount > cursize)
        return -1;
    return c;
}

// ea: 0x0060F4D0
int MSG_ReadLong(msg_t* msg)
{
    unsigned char* data = msg->data;
    int readcount = msg->readcount;
    unsigned char v4 = data[readcount++];
    msg->readcount = readcount;
    int c = v4;
    unsigned char v5 = data[readcount];
    msg->readcount = ++readcount;
    c |= (int)v5 << 8;
    c |= (int)data[readcount] << 16;
    msg->readcount = readcount + 1;
    c |= (int)data[readcount + 1] << 24;
    bool v6 = readcount + 2 <= msg->cursize;
    msg->readcount = readcount + 2;
    if (v6)
        return c;
    return -1;
}

// ea: 0x0060F520
float MSG_ReadFloat(msg_t* msg)
{
    unsigned char* data = msg->data;
    int readcount = msg->readcount;
    unsigned char v4 = data[readcount++];
    msg->readcount = readcount;
    float f;
    unsigned char* fp = (unsigned char*)&f;
    fp[0] = v4;
    unsigned char v5 = data[readcount];
    msg->readcount = ++readcount;
    fp[1] = v5;
    fp[2] = data[readcount];
    msg->readcount = readcount + 1;
    fp[3] = data[readcount + 1];
    bool v6 = readcount + 2 <= msg->cursize;
    msg->readcount = readcount + 2;
    if (!v6)
        return -1.0f;
    return f;
}

static char string_0[256];
static char string_1[256];

// ea: 0x0060F580
char* MSG_ReadString(msg_t* msg)
{
    int readcount = msg->readcount;
    int v2 = (int)strlen((const char*)&msg->data[readcount]);
    if (v2 >= 256 || v2 + readcount >= msg->cursize)
    {
        string_0[0] = 0;
        return string_0;
    }
    strcpy(string_0, (const char*)&msg->data[readcount]);
    msg->readcount += (int)strlen(string_0) + 1;
    return string_0;
}

// ea: 0x0060F600
char* MSG_ReadStringLine(msg_t* msg)
{
    int readcount = msg->readcount;
    int v2 = (int)strlen((const char*)&msg->data[readcount]);
    if (v2 >= 256 || v2 + readcount >= msg->cursize)
    {
        string_1[0] = 0;
        return string_1;
    }
    strcpy(string_1, (const char*)&msg->data[readcount]);
    msg->readcount += (int)strlen(string_1) + 1;
    return string_1;
}

// ============================================================================
// Netchan_* - ea: 0x60F680..0x60F710
// ============================================================================
cvar_t* showpackets = nullptr;  // ?showpackets@@3PAUcvar_t@@A (game.o)
cvar_t* showdrop = nullptr;     // ?showdrop@@3PAUcvar_t@@A (game.o)

#define MAX_MSGLEN 0xC00
struct loopback_t {
    unsigned char data[MAX_MSGLEN];  // +0x000
    int datalen;                     // +0xC00
    int get;                         // +0xC04
    int send;                        // +0xC08
};
static loopback_t loopbacks[2];

// ea: 0x0060F680
int Netchan_Init()
{
    showpackets = Cvar_Get("showpackets", "0", 256);
    showdrop = Cvar_Get("showdrop", "0", 256);
    memset(loopbacks, 0, sizeof(loopbacks));
    return 0;
}

// ea: 0x0060F6D0
void Netchan_Setup(netsrc_t sock, netchan_t* chan, netadr_t adr, int qport)
{
    memset(chan, 0, 0xC30u);
    chan->sock = sock;
    chan->remoteAddress = adr;
    chan->qport = qport;
    chan->incomingSequence = 0;
    chan->outgoingSequence = 1;
}

// ea: 0x00629970
int Netchan_Process(netchan_t* chan, msg_t* msg)
{
    unsigned char* data = msg->data;
    msg->readcount = 0;
    unsigned int seq;
    ((unsigned char*)&seq)[0] = *data;
    msg->readcount = 1;
    ((unsigned char*)&seq)[1] = data[1];
    msg->readcount = 2;
    ((unsigned char*)&seq)[2] = data[2];
    msg->readcount = 3;
    ((unsigned char*)&seq)[3] = data[3];
    int cursize = msg->cursize;
    msg->readcount = 4;
    int v8 = cursize >= 4 ? (int)seq : -1;
    if (chan->sock == NS_SERVER)
        msg->readcount = 6;
    if (showpackets->integer != 0)
        Com_Printf("%s recv %4i : s=%i\n", netsrcString[chan->sock],
                   cursize, v8);
    int incomingSequence = chan->incomingSequence;
    if (v8 > incomingSequence)
    {
        int v12 = v8 - incomingSequence - 1;
        chan->dropped = v12;
        if (v12 > 0 && (showdrop->integer != 0 || showpackets->integer != 0))
            Com_Printf("%s:Dropped %i packets at %i\n",
                       NET_AdrToString(chan->remoteAddress), v12, v8);
        chan->incomingSequence = v8;
        return 1;
    }
    if (showdrop->integer != 0 || showpackets->integer != 0)
        Com_Printf("%s:Out of order packet %i at %i\n",
                   NET_AdrToString(chan->remoteAddress), v8,
                   chan->incomingSequence);
    return 0;
}

// ============================================================================
// MSG_WriteString / Netchan_Transmit / NET_AdrToString
// ea: 0x61F7A0 / 0x61F860 / 0x61F9A0
// ============================================================================
extern int BigShort(unsigned short s);  // core.o
extern void NET_SendPacket(netsrc_t sock, unsigned int length,
                           const void* data, netadr_t to);  // g.o
static char s_0[64];  // ?s_0@@3PADA (game.o @ 0xF58BB8)

// ea: 0x0061F7A0
void MSG_WriteString(msg_t* msg, const char* s)
{
    if (s != nullptr)
    {
        int v2 = (int)strlen(s);
        if (v2 < 256)
        {
            char string[256];
            int v3 = 0;
            if (v2 > 0)
            {
                v3 = v2;
                memcpy(string, s, v2);
            }
            int cursize = msg->cursize;
            string[v3] = 0;
            strcpy((char*)&msg->data[cursize], string);
            msg->cursize += v2 + 1;
        }
        else
        {
            Com_Printf("MSG_WriteString: MAX_STRING_CHARS");
            msg->data[msg->cursize++] = 0;
        }
    }
    else
    {
        msg->data[msg->cursize++] = 0;
    }
}

// ea: 0x0061F860
void Netchan_Transmit(netchan_t* chan, int length,
                      const unsigned char* data)
{
    if (length > 3072)
        Com_Error(ERR_DROP, "Netchan_Transmit: length too large");
    if (length >= 2972)
        Com_Error(ERR_DROP, "Netchan_Transmit: length too large for fragment");
    int send_buf[775];
    memset(send_buf, 0, 3084);
    send_buf[772] = 0;
    int outgoingSequence = chan->outgoingSequence;
    send_buf[0] = outgoingSequence;
    send_buf[773] = outgoingSequence;
    int v4 = 4;
    bool isClient = chan->sock == NS_CLIENT;
    ++chan->outgoingSequence;
    if (isClient)
    {
        *((unsigned short*)&send_buf[1]) = 0;
        v4 = 6;
    }
    memcpy((unsigned char*)send_buf + v4, data, length);
    int v6 = length + v4;
    NET_SendPacket(chan->sock, (unsigned int)v6, send_buf,
                   chan->remoteAddress);
    if (showpackets->integer != 0)
        Com_Printf("%s send %4i : s=%i ack=%i\n",
                   netsrcString[chan->sock], v6,
                   chan->outgoingSequence - 1, chan->incomingSequence);
}

// ea: 0x0061F9A0
const char* NET_AdrToString(netadr_t a)
{
    if (a.type == NA_LOOPBACK)
    {
        Com_sprintf(s_0, 64, "loopback");
        return s_0;
    }
    int v2 = BigShort(a.port);
    if (a.type == NA_IP)
        Com_sprintf(s_0, 64, "%i.%i.%i.%i:%i", a.ip[0], a.ip[1], a.ip[2],
                    a.ip[3], v2);
    else
        Com_sprintf(s_0, 64,
                    "%02x%02x%02x%02x.%02x%02x%02x%02x%02x%02x:%i",
                    a.ipx[0], a.ipx[1], a.ipx[2], a.ipx[3], a.ipx[4],
                    a.ipx[5], a.ipx[6], a.ipx[7], a.ipx[8], a.ipx[9], v2);
    return s_0;
}

// ============================================================================
// NET_* - ea: 0x60F720..0x60FB10
// ============================================================================
// ea: 0x0060F720
int NET_CompareBaseAdrSigned(netadr_t* a, netadr_t* b)
{
    netadrtype_t type = a->type;
    if (a->type != b->type)
        return type - b->type;
    switch (type)
    {
    case NA_LOOPBACK:
        return 0;
    case NA_IP:
        return memcmp(a->ip, b->ip, 4u) != 0;
    case NA_IPX:
        return memcmp(a->ipx, b->ipx, 0xAu) != 0;
    default:
        break;
    }
    Com_Printf("NET_CompareBaseAdrSigned: bad address type\n");
    return 0;
}

// ea: 0x0060F790
int NET_CompareAdrSigned(netadr_t* a, netadr_t* b)
{
    netadrtype_t type = a->type;
    if (a->type != b->type)
        return type - b->type;
    switch (type)
    {
    case NA_LOOPBACK:
        return 0;
    case NA_IP:
    {
        unsigned short port = a->port;
        unsigned short v5 = b->port;
        if (port == v5)
            return memcmp(a->ip, b->ip, 4u) != 0;
        return port - v5;
    }
    case NA_IPX:
    {
        unsigned short v6 = a->port;
        unsigned short v7 = b->port;
        if (v6 == v7)
            return memcmp(a->ipx, b->ipx, 0xAu) != 0;
        return v6 - v7;
    }
    default:
        Com_Printf("NET_CompareAdrSigned: bad address type\n");
        return 0;
    }
}

// ea: 0x0060F840
int NET_CompareAdr(netadr_t a, netadr_t b)
{
    return NET_CompareAdrSigned(&a, &b) == 0;
}

// ea: 0x0060F860
int NET_IsLocalAddress(netadr_t adr)
{
    return adr.type == NA_LOOPBACK;
}

// ea: 0x0060F870
int NET_GetLoopPacket(netsrc_t sock, netadr_t* net_from, msg_t* net_message)
{
    loopback_t* v3 = &loopbacks[sock];
    int v4 = v3->send;
    if ((v4 - v3->get) > 1)
        v3->get = v4 - 1;
    int v5 = v3->get;
    if (v5 >= v4)
        return 0;
    v3->get = v5 + 1;
    memcpy(net_message->data, v3, v3->datalen);
    net_message->cursize = v3->datalen;
    net_from->type = NA_BOT;
    net_from->ip[0] = 0;
    net_from->ipx[0] = 0;
    *(int*)&net_from->ipx[4] = 0;
    *(int*)&net_from->ipx[8] = 0;
    net_from->type = NA_LOOPBACK;
    return 1;
}

// ea: 0x0060F910
void NET_SendLoopPacket(netsrc_t sock, unsigned int length, const void* data)
{
    loopback_t* v3 = &loopbacks[sock ^ 1];
    ++v3->send;
    memcpy(v3, data, length);
    v3->datalen = (int)length;
}

// ea: 0x0060F950
void NET_SendPacket(netsrc_t sock, unsigned int length, const void* data,
                    netadr_t to)
{
    if (showpackets->integer != 0 && *(const int*)data == -1)
        Com_Printf("send packet %4i\n", length);
    if (to.type != NA_LOOPBACK)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\net_chan.cpp";
        AeAssert::gCurrentLine = 362;
        AeAssert::gCurrentExpr = "to.type == NA_LOOPBACK";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    loopback_t* v4 = &loopbacks[sock ^ 1];
    ++v4->send;
    memcpy(v4, data, length);
    v4->datalen = (int)length;
}

// ea: 0x0060FA00
void NET_OutOfBandPrint(netsrc_t sock, netadr_t adr, const char* format, ...)
{
    char string[4];
    char v4[3068];
    va_list ap;
    va_start(ap, format);
    if (adr.type != NA_LOOPBACK)
    {
        string[0] = (char)-1;
        string[1] = (char)-1;
        string[2] = (char)-1;
        string[3] = (char)-1;
        vsprintf(v4, format, ap);
        NET_SendPacket(sock, 4 + (unsigned int)strlen(v4), string, adr);
    }
    va_end(ap);
}

// ea: 0x0060FAA0
int NET_StringToAdr(const char* s, netadr_t* a)
{
    if (strcmp(s, "localhost") != 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\net_chan.cpp";
        AeAssert::gCurrentLine = 406;
        AeAssert::gCurrentExpr = "!strcmp (s, \"localhost\")";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    a->type = NA_BOT;
    a->ip[0] = 0;
    a->ipx[0] = 0;
    *(int*)&a->ipx[4] = 0;
    *(int*)&a->ipx[8] = 0;
    a->type = NA_LOOPBACK;
    return 1;
}
