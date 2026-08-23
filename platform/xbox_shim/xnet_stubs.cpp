#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>

#include "bd/bdGameInfo.h"
#include "xlive.h"

#include <stdio.h>
#include <string.h>

extern "C" {

unsigned int __stdcall XNetGetTitleXnAddr(XNADDR* pxna)
{
    if (pxna == NULL)
        return 0x8000;

    memset(pxna, 0, sizeof(*pxna));
    pxna->ina[0] = 127;
    pxna->ina[3] = 1;
    memcpy(pxna->inaOnline, pxna->ina, sizeof(pxna->ina));
    return 0;
}

int __stdcall XNetTsAddrToInAddr(const XNADDR* ptsa, unsigned int,
                                 const XNKID*, struct in_addr* pina)
{
    if (pina == NULL)
        return 1;
    pina->s_addr = 0;
    if (ptsa != NULL)
        memcpy(&pina->s_addr, ptsa->ina, sizeof(pina->s_addr));
    return 0;
}

int __stdcall XNetXnAddrToInAddr(const XNADDR* pxna, const XNKID*,
                                 struct in_addr* pina)
{
    return XNetTsAddrToInAddr(pxna, 0, NULL, pina);
}

int __stdcall XNetInAddrToXnAddr(struct in_addr ina, XNADDR* pxna,
                                 XNKID* pxnkid)
{
    if (pxna == NULL || pxnkid == NULL)
        return 1;
    memset(pxna, 0, sizeof(*pxna));
    memcpy(pxna->ina, &ina.s_addr, sizeof(ina.s_addr));
    memset(pxnkid, 0, sizeof(*pxnkid));
    return 0;
}

int __stdcall XNetInAddrToString(struct in_addr ina, char* pchBuf, int cchBuf)
{
    if (pchBuf == NULL || cchBuf <= 0)
        return 1;
    const unsigned char* bytes = reinterpret_cast<const unsigned char*>(&ina.s_addr);
    _snprintf_s(pchBuf, cchBuf, _TRUNCATE, "%u.%u.%u.%u",
                bytes[0], bytes[1], bytes[2], bytes[3]);
    return 0;
}

int __stdcall XNetGetConnectStatus(struct in_addr)
{
    return 0;
}

int __stdcall XNetUnregisterInAddr(struct in_addr)
{
    return 0;
}

int __stdcall XNetCreateKey(XNKID* pxnkid, XNKEY* pxnkey)
{
    if (pxnkid != NULL)
        memset(pxnkid, 0, sizeof(*pxnkid));
    if (pxnkey != NULL)
        memset(pxnkey, 0, sizeof(*pxnkey));
    return 0;
}

int __stdcall XNetConnect(struct in_addr)
{
    return 0;
}

int __stdcall XNetQosLookup(
    unsigned int, const XNADDR**, const XNKID**, const XNKEY**, unsigned int,
    const void*, const DWORD*, unsigned int, DWORD, DWORD, HANDLE, XNQOS** ppxnqos)
{
    if (ppxnqos != NULL)
        *ppxnqos = NULL;
    return 1;
}

int __stdcall XNetQosRelease(XNQOS*)
{
    return 0;
}

int __stdcall XNetGetEthernetLinkStatus()
{
    return 1;
}

void __stdcall DmGetXboxName(char* name, unsigned int* size)
{
    static const char value[] = "COD3-WIN32";
    if (name == NULL || size == NULL || *size == 0)
        return;
    unsigned int capacity = *size;
    unsigned int length = static_cast<unsigned int>(strlen(value));
    if (length >= capacity)
        length = capacity - 1;
    memcpy(name, value, length);
    name[length] = '\0';
    *size = length;
}

}

void D3DDevice_SetRenderState_Deferred(unsigned int, unsigned int)
{
}
