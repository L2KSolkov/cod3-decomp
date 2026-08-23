// ============================================================================
// bdPlatformSocket-win32.cpp - WinSock socket implementation (8 funcs).
// Source: bdPlatform:bdPlatformSocket-win32.obj
// Verified against IDA (release decompilation; the binary's create() picks
// protocol 254 VDP when non-blocking, else UDP + SO_BROADCAST).
// ============================================================================

#ifdef _WIN32
  #define WIN32_LEAN_AND_MEAN
  #include <winsock2.h>
  #pragma comment(lib, "ws2_32.lib")
#else
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <arpa/inet.h>
  #include <unistd.h>
#endif

#include <string.h>

#include "bd/bd_types.h"

#define BD_UDP_IP_OVERHEAD 28

// ============================================================================
// bdPlatformSocket::create - ea: 0x9ED1A0
// ============================================================================
int bdPlatformSocket::create(bool blocking) {
    // Xbox uses protocol 254 (VDP) for the nonblocking socket. WinSock does
    // not expose VDP, so the Win32 transport uses the equivalent UDP socket.
    if (!blocking)
        return (int)socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    int handle = (int)socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    int broadcast = 1;
    if (setsockopt(handle, SOL_SOCKET, SO_BROADCAST, (const char*)&broadcast,
                   sizeof(broadcast)) != 0)
        return -1;
    return handle;
}

// ============================================================================
// bdPlatformSocket::bind - ea: 0x9ED210
// ============================================================================
bdSocketStatusCode bdPlatformSocket::bind(int& handle, const bdInAddr* addr,
                                          unsigned short port) {
    if (handle == BD_INVALID_SOCKET_HANDLE)
        return BD_NET_INVALID_HANDLE;

    struct sockaddr_in localAddr;
    memset(&localAddr, 0, sizeof(localAddr));
    localAddr.sin_family = AF_INET;
    localAddr.sin_addr.s_addr = addr->inUn.m_iaddr;
    localAddr.sin_port = htons(port);

    if (::bind(handle, (struct sockaddr*)&localAddr, sizeof(localAddr)) == -1) {
        int error = WSAGetLastError();
        closesocket(handle);
        if (error == 10013 || (error > 10047 && error <= 10049))
            return BD_NET_ADDRESS_IN_USE;
        return BD_NET_ERROR;
    }
    return BD_NET_SUCCESS;
}

// ============================================================================
// bdPlatformSocket::sendTo - ea: 0x9ED320
// ============================================================================
int bdPlatformSocket::sendTo(int handle, const bdInAddr* addr, unsigned short port,
                             const void* data, unsigned int length) {
    if (handle == BD_INVALID_SOCKET_HANDLE)
        return BD_NET_INVALID_HANDLE;

    struct sockaddr_in to;
    memset(&to, 0, sizeof(to));
    to.sin_family = AF_INET;
    to.sin_addr.s_addr = addr->inUn.m_iaddr;
    to.sin_port = htons(port);

    int result = (int)::sendto(handle, (const char*)data, length, 0,
                               (struct sockaddr*)&to, sizeof(to));
    if (result >= 0) {
        m_totalBytesSent += result + BD_UDP_IP_OVERHEAD;
        ++m_totalPacketsSent;
    } else {
        switch (WSAGetLastError()) {
        case 10004: result = BD_NET_BLOCKING_CALL_CANCELED; break;
        case 10022: case 10039: case 10047: case 10049:
            result = BD_NET_ADDRESS_INVALID; break;
        case 10035: result = BD_NET_WOULD_BLOCK; break;
        case 10040: result = BD_NET_MSG_SIZE; break;
        case 10050: case 10051: case 10053: case 10054: case 10060: case 10065:
            result = BD_NET_CONNECTION_RESET; break;
        default: result = BD_NET_ERROR; break;
        }
    }
    return result;
}

// ============================================================================
// bdPlatformSocket::receiveFrom - ea: 0x9ED540
// ============================================================================
int bdPlatformSocket::receiveFrom(int& handle, bdInAddr& addr, unsigned short& port,
                                  void* data, unsigned int size) {
    if (handle == BD_INVALID_SOCKET_HANDLE)
        return BD_NET_INVALID_HANDLE;

    struct sockaddr_in from;
    memset(&from, 0, sizeof(from));
    int fromLen = sizeof(from);
    int received = (int)::recvfrom(handle, (char*)data, size, 0,
                                   (struct sockaddr*)&from, &fromLen);
    if (received >= 0) {
        port = ntohs(from.sin_port);
        addr.inUn.m_iaddr = from.sin_addr.s_addr;
        m_totalBytesRecvd += received + BD_UDP_IP_OVERHEAD;
        ++m_totalPacketsRecvd;
        return received;
    }

    switch (WSAGetLastError()) {
    case 10004: return BD_NET_BLOCKING_CALL_CANCELED;
    case 10022: return BD_NET_NOT_BOUND;
    case 10035: return BD_NET_WOULD_BLOCK;
    case 10040: return BD_NET_MSG_SIZE;
    case 10050: case 10051: case 10053: case 10054: case 10060: case 10065:
        port = ntohs(from.sin_port);
        addr.inUn.m_iaddr = from.sin_addr.s_addr;
        return BD_NET_CONNECTION_RESET;
    default:
        return BD_NET_ERROR;
    }
}

// ============================================================================
// bdPlatformSocket::close - ea: 0x9ED790
// ============================================================================
bool bdPlatformSocket::close(int& handle) {
    if (handle == BD_INVALID_SOCKET_HANDLE)
        return true;
    bool closed = (closesocket(handle) == 0);
    handle = BD_INVALID_SOCKET_HANDLE;
    return closed;
}

// ============================================================================
// bdPlatformSocket::unregisterThread - ea: 0x9ED7C0
// ============================================================================
void bdPlatformSocket::unregisterThread() {
}

// ============================================================================
// bdPlatformSocket::setBlocking - ea: 0x9ED7D0
// ============================================================================
bool bdPlatformSocket::setBlocking(int& handle, bool blocking) {
    if (handle == BD_INVALID_SOCKET_HANDLE)
        return false;
    unsigned long nonblocking = blocking ? 0 : 1;
    return ioctlsocket(handle, FIONBIO, &nonblocking) == 0;
}

// ============================================================================
// bdPlatformSocket::getHostByName - ea: 0x9ED840
// (returns 0 in the binary: no DNS on the target)
// ============================================================================
unsigned int bdPlatformSocket::getHostByName(const char*, bdInAddr*, unsigned int, int) {
    return 0;
}
